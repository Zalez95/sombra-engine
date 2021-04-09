#include <fstream>
#include <sstream>
#include <iomanip>
#include <optional>
#include "GLMJSON.h"
#include "se/physics/RigidBody.h"
#include "se/physics/forces/Gravity.h"
#include "se/physics/forces/PunctualForce.h"
#include "se/physics/forces/DirectionalForce.h"
#include "se/physics/forces/Gravity.h"
#include "se/collision/BoundingBox.h"
#include "se/collision/BoundingSphere.h"
#include "se/collision/Capsule.h"
#include "se/collision/TriangleCollider.h"
#include "se/collision/TerrainCollider.h"
#include "se/collision/CompositeCollider.h"
#include "se/animation/SkeletonAnimator.h"
#include "se/animation/StepAnimations.h"
#include "se/animation/LinearAnimations.h"
#include "se/animation/CubicSplineAnimations.h"
#include "se/graphics/core/UniformVariable.h"
#include "se/app/graphics/RawMesh.h"
#include "se/app/io/SceneSerializer.h"
#include "se/app/io/ShaderLoader.h"
#include "se/app/io/ImageReader.h"
#include "se/app/TagComponent.h"
#include "se/app/ScriptComponent.h"
#include "se/app/MeshComponent.h"
#include "se/app/TerrainComponent.h"
#include "se/app/TransformsComponent.h"
#include "se/app/CameraComponent.h"
#include "se/app/SkinComponent.h"
#include "se/app/AnimationComponent.h"
#include "se/app/MeshComponent.h"
#include "se/app/LightComponent.h"

using namespace se::utils;
using namespace se::graphics;
using namespace se::physics;
using namespace se::collision;
using namespace se::animation;

namespace se::app {

	struct Accessor
	{
		std::size_t buffer;
		TypeId type;
		bool normalized;
		int componentSize;
		std::size_t stride, offset;
	};

	struct SerializeData
	{
		const Scene& scene;
		std::unordered_map<Entity, std::size_t> entityIndexMap;
		std::unordered_map<const AnimationNode*, std::size_t> nodeIndexMap;
		nlohmann::json buffersJson;
		nlohmann::json accessorsJson;
	};

	struct DeserializeData
	{
		nlohmann::json json;
		std::ifstream dataStream;
		std::unordered_map<std::size_t, Entity> indexEntityMap;
		std::unordered_map<std::size_t, AnimationNode*> indexNodeMap;
		nlohmann::json& buffersJson;
		nlohmann::json& accessorsJson;
	};

	template <typename T>
	using ResultOptional = std::pair<Result, std::optional<T>>;

// Repository
	void serializeBuffer(const std::byte* data, size_t size, nlohmann::json& json, std::ostream& dataStream)
	{
		std::size_t location = dataStream.tellp();
		dataStream.write(reinterpret_cast<const char*>(data), size);

		json["offset"] = location;
		json["size"] = size;
	}

	Result deserializeBuffer(const nlohmann::json& json, std::istream& dataStream, std::vector<std::byte>& buffer)
	{
		auto itSize = json.find("size");
		if (itSize == json.end()) {
			return Result(false, "Missing size property");
		}

		auto itOffset = json.find("offset");
		if (itOffset == json.end()) {
			return Result(false, "Missing offset property");
		}

		std::size_t size = itSize->get<std::size_t>();
		std::size_t offset = itOffset->get<std::size_t>();

		buffer.resize(size);
		dataStream.seekg(offset);
		dataStream.read(reinterpret_cast<char*>(buffer.data()), size);

		if (dataStream.fail()) {
			return Result(false, "Failed to read the data stream");
		}

		return Result();
	}


	void serializeAccessor(const Accessor& accessor, nlohmann::json& json)
	{
		json["buffer"] = static_cast<int>(accessor.buffer);
		json["type"] = static_cast<int>(accessor.type);
		json["normalized"] = accessor.normalized;
		json["componentSize"] = accessor.componentSize;
		json["stride"] = accessor.stride;
		json["offset"] = accessor.offset;
	}

	Result deserializeAccessor(const nlohmann::json& json, Accessor& accessor)
	{
		auto itBuffer = json.find("buffer");
		if (itBuffer != json.end()) {
			accessor.buffer = *itBuffer;
		}
		else {
			return Result(false, "Missing buffer property");
		}

		auto itType = json.find("type");
		if (itType != json.end()) {
			accessor.type = static_cast<TypeId>(itType->get<int>());
		}

		auto itNormalized = json.find("normalized");
		if (itNormalized != json.end()) {
			accessor.normalized = *itNormalized;
		}

		auto itComponentSize = json.find("componentSize");
		if (itComponentSize != json.end()) {
			accessor.componentSize = *itComponentSize;
		}

		auto itStride = json.find("stride");
		if (itStride != json.end()) {
			accessor.stride = *itStride;
		}

		auto itOffset = json.find("offset");
		if (itOffset != json.end()) {
			accessor.offset = *itOffset;
		}

		return Result();
	}


	template <typename T>
	void serializeResource(const T& value, const Scene::Key& key, SerializeData& data, nlohmann::json& json, std::ostream& dataStream);

	template <typename T>
	Result deserializeResource(const nlohmann::json& json, const Scene::Key& key, DeserializeData& data, Scene& scene);


	template <>
	void serializeResource<Mesh>(const Mesh& mesh, const Scene::Key&, SerializeData& data, nlohmann::json& json, std::ostream& dataStream)
	{
		// VBOs
		const auto& vao = mesh.getVAO();

		nlohmann::json jsonVBOs;
		const auto& vbos = mesh.getVBOs();
		for (unsigned int i = 0; i < MeshAttributes::NumAttributes; ++i) {
			if (vao.hasVertexAttribute(i)) {
				auto itVBO = std::find_if(vbos.begin(), vbos.end(), [&](const auto& vbo) {
					return vao.checkVertexAttributeVBOBound(i, vbo);
				});
				if (itVBO != vbos.end()) {
					std::size_t bufferSize = itVBO->size();
					std::vector<std::byte> buffer(bufferSize);
					itVBO->read(buffer.data(), bufferSize);

					nlohmann::json bufferJson;
					serializeBuffer(buffer.data(), bufferSize, bufferJson, dataStream);
					data.buffersJson.emplace_back(std::move(bufferJson));

					Accessor accessor;
					accessor.buffer = data.buffersJson.size() - 1;
					mesh.getVAO().getVertexAttribute(
						i, accessor.type, accessor.normalized, accessor.componentSize,
						accessor.stride, accessor.offset
					);

					nlohmann::json accessorJson;
					serializeAccessor(accessor, accessorJson);
					data.accessorsJson.emplace_back(std::move(accessorJson));

					nlohmann::json jsonVBO;
					jsonVBO["accessor"] = data.accessorsJson.size() - 1;
					jsonVBO["attribute"] = i;
					jsonVBOs.emplace_back(std::move(jsonVBO));
				}
			}
		}
		json["vbos"] = jsonVBOs;

		// IBO
		const auto& ibo = mesh.getIBO();

		std::size_t bufferSize = ibo.size();
		std::vector<std::byte> buffer(bufferSize);
		ibo.read(buffer.data(), bufferSize);

		nlohmann::json bufferJson;
		serializeBuffer(buffer.data(), bufferSize, bufferJson, dataStream);
		data.buffersJson.emplace_back(std::move(bufferJson));

		nlohmann::json jsonIBO;
		jsonIBO["buffer"] = data.buffersJson.size() - 1;
		jsonIBO["type"] = static_cast<int>(ibo.getIndexType());
		jsonIBO["count"] = ibo.getIndexCount();
		json["ibo"] = jsonIBO;

		// Bounds
		nlohmann::json jsonBounds;
		auto [minimum, maximum] = mesh.getBounds();
		jsonBounds["minimum"] = toJson(minimum);
		jsonBounds["maximum"] = toJson(maximum);
		json["bounds"] = jsonBounds;
	}

	template <>
	Result deserializeResource<Mesh>(const nlohmann::json& json, const Scene::Key& key, DeserializeData& data, Scene& scene)
	{
		// VBOs
		VertexArray vao;

		auto itVBOs = json.find("vbos");
		if (itVBOs == json.end()) {
			return Result(false, "Missing \"vbos\" property");
		}

		std::vector<VertexBuffer> vbos;
		for (std::size_t i = 0; i < itVBOs->size(); ++i) {
			const auto& jsonVBO = (*itVBOs)[i];

			auto itAccessor = jsonVBO.find("accessor");
			auto itAttribute = jsonVBO.find("attribute");
			if (itAccessor == jsonVBO.end()) {
				return Result(false, "Missing \"accessor\" property in VBO " + std::to_string(i));
			}
			if (itAttribute == jsonVBO.end()) {
				return Result(false, "Missing \"attribute\" property in VBO " + std::to_string(i));
			}

			std::size_t iAccessor = *itAccessor;
			if (iAccessor > data.accessorsJson.size()) {
				return Result(false, "Accessor index " + std::to_string(iAccessor) + " out of bounds in VBO " + std::to_string(i));
			}

			Accessor accessor;
			auto result = deserializeAccessor(data.accessorsJson[iAccessor], accessor);
			if (!result) {
				return Result(false, "Failed to parse accessor " + std::to_string(iAccessor) + ": " + result.description());
			}

			if (accessor.buffer > data.buffersJson) {
				return Result(false, "Buffer index " + std::to_string(accessor.buffer) + " out of bounds in Accessor " + std::to_string(iAccessor));
			}

			std::vector<std::byte> buffer;
			result = deserializeBuffer(data.buffersJson[accessor.buffer], data.dataStream, buffer);
			if (!result) {
				return Result(false, "Failed to parse buffer " + std::to_string(accessor.buffer) + ": " + result.description());
			}

			unsigned int attribute = *itAttribute;
			if (attribute > MeshAttributes::NumAttributes) {
				return Result(false, "Not valid attribute " + std::to_string(attribute) + " in VBO " + std::to_string(i));
			}

			auto& vbo = vbos.emplace_back();
			vbo.resizeAndCopy(buffer.data(), buffer.size());
			vbo.bind();
			if ((accessor.type == graphics::TypeId::Float) || (accessor.type == graphics::TypeId::HalfFloat)) {
				vao.setVertexAttribute(attribute, accessor.type, accessor.normalized, accessor.componentSize, accessor.stride, accessor.offset);
			}
			else if (accessor.type == graphics::TypeId::Double) {
				vao.setVertexDoubleAttribute(attribute, accessor.type, accessor.componentSize, accessor.stride, accessor.offset);
			}
			else {
				vao.setVertexIntegerAttribute(attribute, accessor.type, accessor.componentSize, accessor.stride, accessor.offset);
			}
		}

		// IBO
		auto itIBO = json.find("ibo");
		if (itIBO == json.end()) {
			return Result(false, "Missing \"ibo\" property");
		}

		auto itBuffer = itIBO->find("buffer");
		auto itType = itIBO->find("type");
		auto itCount = itIBO->find("count");
		if (itBuffer == itIBO->end()) {
			return Result(false, "Missing IBO \"buffer\" property");
		}
		if (itType == itIBO->end()) {
			return Result(false, "Missing IBO \"type\" property");
		}
		if (itCount == itIBO->end()) {
			return Result(false, "Missing Count \"count\" property");
		}

		std::size_t iBuffer = *itBuffer;
		if (iBuffer > data.buffersJson.size()) {
			return Result(false, "IBO \"buffer\" out of bounds");
		}

		std::vector<std::byte> buffer;
		auto result = deserializeBuffer(data.buffersJson[iBuffer], data.dataStream, buffer);
		if (!result) {
			return Result(false, "Failed to parse buffer " + std::to_string(iBuffer) + ": " + result.description());
		}

		IndexBuffer ibo;
		ibo.resizeAndCopy(static_cast<void*>(buffer.data()), buffer.size(), static_cast<TypeId>(itType->get<int>()), *itCount);
		vao.bind();
		ibo.bind();

		// Bounds
		auto itBounds = json.find("bounds");
		if (itBounds == json.end()) {
			return Result(false, "Missing \"bounds\" property");
		}

		glm::vec3 minimum, maximum;
		auto itMinimum = itBounds->find("minimum");
		auto itMaximum = itBounds->find("maximum");
		if ((itMinimum == itBounds->end()) || !toVec3(*itMinimum, minimum)
			|| (itMaximum == itBounds->end()) || !toVec3(*itMaximum, maximum)
		) {
			return Result(false, "Wrong \"bounds\" property");
		}

		// Create the mesh
		auto mesh = std::make_shared<Mesh>(std::move(vbos), std::move(ibo), std::move(vao));
		mesh->setBounds(minimum, maximum);

		scene.repository.add(key, std::move(mesh));

		return Result();
	}


	template <>
	void serializeResource<Skin>(const Skin& skin, const Scene::Key&, SerializeData& data, nlohmann::json& json, std::ostream& dataStream)
	{
		nlohmann::json bufferJson;
		serializeBuffer(
			reinterpret_cast<const std::byte*>(skin.inverseBindMatrices.data()),
			skin.inverseBindMatrices.size() * sizeof(glm::mat4), bufferJson, dataStream
		);
		data.buffersJson.emplace_back(std::move(bufferJson));

		json["inverseBindMatrices"] = data.buffersJson.size() - 1;
	}

	template <>
	Result deserializeResource<Skin>(const nlohmann::json& json, const Scene::Key& key, DeserializeData& data, Scene& scene)
	{
		auto itInverseBindMatrices = json.find("inverseBindMatrices");
		if (itInverseBindMatrices == json.end()) {
			return Result(false, "Missing \"inverseBindMatrices\" property");
		}

		std::size_t iBuffer = *itInverseBindMatrices;

		std::vector<std::byte> buffer;
		buffer.reserve(Skin::kMaxJoints * sizeof(glm::mat4));
		auto result = deserializeBuffer(data.buffersJson[iBuffer], data.dataStream, buffer);
		if (!result) {
			return Result(false, "Failed to parse buffer " + std::to_string(iBuffer) + ": " + result.description());
		}

		auto skin = std::make_shared<Skin>();

		glm::mat4* bufPtr = reinterpret_cast<glm::mat4*>(buffer.data());
		std::copy(bufPtr, bufPtr + buffer.size() / sizeof(glm::mat4), skin->inverseBindMatrices.end());

		scene.repository.add(key, std::move(skin));

		return Result();
	}


	template <>
	void serializeResource<SkeletonAnimator>(const SkeletonAnimator& animator, const Scene::Key&, SerializeData& data, nlohmann::json& json, std::ostream& dataStream)
	{
		nlohmann::json nodeAnimatorsVJson;
		animator.processAnimators([&](const char* nodeName, TransformationAnimator::TransformationType tType, TransformationAnimator* animator) {
			const std::byte* bufferData = nullptr;
			std::size_t bufferDataSize = 0;
			std::string animationType, keyFrameType;

			if (auto vec3Animator = dynamic_cast<Vec3Animator*>(animator); vec3Animator) {
				animationType = "VEC3";
				if (auto lV3Animation = std::dynamic_pointer_cast<AnimationVec3Linear>(vec3Animator->getAnimation()); lV3Animation) {
					bufferData = reinterpret_cast<const std::byte*>(lV3Animation->getKeyFrames().data());
					bufferDataSize = lV3Animation->getKeyFrames().size() * sizeof(AnimationVec3Linear::KeyFrame);
					keyFrameType = "Linear";
				}
				else if (auto sV3Animation = std::dynamic_pointer_cast<AnimationVec3Step>(vec3Animator->getAnimation()); sV3Animation) {
					bufferData = reinterpret_cast<const std::byte*>(sV3Animation->getKeyFrames().data());
					bufferDataSize = sV3Animation->getKeyFrames().size() * sizeof(AnimationVec3Step::KeyFrame);
					keyFrameType = "Step";
				}
				else if (auto csV3Animation = std::dynamic_pointer_cast<AnimationVec3CubicSpline>(vec3Animator->getAnimation()); csV3Animation) {
					bufferData = reinterpret_cast<const std::byte*>(csV3Animation->getKeyFrames().data());
					bufferDataSize = csV3Animation->getKeyFrames().size() * sizeof(AnimationVec3CubicSpline::KeyFrame);
					keyFrameType = "CubicSpline";
				}
			}
			else if (auto quatAnimator = dynamic_cast<QuatAnimator*>(animator); quatAnimator) {
				animationType = "QUAT";
				if (auto lQuAnimation = std::dynamic_pointer_cast<AnimationQuatLinear>(quatAnimator->getAnimation()); lQuAnimation) {
					bufferData = reinterpret_cast<const std::byte*>(lQuAnimation->getKeyFrames().data());
					bufferDataSize = lQuAnimation->getKeyFrames().size() * sizeof(AnimationQuatLinear::KeyFrame);
					keyFrameType = "Linear";
				}
				else if (auto sQuAnimation = std::dynamic_pointer_cast<AnimationQuatStep>(quatAnimator->getAnimation()); sQuAnimation) {
					bufferData = reinterpret_cast<const std::byte*>(sQuAnimation->getKeyFrames().data());
					bufferDataSize = sQuAnimation->getKeyFrames().size() * sizeof(AnimationQuatStep::KeyFrame);
					keyFrameType = "Step";
				}
				else if (auto csQuAnimation = std::dynamic_pointer_cast<AnimationQuatCubicSpline>(quatAnimator->getAnimation()); csQuAnimation) {
					bufferData = reinterpret_cast<const std::byte*>(csQuAnimation->getKeyFrames().data());
					bufferDataSize = csQuAnimation->getKeyFrames().size() * sizeof(AnimationQuatCubicSpline::KeyFrame);
					keyFrameType = "CubicSpline";
				}
			}

			nlohmann::json bufferJson;
			serializeBuffer(bufferData, bufferDataSize, bufferJson, dataStream);
			data.buffersJson.emplace_back(std::move(bufferJson));

			nlohmann::json nodeAnimatorJson;
			nodeAnimatorJson["nodeName"] = nodeName;
			nodeAnimatorJson["transformationType"] = static_cast<int>(tType);
			nodeAnimatorJson["animationType"] = animationType;
			nodeAnimatorJson["keyFrameType"] = keyFrameType;
			nodeAnimatorJson["keyFrames"] = data.buffersJson.size() - 1;
			nodeAnimatorsVJson.emplace_back(std::move(nodeAnimatorJson));
		});

		json["loopTime"] = animator.getLoopTime();
		json["nodeAnimators"] = std::move(nodeAnimatorsVJson);
	}

	template <>
	Result deserializeResource<SkeletonAnimator>(const nlohmann::json& json, const std::string& key, DeserializeData& data, Scene& scene)
	{
		auto animator = std::make_shared<SkeletonAnimator>();

		auto itNodeAnimators = json.find("nodeAnimators");
		if (itNodeAnimators == json.end()) {
			return Result(false, "Missing \"nodeAnimators\" property");
		}

		for (std::size_t i = 0; i < itNodeAnimators->size(); ++i) {
			auto jsonNodeAnimator = (*itNodeAnimators)[i];
			auto itNodeName = jsonNodeAnimator.find("nodeName");
			if (itNodeName == jsonNodeAnimator.end()) {
				return Result(false, "Failed to parse nodeAnimators[" + std::to_string(i) + "]: Missing \"nodeName\" property");
			}
			auto itTransformationType = jsonNodeAnimator.find("transformationType");
			if (itTransformationType == jsonNodeAnimator.end()) {
				return Result(false, "Failed to parse nodeAnimators[" + std::to_string(i) + "]: Missing \"transformationType\" property");
			}
			auto itAnimationType = jsonNodeAnimator.find("animationType");
			if (itAnimationType == jsonNodeAnimator.end()) {
				return Result(false, "Failed to parse nodeAnimators[" + std::to_string(i) + "]: Missing \"animationType\" property");
			}
			auto itKeyFrameType = jsonNodeAnimator.find("keyFrameType");
			if (itKeyFrameType == jsonNodeAnimator.end()) {
				return Result(false, "Failed to parse nodeAnimators[" + std::to_string(i) + "]: Missing \"keyFrameType\" property");
			}
			auto itKeyFrames = jsonNodeAnimator.find("keyFrames");
			if (itKeyFrames == jsonNodeAnimator.end()) {
				return Result(false, "Failed to parse nodeAnimators[" + std::to_string(i) + "]: Missing \"keyFrames\" property");
			}

			std::string nodeName = *itNodeName;
			auto transformationType = static_cast<TransformationAnimator::TransformationType>(itTransformationType->get<int>());
			std::size_t iKeyFrames = *itKeyFrames;
			if (iKeyFrames >= data.buffersJson.size()) {
				return Result(false, "Failed to parse nodeAnimators[" + std::to_string(i) + "]: \"keyFrames\"=" + std::to_string(iKeyFrames) + " out of bounds");
			}

			std::vector<std::byte> keyFramesBuffer;
			deserializeBuffer(data.buffersJson[iKeyFrames], data.dataStream, keyFramesBuffer);

			std::string animationType = *itAnimationType, keyFrameType = *itKeyFrameType;
			if (animationType == "VEC3") {
				std::unique_ptr<IAnimation<glm::vec3>> animation;
				if (keyFrameType == "Linear") {
					auto lV3Animation = std::make_unique<AnimationVec3Linear>();
					lV3Animation->setKeyFrames(
						reinterpret_cast<const AnimationVec3Linear::KeyFrame*>(keyFramesBuffer.data()),
						keyFramesBuffer.size() / sizeof(AnimationVec3Linear::KeyFrame)
					);
					animation = std::move(lV3Animation);
				}
				else if (keyFrameType == "Step") {
					auto sV3Animation = std::make_unique<AnimationVec3Step>();
					sV3Animation->setKeyFrames(
						reinterpret_cast<const AnimationVec3Step::KeyFrame*>(keyFramesBuffer.data()),
						keyFramesBuffer.size() / sizeof(AnimationVec3Step::KeyFrame)
					);
					animation = std::move(sV3Animation);
				}
				else if (keyFrameType == "CubicSpline") {
					auto csV3Animation = std::make_unique<AnimationVec3CubicSpline>();
					csV3Animation->setKeyFrames(
						reinterpret_cast<const AnimationVec3CubicSpline::KeyFrame*>(keyFramesBuffer.data()),
						keyFramesBuffer.size() / sizeof(AnimationVec3CubicSpline::KeyFrame)
					);
					animation = std::move(csV3Animation);
				}
				else {
					return Result(false, "Failed to parse nodeAnimators[" + std::to_string(i) + "]: wrong KeyFrame type=" + keyFrameType);
				}

				auto nodeAnimator = std::make_unique<Vec3Animator>(std::move(animation));
				animator->addAnimator(nodeName.c_str(), transformationType, std::move(nodeAnimator));
			}
			else if (animationType == "QUAT") {
				std::unique_ptr<IAnimation<glm::quat>> animation;
				if (keyFrameType == "Linear") {
					auto lQuAnimation = std::make_unique<AnimationQuatLinear>();
					lQuAnimation->setKeyFrames(
						reinterpret_cast<const AnimationQuatLinear::KeyFrame*>(keyFramesBuffer.data()),
						keyFramesBuffer.size() / sizeof(AnimationQuatLinear::KeyFrame)
					);
					animation = std::move(lQuAnimation);
				}
				else if (keyFrameType == "Step") {
					auto sQuAnimation = std::make_unique<AnimationQuatStep>();
					sQuAnimation->setKeyFrames(
						reinterpret_cast<const AnimationQuatStep::KeyFrame*>(keyFramesBuffer.data()),
						keyFramesBuffer.size() / sizeof(AnimationQuatStep::KeyFrame)
					);
					animation = std::move(sQuAnimation);
				}
				else if (keyFrameType == "CubicSpline") {
					auto csQuAnimation = std::make_unique<AnimationQuatCubicSpline>();
					csQuAnimation->setKeyFrames(
						reinterpret_cast<const AnimationQuatCubicSpline::KeyFrame*>(keyFramesBuffer.data()),
						keyFramesBuffer.size() / sizeof(AnimationQuatCubicSpline::KeyFrame)
					);
					animation = std::move(csQuAnimation);
				}
				else {
					return Result(false, "Failed to parse nodeAnimators[" + std::to_string(i) + "]: wrong KeyFrame type=" + keyFrameType);
				}

				auto nodeAnimator = std::make_unique<QuatAnimator>(std::move(animation));
				animator->addAnimator(nodeName.c_str(), transformationType, std::move(nodeAnimator));
			}
			else {
				return Result(false, "Failed to parse nodeAnimators[" + std::to_string(i) + "]: wrong Animation type=" + animationType);
			}
		}

		auto itLoopTime = json.find("loopTime");
		if (itLoopTime == json.end()) {
			return Result(false, "Missing \"loopTime\" property");
		}

		animator->setLoopTime(*itLoopTime);

		scene.repository.add(key, std::move(animator));

		return Result();
	}


	template <>
	void serializeResource<LightSource>(const LightSource& light, const Scene::Key&, SerializeData&, nlohmann::json& json, std::ostream&)
	{
		json["type"] = static_cast<int>(light.type);
		json["color"] = toJson(light.color);
		json["intensity"] = light.intensity;
		json["range"] = light.range;
		json["innerConeAngle"] = light.innerConeAngle;
		json["outerConeAngle"] = light.outerConeAngle;
	}

	template <>
	Result deserializeResource<LightSource>(const nlohmann::json& json, const std::string& key, DeserializeData&, Scene& scene)
	{
		auto light = std::make_shared<LightSource>();

		auto itType = json.find("type");
		if (itType != json.end()) {
			light->type = static_cast<LightSource::Type>(itType->get<int>());
		}

		auto itColor = json.find("color");
		if (itColor != json.end()) {
			toVec3(*itColor, light->color);
		}

		auto itIntensity = json.find("intensity");
		if (itIntensity != json.end()) {
			light->intensity = *itIntensity;
		}

		auto itInnerConeAngle = json.find("innerConeAngle");
		if (itInnerConeAngle != json.end()) {
			light->innerConeAngle = *itInnerConeAngle;
		}

		auto itOuterConeAngle = json.find("outerConeAngle");
		if (itOuterConeAngle != json.end()) {
			light->outerConeAngle = *itOuterConeAngle;
		}

		scene.repository.add(key, std::move(light));

		return Result();
	}


	template <>
	void serializeResource<Program>(const Program&, const Scene::Key& key, SerializeData& data, nlohmann::json& json, std::ostream&)
	{
		auto path = data.scene.repository.find<Scene::Key, ResourcePath<Program>>(key);
		if (path) {
			json["path"] = path->path;
		}
	}

	template <>
	Result deserializeResource<Program>(const nlohmann::json& json, const std::string& key, DeserializeData&, Scene& scene)
	{
		auto itPath = json.find("path");
		if (itPath == json.end()) {
			return Result(false, "Missing \"path\" property");
		}

		std::string vertexPath, geometryPath, fragmentPath;
		std::stringstream ss(itPath->get<std::string>());
		std::getline(ss, vertexPath, ',');
		std::getline(ss, geometryPath, ',');
		std::getline(ss, fragmentPath, ',');

		std::shared_ptr<Program> program;
		auto result = ShaderLoader::createProgram(
			vertexPath.empty()? nullptr : vertexPath.c_str(),
			geometryPath.empty()? nullptr : geometryPath.c_str(),
			fragmentPath.empty()? nullptr : fragmentPath.c_str(),
			program
		);
		if (!result) {
			return Result(false, "Couldn't create the program: " + std::string(result.description()));
		}
		scene.repository.add<Scene::Key, Program>(key, std::move(program));
		scene.repository.emplace<Scene::Key, ResourcePath<Program>>(key, itPath->get<std::string>());

		return Result();
	}


	template <>
	void serializeResource<Texture>(const Texture& texture, const Scene::Key& key, SerializeData& data, nlohmann::json& json, std::ostream&)
	{
		auto path = data.scene.repository.find<Scene::Key, ResourcePath<Texture>>(key);
		if (path) {
			json["path"] = path->path;

			TextureFilter min, mag;
			texture.getFiltering(&min, &mag);

			json["min"] = static_cast<int>(min);
			json["mag"] = static_cast<int>(mag);

			TextureWrap wrapS, wrapT;
			texture.getWrapping(&wrapS, &wrapT);

			json["wrapS"] = static_cast<int>(wrapS);
			json["wrapT"] = static_cast<int>(wrapT);
		}
	}

	template <>
	Result deserializeResource<Texture>(const nlohmann::json& json, const std::string& key, DeserializeData&, Scene& scene)
	{
		auto toColorFormat = [](int channels) {
			switch (channels) {
				case 1:		return ColorFormat::Red;
				case 2:		return ColorFormat::RG;
				case 3:		return ColorFormat::RGB;
				default:	return ColorFormat::RGBA;
			}
		};

		auto itPath = json.find("path");
		if (itPath == json.end()) {
			return Result(false, "Missing \"path\" property");
		}

		// ONLY TEXTURE2D
		auto texture = std::make_shared<Texture>(TextureTarget::Texture2D);

		auto itMin = json.find("min");
		auto itMag = json.find("mag");
		TextureFilter min = (itMin != json.end())? static_cast<TextureFilter>(itMin->get<int>()) : TextureFilter::Nearest;
		TextureFilter mag = (itMag != json.end())? static_cast<TextureFilter>(itMag->get<int>()) : TextureFilter::Nearest;

		auto itWrapS = json.find("wrapS");
		auto itWrapT = json.find("wrapT");
		TextureWrap wrapS = (itWrapS != json.end())? static_cast<TextureWrap>(itWrapS->get<int>()) : TextureWrap::ClampToBorder;
		TextureWrap wrapT = (itWrapS != json.end())? static_cast<TextureWrap>(itWrapT->get<int>()) : TextureWrap::ClampToBorder;

		texture->setFiltering(min, mag)
			.setWrapping(wrapS, wrapT);

		std::string path = itPath->get<std::string>().c_str();
		if (path.substr(path.size() - 3, 3) == "hdr") {
			Image<float> image;
			auto result = ImageReader::readHDR(itPath->get<std::string>().c_str(), image);
			if (!result) {
				return Result(false, "Failed to read the HDR Image: " + std::string(result.description()));
			}

			ColorFormat format = toColorFormat(image.channels);
			texture->setImage(image.pixels.get(), TypeId::Float, format, format, image.width, image.height);
		}
		else {
			Image<unsigned char> image;
			auto result = ImageReader::read(itPath->get<std::string>().c_str(), image);
			if (!result) {
				return Result(false, "Failed to read the Image: " + std::string(result.description()));
			}

			ColorFormat format = toColorFormat(image.channels);
			texture->setImage(image.pixels.get(), TypeId::UnsignedByte, format, format, image.width, image.height);
		}

		texture->generateMipMap();

		scene.repository.add(Scene::Key(key), texture);
		scene.repository.emplace<Scene::Key, ResourcePath<Texture>>(key, itPath->get<std::string>());

		return Result();
	}


	template <>
	void serializeResource<Force>(const Force& force, const Scene::Key&, SerializeData&, nlohmann::json& json, std::ostream&)
	{
		if (auto gravity = dynamic_cast<const Gravity*>(&force); gravity) {
			json["type"] = "Gravity";
			json["value"] = gravity->getValue();
		}
		else if (auto directional = dynamic_cast<const DirectionalForce*>(&force); directional) {
			json["type"] = "DirectionalForce";
			json["value"] = toJson(directional->getValue());
		}
		else if (auto punctual = dynamic_cast<const PunctualForce*>(&force); punctual) {
			json["type"] = "PunctualForce";
			json["value"] = toJson(punctual->getValue());
			json["point"] = toJson(punctual->getPoint());
		}
	}

	template <>
	Result deserializeResource<Force>(const nlohmann::json& json, const std::string& key, DeserializeData&, Scene& scene)
	{
		std::shared_ptr<Force> force;

		auto itType = json.find("type");
		if (itType == json.end()) {
			return Result(false, "Missing \"type\" property");
		}

		std::string type = *itType;
		if (type == "Gravity") {
			auto itValue = json.find("value");
			if (itValue == json.end()) {
				return Result(false, "Gravity missing \"value\" property");
			}

			force = std::make_shared<Gravity>(itValue->get<float>());
		}
		else if (type == "DirectionalForce") {
			auto itValue = json.find("value");
			if (itValue == json.end()) {
				return Result(false, "DirectionalForce missing \"value\" property");
			}

			glm::vec3 value;
			if (!toVec3(*itValue, value)) {
				return Result(false, "Wrong \"value\" property");
			}

			force = std::make_shared<DirectionalForce>(value);
		}
		else if (type == "PunctualForce") {
			auto itValue = json.find("value");
			if (itValue == json.end()) {
				return Result(false, "PunctualForce missing \"value\" property");
			}

			glm::vec3 value;
			if (!toVec3(*itValue, value)) {
				return Result(false, "Wrong \"value\" property");
			}

			auto itPoint = json.find("point");
			if (itPoint == json.end()) {
				return Result(false, "PunctualForce missing \"point\" property");
			}

			glm::vec3 point;
			if (!toVec3(*itPoint, point)) {
				return Result(false, "Wrong \"point\" property");
			}

			force = std::make_shared<PunctualForce>(value, point);
		}
		else {
			return Result(false, "Wrong \"type\" value = " + type);
		}

		scene.repository.add(key, force);

		return Result();
	}

// Components
	template <typename T>
	nlohmann::json serializeComponent(const T& component, SerializeData& data, std::ostream& dataStream);

	template <typename T>
	ResultOptional<T> deserializeComponent(const nlohmann::json& json, Entity entity, DeserializeData& data, Scene& scene);


	template <>
	nlohmann::json serializeComponent<TagComponent>(const TagComponent& tag, SerializeData&, std::ostream&)
	{
		nlohmann::json json;
		json["name"] = tag.getName();
		return json;
	}

	template <>
	ResultOptional<TagComponent> deserializeComponent<TagComponent>(const nlohmann::json& json, Entity, DeserializeData&, Scene&)
	{
		auto itName = json.find("name");
		if (itName != json.end()) {
			return { Result(), TagComponent(itName->get<std::string>()) };
		}
		else {
			return { Result("Missing name property"), std::nullopt };
		}
	}


	template <>
	nlohmann::json serializeComponent<TransformsComponent>(const TransformsComponent& transforms, SerializeData&, std::ostream&)
	{
		nlohmann::json json;

		if (transforms.position != glm::vec3(0.0f)) {
			json["position"] = toJson(transforms.position);
		}

		if (transforms.velocity != glm::vec3(0.0f)) {
			json["velocity"] = toJson(transforms.velocity);
		}

		if (transforms.orientation != glm::quat(1.0f, glm::vec3(0.0f))) {
			json["orientation"] = toJson(transforms.orientation);
		}

		if (transforms.scale != glm::vec3(1.0f)) {
			json["scale"] = toJson(transforms.scale);
		}

		return json;
	}

	template <>
	ResultOptional<TransformsComponent> deserializeComponent<TransformsComponent>(const nlohmann::json& json, Entity, DeserializeData&, Scene&)
	{
		TransformsComponent transforms;

		auto itPosition = json.find("position");
		if (itPosition != json.end()) {
			toVec3(*itPosition, transforms.position);
		}

		auto itVelocity = json.find("velocity");
		if (itVelocity != json.end()) {
			toVec3(*itVelocity, transforms.velocity);
		}

		auto itOrientation = json.find("orientation");
		if (itOrientation != json.end()) {
			toQuat(*itOrientation, transforms.orientation);
		}

		auto itScale = json.find("scale");
		if (itScale != json.end()) {
			toVec3(*itScale, transforms.scale);
		}

		return { Result(), std::move(transforms) };
	}


	template <>
	nlohmann::json serializeComponent<CameraComponent>(const CameraComponent& camera, SerializeData&, std::ostream&)
	{
		nlohmann::json json;

		json["isOrthographic"] = camera.hasOrthographicProjection();
		if (camera.hasOrthographicProjection()) {
			float left, right, bottom, top, zNear, zFar;
			camera.getOrthographicParams(left, right, bottom, top, zNear, zFar);

			json["left"] = left;
			json["right"] = right;
			json["bottom"] = bottom;
			json["top"] = top;
			json["zNear"] = zNear;
			json["zFar"] = zFar;
		}
		else {
			float fovy, aspectRatio, zNear, zFar;
			camera.getPerspectiveParams(fovy, aspectRatio, zNear, zFar);

			json["fovy"] = fovy;
			json["aspectRatio"] = aspectRatio;
			json["zNear"] = zNear;
			json["zFar"] = zFar;
		}

		return json;
	}

	template <>
	ResultOptional<CameraComponent> deserializeComponent<CameraComponent>(const nlohmann::json& json, Entity, DeserializeData&, Scene&)
	{
		CameraComponent camera;

		auto itIsOrtho = json.find("isOrthographic");
		if (itIsOrtho == json.end()) {
			return { Result(false, "Missing isOrthographic property"), std::nullopt };
		}

		if (*itIsOrtho) {
			auto itLeft = json.find("left"), itRight = json.find("right"), itBottom = json.find("bottom"), itTop = json.find("top"),
				itZNear = json.find("zNear"), itZFar = json.find("zFar");

			if ((itLeft == json.end()) || (itRight == json.end()) || (itBottom == json.end()) || (itTop == json.end())
				|| (itZNear == json.end()) || (itZFar == json.end())
			) {
				return { Result(false, "Missing orthographic properties"), std::nullopt };
			}

			camera.setOrthographicProjection(*itLeft, *itRight, *itBottom, *itTop, *itZNear, *itZFar);
		}
		else {
			auto itFovy = json.find("fovy"), itAspectRatio = json.find("aspectRatio"), itZNear = json.find("zNear"), itZFar = json.find("zFar");

			if ((itFovy == json.end()) || (itAspectRatio == json.end()) || (itZNear == json.end()) || (itZFar == json.end())) {
				return { Result(false, "Missing projection properties"), std::nullopt };
			}

			camera.setPerspectiveProjection(*itFovy, *itAspectRatio, *itZNear, *itZFar);
		}

		return { Result(), std::move(camera) };
	}


	template <>
	nlohmann::json serializeComponent<MeshComponent>(const MeshComponent& mesh, SerializeData& data, std::ostream&)
	{
		nlohmann::json json;

		auto rMeshesJson = nlohmann::json::array();
		mesh.processRenderableIndices([&](std::size_t iMesh) {
			Scene::Key key;
			auto value = mesh.get(iMesh).getMesh();
			if (data.scene.repository.findKey<Scene::Key, Mesh>(value, key)) {
				nlohmann::json meshJson;
				meshJson["key"] = key;
				meshJson["hasSkinning"] = mesh.hasSkinning(iMesh);
				meshJson["primitive"] = static_cast<int>(mesh.get(iMesh).getPrimitiveType());
				rMeshesJson.emplace_back(std::move(meshJson));
			}
		});
		json["rMeshes"] = std::move(rMeshesJson);

		return json;
	}

	template <>
	ResultOptional<MeshComponent> deserializeComponent<MeshComponent>(const nlohmann::json& json, Entity entity, DeserializeData&, Scene& scene)
	{
		MeshComponent mesh(scene.application.getEventManager(), entity);

		auto itRMeshes = json.find("rMeshes");
		if (itRMeshes == json.end()) {
			return { Result(false, "Missing \"rMeshes\" property"), std::nullopt };
		}

		for (std::size_t i = 0; i < itRMeshes->size(); ++i) {
			const auto& rMesh = (*itRMeshes)[i];

			auto itKey = rMesh.find("key");
			auto itHasSkinning = rMesh.find("hasSkinning");
			auto itPrimitive = rMesh.find("primitive");
			if (itKey == rMesh.end()) {
				return { Result(false, "Missing \"key\" property at mesh " + std::to_string(i)), std::nullopt };
			}
			if (itHasSkinning == rMesh.end()) {
				return { Result(false, "Missing \"hasSkinning\" property at mesh " + std::to_string(i)), std::nullopt };
			}
			if (itPrimitive == rMesh.end()) {
				return { Result(false, "Missing \"primitive\" property at mesh " + std::to_string(i)), std::nullopt };
			}

			Scene::Key key = *itKey;
			bool hasSkinning = *itHasSkinning;
			PrimitiveType primitive = static_cast<PrimitiveType>(itPrimitive->get<int>());

			auto value = scene.repository.find<Scene::Key, Mesh>(key);
			if (!value) {
				return { Result(false, "Value not found with key " + key), std::nullopt };
			}

			mesh.add(hasSkinning, value, primitive);
		}

		return { Result(), std::move(mesh) };
	}


	template <>
	nlohmann::json serializeComponent<LightComponent>(const LightComponent& light, SerializeData& data, std::ostream&)
	{
		nlohmann::json json;

		Scene::Key key;
		if (data.scene.repository.findKey(light.source, key)) {
			json["sourceKey"] = key;
		}

		return json;
	}

	template <>
	ResultOptional<LightComponent> deserializeComponent<LightComponent>(const nlohmann::json& json, Entity, DeserializeData&, Scene& scene)
	{
		LightComponent light;

		auto itSourceKey = json.find("sourceKey");
		if (itSourceKey != json.end()) {
			Scene::Key key = *itSourceKey;
			if (auto source = scene.repository.find<Scene::Key, LightSource>(key)) {
				light.source = source;
			}
			else {
				return { Result(false, "Key \"" + key + "\" not found"), std::nullopt };
			}
		}

		return { Result(), std::move(light) };
	}


	template <>
	nlohmann::json serializeComponent<RigidBody>(const RigidBody& rigidBody, SerializeData& data, std::ostream&)
	{
		auto forces = nlohmann::json::array();
		rigidBody.processForces([&](std::shared_ptr<Force> force) {
			Scene::Key key;
			if (data.scene.repository.findKey(force, key)) {
				forces.push_back(key);
			}
		});

		nlohmann::json json;
		json["invertedMass"] = rigidBody.getConfig().invertedMass;
		json["invertedInertiaTensor"] = toJson(rigidBody.getConfig().invertedInertiaTensor);
		json["linearDrag"] = rigidBody.getConfig().linearDrag;
		json["angularDrag"] = rigidBody.getConfig().angularDrag;
		json["frictionCoefficient"] = rigidBody.getConfig().frictionCoefficient;
		json["sleepMotion"] = rigidBody.getConfig().sleepMotion;
		json["forces"] = std::move(forces);
		return json;
	}

	template <>
	ResultOptional<RigidBody> deserializeComponent<RigidBody>(const nlohmann::json& json, Entity, DeserializeData&, Scene& scene)
	{
		RigidBody rigidBody;

		auto itInvertedMass = json.find("invertedMass"), itInvertedInertiaTensor = json.find("invertedInertiaTensor"),
			itLinearDrag = json.find("linearDrag"), itAngularDrag = json.find("angularDrag"),
			itFrictionCoefficient = json.find("frictionCoefficient"), itSleepMotion = json.find("sleepMotion"),
			itForces = json.find("forces");

		if ((itInvertedMass == json.end()) || (itInvertedInertiaTensor == json.end())
			|| (itLinearDrag == json.end()) || (itAngularDrag == json.end())
			|| (itFrictionCoefficient == json.end()) || (itSleepMotion == json.end())
			|| (itForces == json.end())
		) {
			return { Result(false, "Missing properties"), std::nullopt };
		}

		rigidBody.getConfig().invertedMass = *itInvertedMass;
		if (!toMat3(*itInvertedInertiaTensor, rigidBody.getConfig().invertedInertiaTensor)) {
			return { Result(false, "Failed to parse the invertedInertiaTensor"), std::nullopt };
		}

		rigidBody.getConfig().linearDrag = *itLinearDrag;
		rigidBody.getConfig().angularDrag = *itAngularDrag;
		rigidBody.getConfig().frictionCoefficient = *itFrictionCoefficient;
		rigidBody.getConfig().sleepMotion = *itSleepMotion;

		for (std::size_t i = 0; i < itForces->size(); ++i) {
			auto forceJson = (*itForces)[i];
			auto force = scene.repository.find<Scene::Key, Force>(forceJson.get<std::string>());
			if (force) {
				rigidBody.addForce(force);
			}
			else {
				return { Result(false, "Failed to parse Force[" + std::to_string(i) + "]: Key " + forceJson.get<std::string>() + " not found"), std::nullopt };
			}
		}

		return { Result(), std::move(rigidBody) };
	}


	template <>
	nlohmann::json serializeComponent<Collider>(const Collider& collider, SerializeData& data, std::ostream& dataStream)
	{
		nlohmann::json json;

		if (auto bbox = dynamic_cast<const BoundingBox*>(&collider)) {
			json["type"] = "BoundingBox";
			json["lengths"] = toJson(bbox->getLengths());
		}
		else if (auto bsphere = dynamic_cast<const BoundingSphere*>(&collider)) {
			json["type"] = "BoundingSphere";
			json["radius"] = bsphere->getRadius();
		}
		else if (auto capsule = dynamic_cast<const Capsule*>(&collider)) {
			json["type"] = "Capsule";
			json["radius"] = capsule->getRadius();
			json["height"] = capsule->getHeight();
		}
		else if (auto triangle = dynamic_cast<const TriangleCollider*>(&collider)) {
			json["type"] = "TriangleCollider";
			json["localVertices"] = {
				toJson(triangle->getLocalVertices()[0]),
				toJson(triangle->getLocalVertices()[1]),
				toJson(triangle->getLocalVertices()[2])
			};
		}
		else if (auto cPoly = dynamic_cast<const ConvexPolyhedron*>(&collider)) {
			json["type"] = "ConvexPolyhedron";

			HalfEdgeMesh mesh = cPoly->getLocalMesh();

			json["vertices_size"] = mesh.vertices.size();

			nlohmann::json verticesBuffer;
			serializeBuffer(
				reinterpret_cast<const std::byte*>(mesh.vertices.data()),
				(mesh.vertices.size() + mesh.vertices.numReleasedIndices()) * sizeof(HEVertex), verticesBuffer, dataStream
			);
			data.buffersJson.emplace_back(std::move(verticesBuffer));
			json["vertices_elements"] = data.buffersJson.size() - 1;

			nlohmann::json releasedVerticesBuffer;
			serializeBuffer(
				reinterpret_cast<const std::byte*>(mesh.vertices.releasedIndices()),
				mesh.vertices.numReleasedIndices() * sizeof(std::size_t), releasedVerticesBuffer, dataStream
			);
			data.buffersJson.emplace_back(std::move(releasedVerticesBuffer));
			json["vertices_released"] = data.buffersJson.size() - 1;

			json["edges_size"] = mesh.edges.size();

			nlohmann::json edgesBuffer;
			serializeBuffer(
				reinterpret_cast<const std::byte*>(mesh.edges.data()),
				(mesh.edges.size() + mesh.edges.numReleasedIndices()) * sizeof(HEEdge), edgesBuffer, dataStream
			);
			data.buffersJson.emplace_back(std::move(edgesBuffer));
			json["edges_elements"] = data.buffersJson.size() - 1;

			nlohmann::json releasedEdgesBuffer;
			serializeBuffer(
				reinterpret_cast<const std::byte*>(mesh.edges.releasedIndices()),
				mesh.edges.numReleasedIndices() * sizeof(std::size_t), releasedEdgesBuffer, dataStream
			);
			data.buffersJson.emplace_back(std::move(releasedEdgesBuffer));
			json["edges_released"] = data.buffersJson.size() - 1;

			json["faces_size"] = mesh.faces.size();

			nlohmann::json facesBuffer;
			serializeBuffer(
				reinterpret_cast<const std::byte*>(mesh.faces.data()),
				(mesh.faces.size() + mesh.faces.numReleasedIndices()) * sizeof(HEFace), facesBuffer, dataStream
			);
			data.buffersJson.emplace_back(std::move(facesBuffer));
			json["faces_elements"] = data.buffersJson.size() - 1;

			nlohmann::json releasedFacesBuffer;
			serializeBuffer(
				reinterpret_cast<const std::byte*>(mesh.faces.releasedIndices()),
				mesh.faces.numReleasedIndices() * sizeof(std::size_t), releasedFacesBuffer, dataStream
			);
			data.buffersJson.emplace_back(std::move(releasedFacesBuffer));
			json["faces_released"] = data.buffersJson.size() - 1;

			std::vector<int> vertexEdgeMap;
			vertexEdgeMap.reserve(3 * mesh.vertexEdgeMap.size());
			for (const auto& [vertices, edge] : mesh.vertexEdgeMap) {
				vertexEdgeMap.push_back(vertices.first);
				vertexEdgeMap.push_back(vertices.second);
				vertexEdgeMap.push_back(edge);
			}

			nlohmann::json vertexEdgeBuffer;
			serializeBuffer(
				reinterpret_cast<const std::byte*>(vertexEdgeMap.data()),
				vertexEdgeMap.size() * sizeof(int), vertexEdgeBuffer, dataStream
			);
			data.buffersJson.emplace_back(std::move(vertexEdgeBuffer));
			json["vertexEdgeMap"] = data.buffersJson.size() - 1;
		}
		else if (auto terrain = dynamic_cast<const TerrainCollider*>(&collider)) {
			nlohmann::json bufferJson;
			serializeBuffer(
				reinterpret_cast<const std::byte*>(terrain->getHeights()),
				terrain->getXSize() * terrain->getZSize() * sizeof(float),
				bufferJson, dataStream
			);
			data.buffersJson.emplace_back(std::move(bufferJson));

			json["type"] = "TerrainCollider";
			json["xSize"] = terrain->getXSize();
			json["zSize"] = terrain->getZSize();
			json["heights"] = data.buffersJson.size() - 1;
		}
		else if (auto composite = dynamic_cast<const CompositeCollider*>(&collider)) {
			auto colliderPartsJson = nlohmann::json::array();
			composite->processParts([&](const Collider& part) {
				colliderPartsJson.emplace_back( serializeComponent<Collider>(part, data, dataStream) );
			});

			json["type"] = "CompositeCollider";
			json["parts"] = std::move(colliderPartsJson);
		}

		return json;
	}

	template <>
	ResultOptional<std::unique_ptr<Collider>> deserializeComponent<std::unique_ptr<Collider>>(const nlohmann::json& json, Entity entity, DeserializeData& data, Scene& scene)
	{
		std::unique_ptr<Collider> collider;

		auto itType = json.find("type");
		if (itType == json.end()) {
			return { Result(false, "Missing \"type\" property"), std::nullopt };
		}

		if (*itType == "BoundingBox") {
			auto itLengths = json.find("lengths");
			if (itLengths == json.end()) {
				return { Result(false, "Missing BoundingBox \"lengths\" property"), std::nullopt };
			}

			glm::vec3 lengths;
			if (!toVec3(*itLengths, lengths)) {
				return { Result(false, "Failed to parse BoundingBox \"lengths\" property"), std::nullopt };
			}

			collider = std::make_unique<BoundingBox>(lengths);
		}
		else if (*itType == "BoundingSphere") {
			auto itRadius = json.find("radius");
			if (itRadius == json.end()) {
				return { Result(false, "Missing BoundingSphere \"radius\" property"), std::nullopt };
			}

			collider = std::make_unique<BoundingSphere>(itRadius->get<float>());
		}
		else if (*itType == "Capsule") {
			auto itRadius = json.find("radius");
			if (itRadius == json.end()) {
				return { Result(false, "Missing Capsule \"radius\" property"), std::nullopt };
			}
			auto itHeight = json.find("height");
			if (itHeight == json.end()) {
				return { Result(false, "Missing Capsule \"height\" property"), std::nullopt };
			}

			collider = std::make_unique<Capsule>(itRadius->get<float>(), itHeight->get<float>());
		}
		else if (*itType == "TriangleCollider") {
			auto itVertices = json.find("localVertices");
			if (itVertices == json.end()) {
				return { Result(false, "Missing TriangleCollider \"localVertices\" property"), std::nullopt };
			}

			if (itVertices->size() < 3) {
				return { Result(false, "Not enough localVertices"), std::nullopt };
			}

			std::array<glm::vec3, 3> vertices;
			for (std::size_t i = 0; i < 3; ++i) {
				if (!toVec3((*itVertices)[i], vertices[i])) {
					return { Result(false, "Failed to parse the \"localVertices\" property"), std::nullopt };
				}
			}

			collider = std::make_unique<TriangleCollider>(vertices);
		}
		else if (*itType == "ConvexPolyhedron") {
			auto itVerticesSize = json.find("vertices_size");
			auto itVerticesElements = json.find("vertices_elements");
			auto itVerticesReleased = json.find("vertices_released");
			if ((itVerticesSize == json.end()) || (itVerticesElements == json.end()) || (itVerticesReleased == json.end())) {
				return { Result(false, "Missing vertices properties"), std::nullopt };
			}

			std::size_t verticesSize = *itVerticesSize;
			std::size_t verticesElements = *itVerticesElements;
			std::size_t verticesReleased = *itVerticesReleased;
			if ((verticesElements >= data.buffersJson.size()) || (verticesReleased >= data.buffersJson.size())) {
				return { Result(false, "Vertices properties out of bounds"), std::nullopt };
			}

			std::vector<std::byte> vElementsBuffer, vReleasedBuffer;
			deserializeBuffer(data.buffersJson[verticesElements], data.dataStream, vElementsBuffer);
			deserializeBuffer(data.buffersJson[verticesReleased], data.dataStream, vReleasedBuffer);

			auto itEdgesSize = json.find("edges_size");
			auto itEdgesElements = json.find("edges_elements");
			auto itEdgesReleased = json.find("edges_released");
			if ((itEdgesSize == json.end()) || (itEdgesElements == json.end()) || (itEdgesReleased == json.end())) {
				return { Result(false, "Missing edges properties"), std::nullopt };
			}

			std::size_t edgesSize = *itEdgesSize;
			std::size_t edgesElements = *itEdgesElements;
			std::size_t edgesReleased = *itEdgesReleased;
			if ((edgesElements >= data.buffersJson.size()) || (edgesReleased >= data.buffersJson.size())) {
				return { Result(false, "Edges properties out of bounds"), std::nullopt };
			}

			std::vector<std::byte> eElementsBuffer, eReleasedBuffer;
			deserializeBuffer(data.buffersJson[edgesElements], data.dataStream, eElementsBuffer);
			deserializeBuffer(data.buffersJson[edgesReleased], data.dataStream, eReleasedBuffer);

			auto itFacesSize = json.find("faces_size");
			auto itFacesElements = json.find("faces_elements");
			auto itFacesReleased = json.find("faces_released");
			if ((itFacesSize == json.end()) || (itFacesElements == json.end()) || (itFacesReleased == json.end())) {
				return { Result(false, "Missing faces properties"), std::nullopt };
			}

			std::size_t facesSize = *itFacesSize;
			std::size_t facesElements = *itFacesElements;
			std::size_t facesReleased = *itFacesReleased;
			if ((facesElements >= data.buffersJson.size()) || (facesReleased >= data.buffersJson.size())) {
				return { Result(false, "Faces properties out of bounds"), std::nullopt };
			}

			std::vector<std::byte> fElementsBuffer, fReleasedBuffer;
			deserializeBuffer(data.buffersJson[facesElements], data.dataStream, fElementsBuffer);
			deserializeBuffer(data.buffersJson[facesReleased], data.dataStream, fReleasedBuffer);

			auto itVertexEdgeMap = json.find("vertexEdgeMap");
			if (itVertexEdgeMap == json.end()) {
				return { Result(false, "Missing \"vertexEdgeMap\" property"), std::nullopt };
			}

			std::size_t vertexEdgeMap = *itVertexEdgeMap;

			std::vector<std::byte> veMapBuffer;
			deserializeBuffer(data.buffersJson[vertexEdgeMap], data.dataStream, veMapBuffer);
			auto intBuff = reinterpret_cast<const int*>(veMapBuffer.data());

			HalfEdgeMesh mesh;
			mesh.vertices = utils::PackedVector<HEVertex>(
				reinterpret_cast<const HEVertex*>(vElementsBuffer.data()), vElementsBuffer.size() / sizeof(HEVertex), verticesSize,
				reinterpret_cast<const std::size_t*>(vReleasedBuffer.data()), vReleasedBuffer.size() / sizeof(std::size_t)
			);
			mesh.edges = utils::PackedVector<HEEdge>(
				reinterpret_cast<const HEEdge*>(eElementsBuffer.data()), eElementsBuffer.size() / sizeof(HEEdge), edgesSize,
				reinterpret_cast<const std::size_t*>(eReleasedBuffer.data()), eReleasedBuffer.size() / sizeof(std::size_t)
			);
			mesh.faces = utils::PackedVector<HEFace>(
				reinterpret_cast<const HEFace*>(fElementsBuffer.data()), fElementsBuffer.size() / sizeof(HEFace), facesSize,
				reinterpret_cast<const std::size_t*>(fReleasedBuffer.data()), fReleasedBuffer.size() / sizeof(std::size_t)
			);
			mesh.vertexEdgeMap.reserve(veMapBuffer.size() / sizeof(int));
			for (std::size_t i = 0; i < veMapBuffer.size() / sizeof(int); i+=3) {
				mesh.vertexEdgeMap.emplace(std::make_pair(intBuff[i], intBuff[i+1]), intBuff[i+2]);
			}

			collider = std::make_unique<ConvexPolyhedron>(std::move(mesh));
		}
		else if (*itType == "TerrainCollider") {
			auto itHeights = json.find("heights");
			if (itHeights == json.end()) {
				return { Result(false, "Missing TriangleCollider \"heights\" property"), std::nullopt };
			}

			std::size_t heights = *itHeights;
			if (*itHeights >= data.buffersJson.size()) {
				return { Result(false, "Heights buffer " + std::to_string(heights) + " out of bounds"), std::nullopt };
			}

			auto itXSize = json.find("xSize");
			if (itXSize == json.end()) {
				return { Result(false, "Missing TriangleCollider \"xSize\" property"), std::nullopt };
			}

			auto itZSize = json.find("zSize");
			if (itZSize == json.end()) {
				return { Result(false, "Missing TriangleCollider \"zSize\" property"), std::nullopt };
			}

			std::vector<std::byte> buffer;
			deserializeBuffer(data.buffersJson[heights], data.dataStream, buffer);

			auto terrain = std::make_unique<TerrainCollider>();
			terrain->setHeights(reinterpret_cast<const float*>(buffer.data()), *itXSize, *itZSize);
			collider = std::move(collider);
		}
		else if (*itType == "CompositeCollider") {
			auto itParts = json.find("parts");
			if (itParts == json.end()) {
				return { Result(false, "Missing CompositeCollider \"parts\" property"), std::nullopt };
			}

			std::vector<std::unique_ptr<Collider>> parts;
			for (std::size_t i = 0; i < itParts->size(); ++i) {
				auto jsonPart = (*itParts)[i];
				auto [result, part] = deserializeComponent<std::unique_ptr<Collider>>(jsonPart, entity, data, scene);
				if (!result) {
					return { Result(false, "Failed to parse CompositeCollider at parts[" + std::to_string(i) + "]:" + result.description()), std::nullopt };
				}
				parts.emplace_back(std::move(*part));
			}
			collider = std::make_unique<CompositeCollider>(std::move(parts));
		}
		else {
			return { Result(false, "Wrong type: " + itType->get<std::string>()), std::nullopt };
		}

		return { Result(), std::move(collider) };
	}


	template <>
	nlohmann::json serializeComponent<SkinComponent>(const SkinComponent& skin, SerializeData& data, std::ostream&)
	{
		nlohmann::json json;

		Scene::Key key;
		auto itNodeIndex = data.nodeIndexMap.find(skin.getRoot());
		if ((itNodeIndex != data.nodeIndexMap.end()) && data.scene.repository.findKey(skin.getSkin(), key)) {
			auto mapNodeJointJson = nlohmann::json::array();
			for (auto [node, jointIndex] : skin.getMapNodeJoint()) {
				auto itNode = data.nodeIndexMap.find(node);
				if (itNode != data.nodeIndexMap.end()) {
					nlohmann::json pairJson;
					pairJson["node"] = itNode->second;
					pairJson["joint"] = jointIndex;
					mapNodeJointJson.emplace_back(std::move(pairJson));
				}
			}

			json["root"] = itNodeIndex->second;
			json["mapNodeJoint"] = std::move(mapNodeJointJson);
			json["skin"] = key;
		}

		return json;
	}

	template <>
	ResultOptional<SkinComponent> deserializeComponent<SkinComponent>(const nlohmann::json& json, Entity, DeserializeData& data, Scene& scene)
	{
		auto itRoot = json.find("root");
		if (itRoot == json.end()) {
			return { Result(false, "Missing \"root\" property"), std::nullopt };
		}

		auto itSkin = json.find("skin");
		if (itSkin == json.end()) {
			return { Result(false, "Missing \"skin\" property"), std::nullopt };
		}

		auto itMapNodeJoint = json.find("mapNodeJoint");
		if (itMapNodeJoint == json.end()) {
			return { Result(false, "Missing \"mapNodeJoint\" property"), std::nullopt };
		}

		auto itRoot2 = data.indexNodeMap.find(*itRoot);
		if (itRoot2 == data.indexNodeMap.end()) {
			return { Result(false, "Node " + std::to_string(itRoot->get<std::size_t>()) + " not found"), std::nullopt };
		}

		auto skin = scene.repository.find<Scene::Key, Skin>(*itSkin);
		if (!skin) {
			return { Result(false, "Skin \"" + itSkin->get<std::string>() + "\" not found"), std::nullopt };
		}

		SkinComponent::MapNodeJoint mapNodeJoint;
		for (std::size_t i = 0; i < itMapNodeJoint->size(); ++i) {
			auto itNode = (*itMapNodeJoint)[i].find("node");
			if (itNode == (*itMapNodeJoint)[i].end()) {
				return { Result(false, "Missing \"node\" property on mapNodeJoint[" + std::to_string(i) + "]"), std::nullopt };
			}

			auto itJoint = (*itMapNodeJoint)[i].find("joint");
			if (itJoint == (*itMapNodeJoint)[i].end()) {
				return { Result(false, "Missing \"joint\" property on mapNodeJoint[" + std::to_string(i) + "]"), std::nullopt };
			}

			auto itNode2 = data.indexNodeMap.find(*itNode);
			if (itNode2 == data.indexNodeMap.end()) {
				return { Result(false, "Node " + std::to_string(itNode->get<std::size_t>()) + " not found on mapNodeJoint[" + std::to_string(i) + "]"), std::nullopt };
			}

			mapNodeJoint.emplace_back(itNode2->second, itJoint->get<std::size_t>());
		}

		return { Result(), SkinComponent(itRoot2->second, std::move(skin), std::move(mapNodeJoint)) };
	}


	template <>
	nlohmann::json serializeComponent<AnimationComponent>(const AnimationComponent& animation, SerializeData& data, std::ostream&)
	{
		nlohmann::json json;

		auto itNodeIndex = data.nodeIndexMap.find(animation.getRootNode());
		if (itNodeIndex != data.nodeIndexMap.end()) {
			auto animatorsJson = nlohmann::json::array();
			animation.processSAnimators([&](std::shared_ptr<SkeletonAnimator> sAnimator) {
				Scene::Key key;
				if (data.scene.repository.findKey(sAnimator, key)) {
					animatorsJson.emplace_back(key);
				}
			});

			json["root"] = itNodeIndex->second;
			json["animators"] = std::move(animatorsJson);
		}

		return json;
	}

	template <>
	ResultOptional<AnimationComponent> deserializeComponent<AnimationComponent>(const nlohmann::json& json, Entity, DeserializeData& data, Scene& scene)
	{
		auto itRoot = json.find("root");
		if (itRoot == json.end()) {
			return { Result(false, "Missing \"root\" property"), std::nullopt };
		}

		auto itAnimators = json.find("animators");
		if (itAnimators == json.end()) {
			return { Result(false, "Missing \"animators\" property"), std::nullopt };
		}

		auto itRoot2 = data.indexNodeMap.find(*itRoot);
		if (itRoot2 == data.indexNodeMap.end()) {
			return { Result(false, "Node " + std::to_string(itRoot->get<std::size_t>()) + " not found"), std::nullopt };
		}

		AnimationComponent animation(itRoot2->second);
		for (std::size_t i = 0; i < itAnimators->size(); ++i) {
			auto animator = scene.repository.find<Scene::Key, SkeletonAnimator>((*itAnimators)[i]);
			if (!animator) {
				return { Result(false, "SkeletonAnimator " + (*itAnimators)[i].get<std::string>() + " not found"), std::nullopt };
			}

			animation.addAnimator(animator);
		}
		return { Result(), std::move(animation) };
	}

// Other
	void serializeAnimationNode(
		const AnimationNode& node, const std::unordered_map<const AnimationNode*, std::size_t>& nodeIndexMap,
		nlohmann::json& json
	) {
		auto& nodeData = node.getData();

		json["name"] = nodeData.name.data();

		nlohmann::json nodeLocalJson;
		nodeLocalJson["position"] = toJson(nodeData.localTransforms.position);
		nodeLocalJson["orientation"] = toJson(nodeData.localTransforms.orientation);
		nodeLocalJson["scale"] = toJson(nodeData.localTransforms.scale);
		json["localTransforms"] = nodeLocalJson;

		auto childrenJson = nlohmann::json::array();
		for (auto it = node.cbegin<Traversal::Children>(); it != node.cend<Traversal::Children>(); ++it) {
			childrenJson.emplace_back(nodeIndexMap.find(&(*it))->second);
		}
		json["children"] = std::move(childrenJson);
	}

	Result deserializeAnimationNode(
		const nlohmann::json& json,
		AnimationNode& node, std::vector<std::size_t>& childrenIndices
	) {
		auto& nodeData = node.getData();

		auto itName = json.find("name");
		if (itName != json.end()) {
			std::string name = *itName;
			std::copy(name.begin(), name.end(), nodeData.name.begin());
		}
		else {
			return Result(false, "Missing name");
		}

		auto itLocal = json.find("localTransforms");
		if (itLocal != json.end()) {
			auto itPosition = itLocal->find("position");
			if (itPosition != itLocal->end()) {
				toVec3(*itPosition, nodeData.localTransforms.position);
			}
			auto itOrientation = itLocal->find("orientation");
			if (itOrientation != itLocal->end()) {
				toQuat(*itOrientation, nodeData.localTransforms.orientation);
			}
			auto itScale = itLocal->find("scale");
			if (itScale != itLocal->end()) {
				toVec3(*itScale, nodeData.localTransforms.scale);
			}
		}
		else {
			return Result(false, "Missing local transforms");
		}

		auto itChildren = json.find("children");
		if (itChildren != json.end()) {
			childrenIndices = itChildren->get<std::vector<std::size_t>>();
		}
		else {
			return Result(false, "Missing children");
		}

		return Result();
	}


	template <typename T>
	void serializeRVector(const std::string& tag, SerializeData& data, nlohmann::json& json, std::ostream& dataStream)
	{
		auto resourcesVJson = nlohmann::json::array();

		data.scene.repository.iterate<Scene::Key, T>([&](const Scene::Key& k, const std::shared_ptr<T>& v) {
			nlohmann::json resourceJson;
			resourceJson["key"] = k;
			serializeResource<T>(*v, k, data, resourceJson, dataStream);
			resourcesVJson.emplace_back(std::move(resourceJson));
		});

		if (!resourcesVJson.empty()) {
			json[tag] = std::move(resourcesVJson);
		}
	}

	template <typename T>
	Result deserializeRVector(const std::string& tag, DeserializeData& data, Scene& scene)
	{
		auto it = data.json.find(tag);
		if (it != data.json.end()) {
			for (std::size_t i = 0; i < it->size(); ++i) {
				auto& resourceJson = (*it)[i];

				auto itKey = resourceJson.find("key");
				if (itKey == resourceJson.end()) {
					return Result(false, "Missing key");
				}

				if (auto result = deserializeResource<T>(resourceJson, *itKey, data, scene); !result) {
					return Result(false, "Failed to deserialize " + tag + "[" + std::to_string(i) + "]: " + result.description());
				}
			}
		}

		return Result();
	}


	template <typename T>
	void serializeCVector(const std::string& tag, SerializeData& data, nlohmann::json& json, std::ostream& dataStream)
	{
		auto componentsVJson = nlohmann::json::array();

		for (Entity entity : data.scene.entities) {
			std::size_t index = data.entityIndexMap.find(entity)->second;
			auto [component] = data.scene.application.getEntityDatabase().getComponents<T>(entity);

			if (component) {
				nlohmann::json componentJson = serializeComponent<T>(*component, data, dataStream);
				componentJson["entity"] = index;
				componentsVJson.emplace_back(std::move(componentJson));
			}
		}

		if (!componentsVJson.empty()) {
			json[tag] = std::move(componentsVJson);
		}
	}

	template <typename T, bool hasDerived = false>
	Result deserializeCVector(const std::string& tag, DeserializeData& data, Scene& scene)
	{
		auto it = data.json.find(tag);
		if (it != data.json.end()) {
			for (std::size_t i = 0; i < it->size(); ++i) {
				auto& componentJson = (*it)[i];

				auto itEntity = componentJson.find("entity");
				if (itEntity == componentJson.end()) {
					return Result(false, "Failed to deserialize " + tag + "[" + std::to_string(i) + "]: Missing \"entity\" property");
				}

				std::size_t iEntity = itEntity->get<std::size_t>();
				auto itEntity2 = data.indexEntityMap.find(iEntity);
				if (itEntity2 == data.indexEntityMap.end()) {
					return Result(false, "Failed to deserialize " + tag + "[" + std::to_string(i) + "]: Entity " + std::to_string(iEntity) + " not found");
				}

				if constexpr (hasDerived) {
					auto [result, component] = deserializeComponent<std::unique_ptr<T>>(componentJson, itEntity2->second, data, scene);
					if (!result) {
						return Result(false, "Failed to deserialize " + tag + "[" + std::to_string(i) + "]: " + result.description());
					}
					scene.application.getEntityDatabase().addComponent<T>(itEntity2->second, std::move(*component));
				}
				else {
					auto [result, component] = deserializeComponent<T>(componentJson, itEntity2->second, data, scene);
					if (!result) {
						return Result(false, "Failed to deserialize " + tag + "[" + std::to_string(i) + "]: " + result.description());
					}
					scene.application.getEntityDatabase().addComponent<T>(itEntity2->second, std::move(*component));
				}
			}
		}

		return Result();
	}


	void serializeRepository(SerializeData& data, nlohmann::json& json, std::ostream& dataStream)
	{
		serializeRVector<Mesh>("meshes", data, json, dataStream);
		serializeRVector<Skin>("skins", data, json, dataStream);
		serializeRVector<SkeletonAnimator>("skeletonAnimators", data, json, dataStream);
		serializeRVector<LightSource>("lightSources", data, json, dataStream);
		serializeRVector<Program>("programs", data, json, dataStream);
		serializeRVector<Texture>("textures", data, json, dataStream);
		serializeRVector<Force>("forces", data, json, dataStream);
	}

	Result deserializeRepository(DeserializeData& data, Scene& scene)
	{
		if (auto result = deserializeRVector<Mesh>("meshes", data, scene); !result) { return result; }
		if (auto result = deserializeRVector<Skin>("skins", data, scene); !result) { return result; }
		if (auto result = deserializeRVector<SkeletonAnimator>("skeletonAnimators", data, scene); !result) { return result; }
		if (auto result = deserializeRVector<LightSource>("lightSources", data, scene); !result) { return result; }
		if (auto result = deserializeRVector<Program>("programs", data, scene); !result) { return result; }
		if (auto result = deserializeRVector<Texture>("textures", data, scene); !result) { return result; }
		if (auto result = deserializeRVector<Force>("forces", data, scene); !result) { return result; }
		return Result();
	}


	void serializeNodes(SerializeData& data, nlohmann::json& json)
	{
		std::size_t numNodes = 0;
		for (auto itNode = data.scene.rootNode.cbegin(); itNode != data.scene.rootNode.cend(); ++itNode) {
			data.nodeIndexMap.emplace(&(*itNode), numNodes++);
		}

		auto nodesVJson = nlohmann::json::array();
		for (auto itNode = data.scene.rootNode.cbegin(); itNode != data.scene.rootNode.cend(); ++itNode) {
			nlohmann::json nodeJson;
			serializeAnimationNode(*itNode, data.nodeIndexMap, nodeJson);
			nodesVJson.emplace_back(std::move(nodeJson));
		}

		if (!nodesVJson.empty()) {
			json["nodes"] = std::move(nodesVJson);
		}
	}

	Result deserializeNodes(DeserializeData& data, Scene& scene)
	{
		auto itNodes = data.json.find("nodes");
		if (itNodes != data.json.end()) {
			std::vector<std::unique_ptr<AnimationNode>> nodes(itNodes->size());
			std::unordered_map<AnimationNode*, std::vector<std::size_t>> childrenIndicesMap;

			data.indexNodeMap.reserve(itNodes->size());
			childrenIndicesMap.reserve(itNodes->size());
			for (std::size_t i = 0; i < itNodes->size(); ++i) {
				auto node = std::make_unique<AnimationNode>();
				std::vector<std::size_t> childrenIndices;
				auto result = deserializeAnimationNode((*itNodes)[i], *node, childrenIndices);
				if (!result) {
					return Result(false, "Failed to parse the node " + std::to_string(i) + ": " + result.description());
				}

				data.indexNodeMap[i] = node.get();
				childrenIndicesMap[node.get()] = std::move(childrenIndices);
				nodes[i] = std::move(node);
			}

			for (auto& [node, childrenIndices] : childrenIndicesMap) {
				for (auto childIndex : childrenIndices) {
					node->insert(node->cend(), std::move(nodes[childIndex]));
				}
			}

			for (auto& node : nodes) {
				if (node) {
					auto it = scene.rootNode.insert(scene.rootNode.cend(), std::move(node));
					updateWorldTransforms(*it);
				}
			}
		}

		return Result();
	}


	void serializeEntities(SerializeData& data, nlohmann::json& json)
	{
		json["numEntities"] = data.scene.entities.size();

		data.entityIndexMap.reserve(data.scene.entities.size());
		for (std::size_t i = 0; i < data.scene.entities.size(); ++i) {
			data.entityIndexMap.emplace(data.scene.entities[i], i);
		}
	}

	Result deserializeEntities(DeserializeData& data, Scene& scene)
	{
		auto itNumEntities = data.json.find("numEntities");
		if (itNumEntities == data.json.end()) {
			return Result(false, "Missing numEntities property");
		}

		std::size_t numEntities = *itNumEntities;
		scene.entities.reserve(scene.entities.size() + numEntities);
		data.indexEntityMap.reserve(numEntities);
		for (std::size_t i = 0; i < numEntities; ++i) {
			Entity entity = scene.application.getEntityDatabase().addEntity();
			if (entity != kNullEntity) {
				scene.entities.push_back(entity);
				data.indexEntityMap.emplace(i, entity);
			}
			else {
				return Result(false, "Can't add more entities");
			}
		}

		return Result();
	}


	void serializeComponents(SerializeData& data, nlohmann::json& json, std::ostream& dataStream)
	{
		serializeCVector<TagComponent>("tags", data, json, dataStream);
		serializeCVector<TransformsComponent>("transforms", data, json, dataStream);
		serializeCVector<CameraComponent>("cameras", data, json, dataStream);
		serializeCVector<MeshComponent>("meshComponents", data, json, dataStream);
		serializeCVector<LightComponent>("lights", data, json, dataStream);
		serializeCVector<RigidBody>("rigidBodies", data, json, dataStream);
		serializeCVector<Collider>("colliders", data, json, dataStream);
		serializeCVector<SkinComponent>("skinComponents", data, json, dataStream);
		serializeCVector<AnimationComponent>("animationComponents", data, json, dataStream);
	}

	Result deserializeComponents(DeserializeData& data, Scene& scene)
	{
		if (auto result = deserializeCVector<TagComponent>("tags", data, scene); !result) { return result; }
		if (auto result = deserializeCVector<TransformsComponent>("transforms", data, scene); !result) { return result; }
		if (auto result = deserializeCVector<CameraComponent>("cameras", data, scene); !result) { return result; }
		if (auto result = deserializeCVector<MeshComponent>("meshComponents", data, scene); !result) { return result; }
		if (auto result = deserializeCVector<LightComponent>("lights", data, scene); !result) { return result; }
		if (auto result = deserializeCVector<RigidBody>("rigidBodies", data, scene); !result) { return result; }
		if (auto result = deserializeCVector<Collider, true>("colliders", data, scene); !result) { return result; }
		if (auto result = deserializeCVector<SkinComponent>("skinComponents", data, scene); !result) { return result; }
		if (auto result = deserializeCVector<AnimationComponent>("animationComponents", data, scene); !result) { return result; }
		return Result();
	}


	Result SceneSerializer::serialize(const std::string& path, const Scene& scene)
	{
		std::ofstream outputJSONStream(path);
		std::ofstream outputDATAStream(path + ".dat", std::ios::binary);
		if (!outputJSONStream.good() || !outputDATAStream.good()) {
			return Result(false, "Can't open the file");
		}

		nlohmann::json outputJson;
		SerializeData data = { scene, {}, {}, {}, {} };

		serializeRepository(data, outputJson, outputDATAStream);
		serializeNodes(data, outputJson);
		serializeEntities(data, outputJson);
		serializeComponents(data, outputJson, outputDATAStream);

		outputJson["buffers"] = std::move(data.buffersJson);
		outputJson["accessors"] = std::move(data.accessorsJson);

		outputJSONStream << std::setw(4) << outputJson;
		return Result();
	}


	Result SceneSerializer::deserialize(const std::string& path, Scene& output)
	{
		std::ifstream inputJSONStream(path);
		if (!inputJSONStream.good()) {
			return Result(false, "Can't open the file");
		}

		std::ifstream dataStream(path + ".dat", std::ios::binary);
		if (!dataStream.good()) {
			return Result(false, "Can't open the data file");
		}

		nlohmann::json json;
		try {
			inputJSONStream >> json;
		}
		catch(const nlohmann::json::parse_error& e) {
			return Result(false, "Failed to parse the JSON file: " + std::string(e.what()));
		}

		auto itBuffers = json.find("buffers");
		if (itBuffers == json.end()) {
			return Result(false, "Missing \"buffers\" property");
		}
		auto itAccessors = json.find("accessors");
		if (itAccessors == json.end()) {
			return Result(false, "Missing \"accessors\" property");
		}

		auto& buffers = *itBuffers;
		auto& accessors = *itAccessors;
		DeserializeData data = { std::move(json), std::move(dataStream), {}, {}, buffers, accessors };
		if (auto result = deserializeRepository(data, output); !result) {
			return Result(false, "Failed to deserialized the repository: " + std::string(result.description()));
		}
		if (auto result = deserializeNodes(data, output); !result) {
			return Result(false, "Failed to deserialized the nodes: " + std::string(result.description()));
		}
		if (auto result = deserializeEntities(data, output); !result) {
			return Result(false, "Failed to deserialized the entities: " + std::string(result.description()));
		}
		if (auto result = deserializeComponents(data, output); !result) {
			return Result(false, "Failed to deserialized the components: " + std::string(result.description()));
		}

		return Result();
	}

}
