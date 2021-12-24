#include <fstream>
#include <sstream>
#include <iomanip>
#include <optional>
#include <AudioFile.h>
#include "GLMJSON.h"
#include "se/physics/forces/Gravity.h"
#include "se/physics/forces/PunctualForce.h"
#include "se/physics/forces/DirectionalForce.h"
#include "se/physics/forces/Gravity.h"
#include "se/physics/collision/BoundingBox.h"
#include "se/physics/collision/BoundingSphere.h"
#include "se/physics/collision/Capsule.h"
#include "se/physics/collision/TriangleCollider.h"
#include "se/physics/collision/TerrainCollider.h"
#include "se/physics/collision/TriangleMeshCollider.h"
#include "se/physics/collision/CompositeCollider.h"
#include "se/animation/SkeletonAnimator.h"
#include "se/animation/StepAnimations.h"
#include "se/animation/LinearAnimations.h"
#include "se/animation/CubicSplineAnimations.h"
#include "se/graphics/core/UniformVariable.h"
#include "se/graphics/core/GraphicsOperations.h"
#include "se/graphics/GraphicsEngine.h"
#include "se/graphics/Renderer.h"
#include "se/audio/Buffer.h"
#include "se/app/graphics/RawMesh.h"
#include "se/app/io/SceneSerializer.h"
#include "se/app/io/ShaderLoader.h"
#include "se/app/io/ImageReader.h"
#include "se/app/TagComponent.h"
#include "se/app/ScriptComponent.h"
#include "se/app/MeshComponent.h"
#include "se/app/TerrainComponent.h"
#include "se/app/ParticleSystemComponent.h"
#include "se/app/TransformsComponent.h"
#include "se/app/CameraComponent.h"
#include "se/app/SkinComponent.h"
#include "se/app/AnimationComponent.h"
#include "se/app/LightComponent.h"
#include "se/app/LightProbeComponent.h"
#include "se/app/RigidBodyComponent.h"
#include "se/app/AudioSourceComponent.h"
#include "se/app/ScriptComponent.h"

using namespace se::utils;
using namespace se::graphics;
using namespace se::physics;
using namespace se::animation;
using namespace se::audio;

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
		std::unordered_map<std::size_t, std::size_t> linkedScenesMap;
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
	void serializeResource(const Repository::ResourceRef<T>& resource, SerializeData& data, nlohmann::json& json, std::ostream& dataStream);

	template <typename T>
	Result deserializeResource(const nlohmann::json& json, Repository::ResourceRef<T>& resource, DeserializeData& data, Scene& scene);


	template <>
	void serializeResource<Mesh>(const Repository::ResourceRef<Mesh>& mesh, SerializeData& data, nlohmann::json& json, std::ostream& dataStream)
	{
		// VBOs
		const auto& vao = mesh->getVAO();

		nlohmann::json jsonVBOs;
		const auto& vbos = mesh->getVBOs();
		for (unsigned int i = 0; i < MeshAttributes::NumAttributes; ++i) {
			if (vao.isAttributeEnabled(i)) {
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
					mesh->getVAO().getVertexAttribute(
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
		const auto& ibo = mesh->getIBO();

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
		auto [minimum, maximum] = mesh->getBounds();
		jsonBounds["minimum"] = toJson(minimum);
		jsonBounds["maximum"] = toJson(maximum);
		json["bounds"] = jsonBounds;
	}

	template <>
	Result deserializeResource<Mesh>(const nlohmann::json& json, Repository::ResourceRef<Mesh>& mesh, DeserializeData& data, Scene& scene)
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
			vao.enableAttribute(attribute);
			if (attribute == MeshAttributes::JointIndexAttribute) {
				vao.setVertexIntegerAttribute(attribute, accessor.type, accessor.componentSize, accessor.stride, accessor.offset);
			}
			else {
				vao.setVertexAttribute(attribute, accessor.type, accessor.normalized, accessor.componentSize, accessor.stride, accessor.offset);
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
		if ((itMinimum == itBounds->end()) || !toVec(*itMinimum, minimum)
			|| (itMaximum == itBounds->end()) || !toVec(*itMaximum, maximum)
		) {
			return Result(false, "Wrong \"bounds\" property");
		}

		// Create the mesh
		auto meshSPtr = std::make_shared<Mesh>(std::move(vbos), std::move(ibo), std::move(vao));
		meshSPtr->setBounds(minimum, maximum);

		mesh = scene.repository.insert(std::move(meshSPtr));

		return Result();
	}


	template <>
	void serializeResource<Skin>(const Repository::ResourceRef<Skin>& skin, SerializeData& data, nlohmann::json& json, std::ostream& dataStream)
	{
		nlohmann::json bufferJson;
		serializeBuffer(
			reinterpret_cast<const std::byte*>(skin->inverseBindMatrices.data()),
			skin->inverseBindMatrices.size() * sizeof(glm::mat4), bufferJson, dataStream
		);
		data.buffersJson.emplace_back(std::move(bufferJson));

		json["inverseBindMatrices"] = data.buffersJson.size() - 1;
	}

	template <>
	Result deserializeResource<Skin>(const nlohmann::json& json, Repository::ResourceRef<Skin>& skin, DeserializeData& data, Scene& scene)
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

		auto skinSPtr = std::make_shared<Skin>();

		glm::mat4* bufPtr = reinterpret_cast<glm::mat4*>(buffer.data());
		std::copy(bufPtr, bufPtr + buffer.size() / sizeof(glm::mat4), skinSPtr->inverseBindMatrices.end());

		skin = scene.repository.insert(std::move(skinSPtr));

		return Result();
	}


	template <>
	void serializeResource<SkeletonAnimator>(const Repository::ResourceRef<SkeletonAnimator>& animator, SerializeData& data, nlohmann::json& json, std::ostream& dataStream)
	{
		nlohmann::json nodeAnimatorsVJson;
		animator->processAnimators([&](const char* nodeName, TransformationAnimator::TransformationType tType, TransformationAnimator* animator) {
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

		json["loopTime"] = animator->getLoopTime();
		json["nodeAnimators"] = std::move(nodeAnimatorsVJson);
	}

	template <>
	Result deserializeResource<SkeletonAnimator>(const nlohmann::json& json, Repository::ResourceRef<SkeletonAnimator>& animator, DeserializeData& data, Scene& scene)
	{
		auto animatorSPtr = std::make_shared<SkeletonAnimator>();

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
				animatorSPtr->addAnimator(nodeName.c_str(), transformationType, std::move(nodeAnimator));
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
				animatorSPtr->addAnimator(nodeName.c_str(), transformationType, std::move(nodeAnimator));
			}
			else {
				return Result(false, "Failed to parse nodeAnimators[" + std::to_string(i) + "]: wrong Animation type=" + animationType);
			}
		}

		auto itLoopTime = json.find("loopTime");
		if (itLoopTime == json.end()) {
			return Result(false, "Missing \"loopTime\" property");
		}

		animatorSPtr->setLoopTime(*itLoopTime);

		animator = scene.repository.insert(std::move(animatorSPtr));

		return Result();
	}


	template <>
	void serializeResource<LightSource>(const Repository::ResourceRef<LightSource>& light, SerializeData&, nlohmann::json& json, std::ostream&)
	{
		json["type"] = static_cast<int>(light->getType());
		json["color"] = toJson(light->getColor());
		json["intensity"] = light->getIntensity();
		json["range"] = light->getRange();

		if (light->getType() == LightSource::Type::Spot) {
			float innerConeAngle, outerConeAngle;
			light->getSpotLightRange(innerConeAngle, outerConeAngle);
			json["innerConeAngle"] = innerConeAngle;
			json["outerConeAngle"] = outerConeAngle;
		}

		json["castsShadows"] = light->castsShadows();
		if (light->castsShadows()) {
			float size, zNear, zFar;
			std::size_t resolution, numCascades;
			light->getShadows(resolution, zNear, zFar, size, numCascades);

			json["shadowResolution"] = resolution;
			json["shadowZNear"] = zNear;
			json["shadowZFar"] = zFar;
			json["shadowSize"] = size;
			json["shadowNumCascades"] = numCascades;
		}
	}

	template <>
	Result deserializeResource<LightSource>(const nlohmann::json& json, Repository::ResourceRef<LightSource>& light, DeserializeData&, Scene& scene)
	{
		auto itType = json.find("type");
		if (itType == json.end()) {
			return Result(false, "Missing \"type\" property");
		}

		auto lightSPtr = std::make_shared<LightSource>(scene.application.getEventManager(), static_cast<LightSource::Type>(itType->get<int>()));

		auto itColor = json.find("color");
		if (itColor != json.end()) {
			glm::vec3 color;
			if (toVec(*itColor, color)) {
				lightSPtr->setColor(color);
			}
		}

		auto itIntensity = json.find("intensity");
		if (itIntensity != json.end()) {
			lightSPtr->setIntensity(itIntensity->get<float>());
		}

		auto itRange = json.find("range");
		if (itRange != json.end()) {
			lightSPtr->setRange(itRange->get<float>());
		}

		if (lightSPtr->getType() == LightSource::Type::Spot) {
			auto itInnerConeAngle = json.find("innerConeAngle");
			auto itOuterConeAngle = json.find("outerConeAngle");
			if ((itInnerConeAngle != json.end()) && (itOuterConeAngle != json.end())) {
				lightSPtr->setSpotLightRange(itInnerConeAngle->get<float>(), itOuterConeAngle->get<float>());
			}
		}

		auto itCastsShadows = json.find("castsShadows");
		if ((itCastsShadows != json.end()) && itCastsShadows->get<bool>()) {
			float size = 10.0f, zNear = -1.0f, zFar = 1.0f;
			std::size_t resolution = 1024, numCascades = 1;

			auto itResolution = json.find("shadowResolution");
			if (itResolution != json.end()) {
				resolution = *itResolution;
			}

			auto itZNear = json.find("shadowZNear");
			if (itZNear != json.end()) {
				zNear = *itZNear;
			}

			auto itZFar = json.find("shadowZFar");
			if (itZFar != json.end()) {
				zFar = *itZFar;
			}

			auto itSize = json.find("shadowSize");
			if (itSize != json.end()) {
				size = *itSize;
			}

			auto itNumCascades = json.find("shadowNumCascades");
			if (itNumCascades != json.end()) {
				numCascades = *itNumCascades;
			}

			lightSPtr->setShadows(resolution, zNear, zFar, size, numCascades);
		}

		light = scene.repository.insert(std::move(lightSPtr));

		return Result();
	}


	template <>
	void serializeResource<Texture>(const Repository::ResourceRef<Texture>& texture, SerializeData& data, nlohmann::json& json, std::ostream& dataStream)
	{
		TextureTarget target = texture->getTarget();
		json["target"] = static_cast<int>(target);

		TypeId type = texture->getTypeId();
		json["type"] = static_cast<int>(type);

		ColorFormat color = texture->getColorFormat();
		json["color"] = static_cast<int>(color);

		TextureFilter min, mag;
		texture->getFiltering(&min, &mag);

		json["min"] = static_cast<int>(min);
		json["mag"] = static_cast<int>(mag);

		TextureWrap wrapS, wrapT, wrapR;
		texture->getWrapping(&wrapS, &wrapT, &wrapR);

		json["wrapS"] = static_cast<int>(wrapS);
		json["wrapT"] = static_cast<int>(wrapT);
		json["wrapR"] = static_cast<int>(wrapR);

		json["textureUnit"] = texture->getTextureUnit();

		std::string path = texture.getResource().getPath();
		if (!path.empty()) {
			// Use the path to the image file
			json["path"] = path;
		}
		else {
			// Save the texture to a buffer in the dataStream
			std::size_t width = texture->getWidth();
			std::size_t height = (target != TextureTarget::Texture1D)? texture->getHeight() : 1;
			std::size_t depth = ((target != TextureTarget::Texture1D) && (target != TextureTarget::Texture2D))? texture->getDepth() : 1;

			std::vector<std::byte> bufferData;
			if (target == TextureTarget::CubeMap) {
				std::size_t sideSize = width * height * depth * toNumberOfComponents(color) * toTypeSize(type);
				bufferData.resize(6 * sideSize);
				for (int i = 0; i < 6; ++i) {
					texture->getImage(type, toUnSizedColorFormat(color), &bufferData[i * sideSize], i);
				}
			}
			else {
				bufferData.resize(width * height * depth * toNumberOfComponents(color) * toTypeSize(type));
				texture->getImage(type, toUnSizedColorFormat(color), bufferData.data());
			}

			nlohmann::json bufferJson;
			serializeBuffer(bufferData.data(), bufferData.size(), bufferJson, dataStream);
			data.buffersJson.emplace_back(std::move(bufferJson));

			json["width"] = width;
			json["height"] = height;
			json["depth"] = depth;
			json["buffer"] = data.buffersJson.size() - 1;
		}
	}

	template <>
	Result deserializeResource<Texture>(const nlohmann::json& json, Repository::ResourceRef<Texture>& texture, DeserializeData& data, Scene& scene)
	{
		auto itTarget = json.find("target");
		if (itTarget == json.end()) {
			return Result(false, "Missing \"target\" property");
		}
		TextureTarget target = static_cast<TextureTarget>(itTarget->get<int>());

		auto itType = json.find("type");
		if (itType == json.end()) {
			return Result(false, "Missing \"type\" property");
		}
		TypeId type = static_cast<TypeId>(itType->get<int>());

		auto itColor = json.find("color");
		if (itColor == json.end()) {
			return Result(false, "Missing \"color\" property");
		}
		ColorFormat color = static_cast<ColorFormat>(itColor->get<int>());

		auto itTextureUnit = json.find("textureUnit");
		if (itTextureUnit == json.end()) {
			return Result(false, "Missing \"textureUnit\" property");
		}
		int textureUnit = *itTextureUnit;

		auto itMin = json.find("min");
		auto itMag = json.find("mag");
		TextureFilter min = (itMin != json.end())? static_cast<TextureFilter>(itMin->get<int>()) : TextureFilter::Nearest;
		TextureFilter mag = (itMag != json.end())? static_cast<TextureFilter>(itMag->get<int>()) : TextureFilter::Nearest;

		auto itWrapS = json.find("wrapS");
		auto itWrapT = json.find("wrapT");
		auto itWrapR = json.find("wrapR");
		TextureWrap wrapS = (itWrapS != json.end())? static_cast<TextureWrap>(itWrapS->get<int>()) : TextureWrap::ClampToBorder;
		TextureWrap wrapT = (itWrapT != json.end())? static_cast<TextureWrap>(itWrapT->get<int>()) : TextureWrap::ClampToBorder;
		TextureWrap wrapR = (itWrapR != json.end())? static_cast<TextureWrap>(itWrapR->get<int>()) : TextureWrap::ClampToBorder;

		auto textureSPtr = std::make_shared<Texture>(target);

		auto itPath = json.find("path");
		auto itBuffer = json.find("buffer");
		if (itPath != json.end()) {
			// Load from file, ONLY TEXTURE2D
			std::string path = *itPath;

			if ((path.size() > 3) && (path.substr(path.size() - 3, 3) == "hdr")) {
				Image<float> image;
				auto result = ImageReader::readHDR(path.c_str(), image, static_cast<int>(toNumberOfComponents(color)));
				if (!result) {
					return Result(false, "Failed to read the HDR Image: " + std::string(result.description()));
				}

				textureSPtr->setImage(image.pixels.get(), TypeId::Float, color, color, image.width, image.height);
			}
			else {
				Image<unsigned char> image;
				auto result = ImageReader::read(path.c_str(), image, static_cast<int>(toNumberOfComponents(color)));
				if (!result) {
					return Result(false, "Failed to read the Image: " + std::string(result.description()));
				}

				textureSPtr->setImage(image.pixels.get(), TypeId::UnsignedByte, color, color, image.width, image.height);
			}

			texture = scene.repository.insert(textureSPtr);
			texture.getResource().setPath(path.c_str());
		}
		else if (itBuffer != json.end()) {
			auto itWidth = json.find("width");
			if (itWidth == json.end()) {
				return Result(false, "Missing \"width\" property");
			}
			auto itHeight = json.find("height");
			if (itHeight == json.end()) {
				return Result(false, "Missing \"height\" property");
			}
			auto itDepth = json.find("depth");
			if (itDepth == json.end()) {
				return Result(false, "Missing \"depth\" property");
			}

			std::size_t width = *itWidth, height = *itHeight, depth = *itDepth;
			std::size_t iBuffer = *itBuffer;

			if (*itBuffer >= data.buffersJson.size()) {
				return Result(false, "Buffer " + std::to_string(iBuffer) + " out of bounds");
			}

			std::vector<std::byte> buffer;
			auto result = deserializeBuffer(data.buffersJson[iBuffer], data.dataStream, buffer);
			if (!result) {
				return Result(false, "Failed to parse buffer " + std::to_string(iBuffer) + ": " + result.description());
			}

			if (target == TextureTarget::CubeMap) {
				std::size_t sideSize = width * height * toNumberOfComponents(color) * toTypeSize(type);
				for (int i = 0; i < 6; ++i) {
					textureSPtr->setImage(buffer.data() + i * sideSize, type, toUnSizedColorFormat(color), color, width, height, 0, i);
				}
			}
			else {
				textureSPtr->setImage(buffer.data(), type, toUnSizedColorFormat(color), color, width, height, depth);
			}

			texture = scene.repository.insert(textureSPtr);
		}
		else {
			return Result(false, "Missing \"path\" and \"buffer\" properties");
		}

		textureSPtr->setTextureUnit(textureUnit)
			.setFiltering(min, mag)
			.setWrapping(wrapS, wrapT, wrapR)
			.generateMipMap();

		return Result();
	}


	template <>
	void serializeResource<Program>(const Repository::ResourceRef<Program>& program, SerializeData&, nlohmann::json& json, std::ostream&)
	{
		if (auto path = program.getResource().getPath()) {
			json["path"] = path;
		}
	}

	template <>
	Result deserializeResource<Program>(const nlohmann::json& json, Repository::ResourceRef<Program>& program, DeserializeData&, Scene& scene)
	{
		auto itPath = json.find("path");
		if (itPath == json.end()) {
			return Result(false, "Missing \"path\" property");
		}

		std::string vertexPath, geometryPath, fragmentPath;
		std::stringstream ss(itPath->get<std::string>());
		std::getline(ss, vertexPath, '|');
		std::getline(ss, geometryPath, '|');
		std::getline(ss, fragmentPath, '|');

		std::shared_ptr<Program> programSPtr;
		auto result = ShaderLoader::createProgram(
			vertexPath.empty()? nullptr : vertexPath.c_str(),
			geometryPath.empty()? nullptr : geometryPath.c_str(),
			fragmentPath.empty()? nullptr : fragmentPath.c_str(),
			programSPtr
		);
		if (!result) {
			return Result(false, "Couldn't create the program: " + std::string(result.description()));
		}

		program = scene.repository.insert(std::move(programSPtr));
		program.getResource().setPath( itPath->get<std::string>().c_str() );

		return Result();
	}


	template <>
	void serializeResource<RenderableShaderStep>(const Repository::ResourceRef<RenderableShaderStep>& step, SerializeData& data, nlohmann::json& json, std::ostream&)
	{
		nlohmann::json bindablesVJson;
		step->processPrograms([&](const Repository::ResourceRef<Program>& program) {
			nlohmann::json bindableJson = { { "type", "Program" }, { "name", program.getResource().getName() } };
			bindablesVJson.emplace_back(std::move(bindableJson));
		});
		step->processTextures([&](const Repository::ResourceRef<Texture>& texture) {
			nlohmann::json bindableJson = { { "type", "Texture" }, { "name", texture.getResource().getName() } };
			bindablesVJson.emplace_back(std::move(bindableJson));
		});
		step->processBindables([&](std::shared_ptr<Bindable> bindable) {
			nlohmann::json bindableJson;
			if (auto uniform = std::dynamic_pointer_cast<IUniformVariable>(bindable)) {
				if (auto program = data.scene.repository.findResource(uniform->getProgram().get())) {
					if (auto uniform1 = std::dynamic_pointer_cast<UniformVariableValue<int>>(bindable)) {
						bindableJson = { { "type", "UniformVariableValue" }, { "UniformType", "int" }, { "value", uniform1->getValue() } };
					}
					else if (auto uniform2 = std::dynamic_pointer_cast<UniformVariableValue<unsigned int>>(bindable)) {
						bindableJson = { { "type", "UniformVariableValue" }, { "UniformType", "unsigned int" }, { "value", uniform2->getValue() } };
					}
					else if (auto uniform3 = std::dynamic_pointer_cast<UniformVariableValue<float>>(bindable)) {
						bindableJson = { { "type", "UniformVariableValue" }, { "UniformType", "float" }, { "value", uniform3->getValue() } };
					}
					else if (auto uniform4 = std::dynamic_pointer_cast<UniformVariableValue<glm::vec2>>(bindable)) {
						bindableJson = { { "type", "UniformVariableValue" }, { "UniformType", "vec2" }, { "value", toJson(uniform4->getValue()) } };
					}
					else if (auto uniform5 = std::dynamic_pointer_cast<UniformVariableValue<glm::ivec2>>(bindable)) {
						bindableJson = { { "type", "UniformVariableValue" }, { "UniformType", "ivec2" }, { "value", toJson(uniform5->getValue()) } };
					}
					else if (auto uniform6 = std::dynamic_pointer_cast<UniformVariableValue<glm::vec3>>(bindable)) {
						bindableJson = { { "type", "UniformVariableValue" }, { "UniformType", "vec3" }, { "value", toJson(uniform6->getValue()) } };
					}
					else if (auto uniform7 = std::dynamic_pointer_cast<UniformVariableValue<glm::ivec3>>(bindable)) {
						bindableJson = { { "type", "UniformVariableValue" }, { "UniformType", "ivec3" }, { "value", toJson(uniform7->getValue()) } };
					}
					else if (auto uniform8 = std::dynamic_pointer_cast<UniformVariableValue<glm::vec4>>(bindable)) {
						bindableJson = { { "type", "UniformVariableValue" }, { "UniformType", "vec4" }, { "value", toJson(uniform8->getValue()) } };
					}
					else if (auto uniform9 = std::dynamic_pointer_cast<UniformVariableValue<glm::ivec4>>(bindable)) {
						bindableJson = { { "type", "UniformVariableValue" }, { "UniformType", "ivec4" }, { "value", toJson(uniform9->getValue()) } };
					}
					else if (auto uniform10 = std::dynamic_pointer_cast<UniformVariableValue<glm::mat3>>(bindable)) {
						bindableJson = { { "type", "UniformVariableValue" }, { "UniformType", "mat3" }, { "value", toJson(uniform10->getValue()) } };
					}
					else if (auto uniform11 = std::dynamic_pointer_cast<UniformVariableValue<glm::mat4>>(bindable)) {
						bindableJson = { { "type", "UniformVariableValue" }, { "UniformType", "mat4" }, { "value", toJson(uniform11->getValue()) } };
					}
					else if (auto uniform12 = std::dynamic_pointer_cast<UniformVariableValue<glm::mat3x4>>(bindable)) {
						bindableJson = { { "type", "UniformVariableValue" }, { "UniformType", "mat3x4" }, { "value", toJson(uniform12->getValue()) } };
					}
					else if (auto uniform13 = std::dynamic_pointer_cast<UniformVariableValueVector<int>>(bindable)) {
						const int* ptr; std::size_t size; uniform13->getValue(ptr, size);
						auto dataVJson = nlohmann::json::array();
						for (std::size_t i = 0; i < size; ++i) { dataVJson.push_back(ptr[i]); }
						bindableJson = { { "type", "UniformVariableValueVector" }, { "UniformType", "int" }, { "value", std::move(dataVJson) } };
					}
					else if (auto uniform14 = std::dynamic_pointer_cast<UniformVariableValueVector<unsigned int>>(bindable)) {
						const unsigned int* ptr; std::size_t size; uniform14->getValue(ptr, size);
						auto dataVJson = nlohmann::json::array();
						for (std::size_t i = 0; i < size; ++i) { dataVJson.push_back(ptr[i]); }
						bindableJson = { { "type", "UniformVariableValueVector" }, { "UniformType", "int" }, { "value", std::move(dataVJson) } };
					}
					else if (auto uniform15 = std::dynamic_pointer_cast<UniformVariableValueVector<float>>(bindable)) {
						const float* ptr; std::size_t size; uniform15->getValue(ptr, size);
						auto dataVJson = nlohmann::json::array();
						for (std::size_t i = 0; i < size; ++i) { dataVJson.push_back(ptr[i]); }
						bindableJson = { { "type", "UniformVariableValueVector" }, { "UniformType", "float" }, { "value", std::move(dataVJson) } };
					}
					else if (auto uniform16 = std::dynamic_pointer_cast<UniformVariableValueVector<glm::vec2>>(bindable)) {
						const glm::vec2* ptr; std::size_t size; uniform16->getValue(ptr, size);
						auto dataVJson = nlohmann::json::array();
						for (std::size_t i = 0; i < size; ++i) { dataVJson.push_back(toJson(ptr[i])); }
						bindableJson = { { "type", "UniformVariableValueVector" }, { "UniformType", "vec2" }, { "value", std::move(dataVJson) } };
					}
					else if (auto uniform17 = std::dynamic_pointer_cast<UniformVariableValueVector<glm::ivec2>>(bindable)) {
						const glm::ivec2* ptr; std::size_t size; uniform17->getValue(ptr, size);
						auto dataVJson = nlohmann::json::array();
						for (std::size_t i = 0; i < size; ++i) { dataVJson.push_back(toJson(ptr[i])); }
						bindableJson = { { "type", "UniformVariableValueVector" }, { "UniformType", "ivec2" }, { "value", std::move(dataVJson) } };
					}
					else if (auto uniform18 = std::dynamic_pointer_cast<UniformVariableValueVector<glm::vec3>>(bindable)) {
						const glm::vec3* ptr; std::size_t size; uniform18->getValue(ptr, size);
						auto dataVJson = nlohmann::json::array();
						for (std::size_t i = 0; i < size; ++i) { dataVJson.push_back(toJson(ptr[i])); }
						bindableJson = { { "type", "UniformVariableValueVector" }, { "UniformType", "vec3" }, { "value", std::move(dataVJson) } };
					}
					else if (auto uniform19 = std::dynamic_pointer_cast<UniformVariableValueVector<glm::ivec3>>(bindable)) {
						const glm::ivec3* ptr; std::size_t size; uniform19->getValue(ptr, size);
						auto dataVJson = nlohmann::json::array();
						for (std::size_t i = 0; i < size; ++i) { dataVJson.push_back(toJson(ptr[i])); }
						bindableJson = { { "type", "UniformVariableValueVector" }, { "UniformType", "ivec3" }, { "value", std::move(dataVJson) } };
					}
					else if (auto uniform20 = std::dynamic_pointer_cast<UniformVariableValueVector<glm::vec4>>(bindable)) {
						const glm::vec4* ptr; std::size_t size; uniform20->getValue(ptr, size);
						auto dataVJson = nlohmann::json::array();
						for (std::size_t i = 0; i < size; ++i) { dataVJson.push_back(toJson(ptr[i])); }
						bindableJson = { { "type", "UniformVariableValueVector" }, { "UniformType", "vec4" }, { "value", std::move(dataVJson) } };
					}
					else if (auto uniform21 = std::dynamic_pointer_cast<UniformVariableValueVector<glm::ivec4>>(bindable)) {
						const glm::ivec4* ptr; std::size_t size; uniform21->getValue(ptr, size);
						auto dataVJson = nlohmann::json::array();
						for (std::size_t i = 0; i < size; ++i) { dataVJson.push_back(toJson(ptr[i])); }
						bindableJson = { { "type", "UniformVariableValueVector" }, { "UniformType", "ivec4" }, { "value", std::move(dataVJson) } };
					}
					else if (auto uniform22 = std::dynamic_pointer_cast<UniformVariableValueVector<glm::mat3>>(bindable)) {
						const glm::mat3* ptr; std::size_t size; uniform22->getValue(ptr, size);
						auto dataVJson = nlohmann::json::array();
						for (std::size_t i = 0; i < size; ++i) { dataVJson.push_back(toJson(ptr[i])); }
						bindableJson = { { "type", "UniformVariableValueVector" }, { "UniformType", "mat3" }, { "value", std::move(dataVJson) } };
					}
					else if (auto uniform23 = std::dynamic_pointer_cast<UniformVariableValueVector<glm::mat4>>(bindable)) {
						const glm::mat4* ptr; std::size_t size; uniform23->getValue(ptr, size);
						auto dataVJson = nlohmann::json::array();
						for (std::size_t i = 0; i < size; ++i) { dataVJson.push_back(toJson(ptr[i])); }
						bindableJson = { { "type", "UniformVariableValueVector" }, { "UniformType", "mat4" }, { "value", std::move(dataVJson) } };
					}
					else if (auto uniform24 = std::dynamic_pointer_cast<UniformVariableValueVector<glm::mat3x4>>(bindable)) {
						const glm::mat3x4* ptr; std::size_t size; uniform24->getValue(ptr, size);
						auto dataVJson = nlohmann::json::array();
						for (std::size_t i = 0; i < size; ++i) { dataVJson.push_back(toJson(ptr[i])); }
						bindableJson = { { "type", "UniformVariableValueVector" }, { "UniformType", "mat3x4" }, { "value", std::move(dataVJson) } };
					}

					bindableJson["name"] = uniform->getName();
					bindableJson["program"] = program.getResource().getName();
				}
			}
			else if (auto setOperation = std::dynamic_pointer_cast<SetOperation>(bindable)) {
				bindableJson = { { "type", "SetOperation" }, { "operation", static_cast<int>(setOperation->getOperation()) }, { "active", setOperation->getEnable() } };
			}

			if (!bindableJson.empty()) {
				bindablesVJson.emplace_back(std::move(bindableJson));
			}
		});
		json["renderer"] = step->getPass()->getRenderer().getName();
		json["bindables"] = std::move(bindablesVJson);
	}

	template <>
	Result deserializeResource<RenderableShaderStep>(const nlohmann::json& json, Repository::ResourceRef<RenderableShaderStep>& step, DeserializeData&, Scene& scene)
	{
		auto itRenderer = json.find("renderer");
		if (itRenderer == json.end()) {
			return Result(false, "Missing \"renderer\" property");
		}

		auto renderNode = scene.application.getExternalTools().graphicsEngine->getRenderGraph().getNode(*itRenderer);
		auto renderer = dynamic_cast<Renderer*>(renderNode);
		if (!renderer) {
			return Result(false, "\"renderer\"=" + itRenderer->get<std::string>() + " not found");
		}

		auto itBindables = json.find("bindables");
		if (itBindables == json.end()) {
			return Result(false, "Missing \"bindables\" property");
		}

		auto stepSPtr = std::make_shared<RenderableShaderStep>(*renderer);
		for (std::size_t i = 0; i < itBindables->size(); ++i) {
			const auto& jBindable = (*itBindables)[i];
			std::shared_ptr<Bindable> bindable;

			auto itType = jBindable.find("type");
			if (itType == jBindable.end()) {
				return Result(false, "Failed to parse bindables[" + std::to_string(i) + "]: Missing \"type\" property");
			}

			if (*itType == "UniformVariableValue") {
				auto itUniformType = jBindable.find("UniformType");
				if (itUniformType == jBindable.end()) {
					return Result(false, "Failed to parse bindables[" + std::to_string(i) + "]: UniformVariableValue missing \"UniformType\" property");
				}
				auto itName = jBindable.find("name");
				if (itName == jBindable.end()) {
					return Result(false, "Failed to parse bindables[" + std::to_string(i) + "]: UniformVariableValue missing \"name\" property");
				}
				auto itProgram = jBindable.find("program");
				if (itProgram == jBindable.end()) {
					return Result(false, "Failed to parse bindables[" + std::to_string(i) + "]: UniformVariableValue missing \"program\" property");
				}
				auto itValue = jBindable.find("value");
				if (itValue == jBindable.end()) {
					return Result(false, "Failed to parse bindables[" + std::to_string(i) + "]: UniformVariableValue missing \"value\" property");
				}

				std::string name = *itName;

				auto program = scene.repository.findByName<Program>(itProgram->get<std::string>().c_str());
				if (!program) {
					return Result(false, "Failed to parse bindables[" + std::to_string(i) + "]: \"program\"= " + itProgram->get<std::string>() + " not found");
				}

				if (*itUniformType == "int") {
					bindable = std::make_shared<UniformVariableValue<int>>(name.c_str(), program.get(), *itValue);
				}
				else if (*itUniformType == "unsigned int") {
					bindable = std::make_shared<UniformVariableValue<unsigned int>>(name.c_str(), program.get(), *itValue);
				}
				else if (*itUniformType == "float") {
					bindable = std::make_shared<UniformVariableValue<float>>(name.c_str(), program.get(), *itValue);
				}
				else if (*itUniformType == "vec2") {
					glm::vec2 value;
					if (toVec(*itValue, value)) {
						bindable = std::make_shared<UniformVariableValue<glm::vec2>>(name.c_str(), program.get(), value);
					}
				}
				else if (*itUniformType == "ivec2") {
					glm::ivec2 value;
					if (toVec(*itValue, value)) {
						bindable = std::make_shared<UniformVariableValue<glm::ivec2>>(name.c_str(), program.get(), value);
					}
				}
				else if (*itUniformType == "vec3") {
					glm::vec3 value;
					if (toVec(*itValue, value)) {
						bindable = std::make_shared<UniformVariableValue<glm::vec3>>(name.c_str(), program.get(), value);
					}
				}
				else if (*itUniformType == "ivec3") {
					glm::ivec3 value;
					if (toVec(*itValue, value)) {
						bindable = std::make_shared<UniformVariableValue<glm::ivec3>>(name.c_str(), program.get(), value);
					}
				}
				else if (*itUniformType == "vec4") {
					glm::vec4 value;
					if (toVec(*itValue, value)) {
						bindable = std::make_shared<UniformVariableValue<glm::vec4>>(name.c_str(), program.get(), value);
					}
				}
				else if (*itUniformType == "ivec4") {
					glm::ivec4 value;
					if (toVec(*itValue, value)) {
						bindable = std::make_shared<UniformVariableValue<glm::ivec4>>(name.c_str(), program.get(), value);
					}
				}
				else if (*itUniformType == "mat3") {
					glm::mat3 value;
					if (toMat(*itValue, value)) {
						bindable = std::make_shared<UniformVariableValue<glm::mat3>>(name.c_str(), program.get(), value);
					}
				}
				else if (*itUniformType == "mat4") {
					glm::mat4 value;
					if (toMat(*itValue, value)) {
						bindable = std::make_shared<UniformVariableValue<glm::mat4>>(name.c_str(), program.get(), value);
					}
				}
				else if (*itUniformType == "mat3x4") {
					glm::mat3x4 value;
					if (toMat(*itValue, value)) {
						bindable = std::make_shared<UniformVariableValue<glm::mat3x4>>(name.c_str(), program.get(), value);
					}
				}
			}
			else if (*itType == "UniformVariableValueVector") {
				auto itUniformType = jBindable.find("UniformType");
				if (itUniformType == jBindable.end()) {
					return Result(false, "Failed to parse bindables[" + std::to_string(i) + "]: UniformVariableValue missing \"UniformType\" property");
				}
				auto itName = jBindable.find("name");
				if (itName == jBindable.end()) {
					return Result(false, "Failed to parse bindables[" + std::to_string(i) + "]: UniformVariableValue missing \"name\" property");
				}
				auto itProgram = jBindable.find("program");
				if (itProgram == jBindable.end()) {
					return Result(false, "Failed to parse bindables[" + std::to_string(i) + "]: UniformVariableValue missing \"program\" property");
				}
				auto itValue = jBindable.find("value");
				if (itValue == jBindable.end()) {
					return Result(false, "Failed to parse bindables[" + std::to_string(i) + "]: UniformVariableValue missing \"value\" property");
				}

				std::string name = *itName;

				auto program = scene.repository.findByName<Program>(itProgram->get<std::string>().c_str());
				if (!program) {
					return Result(false, "Failed to parse bindables[" + std::to_string(i) + "]: \"program\"= " + itProgram->get<std::string>() + " not found");
				}

				if (*itUniformType == "int") {
					std::vector<int> value = *itValue;
					bindable = std::make_shared<UniformVariableValueVector<int>>(name.c_str(), program.get(), value.data(), value.size());
				}
				else if (*itUniformType == "unsigned int") {
					std::vector<unsigned int> value = *itValue;
					bindable = std::make_shared<UniformVariableValueVector<unsigned int>>(name.c_str(), program.get(), value.data(), value.size());
				}
				else if (*itUniformType == "float") {
					std::vector<float> value = *itValue;
					bindable = std::make_shared<UniformVariableValueVector<float>>(name.c_str(), program.get(), value.data(), value.size());
				}
				else if (*itUniformType == "vec2") {
					std::vector<glm::vec2> value;
					for (const auto& jv : *itValue) {
						if (glm::vec2 v; toVec(jv, v)) {
							value.push_back(v);
						}
					}
					bindable = std::make_shared<UniformVariableValueVector<glm::vec2>>(name.c_str(), program.get(), value.data(), value.size());
				}
				else if (*itUniformType == "ivec2") {
					std::vector<glm::ivec2> value;
					for (const auto& jv : *itValue) {
						if (glm::ivec2 v; toVec(jv, v)) {
							value.push_back(v);
						}
					}
					bindable = std::make_shared<UniformVariableValueVector<glm::ivec2>>(name.c_str(), program.get(), value.data(), value.size());
				}
				else if (*itUniformType == "vec3") {
					std::vector<glm::vec3> value;
					for (const auto& jv : *itValue) {
						if (glm::vec3 v; toVec(jv, v)) {
							value.push_back(v);
						}
					}
					bindable = std::make_shared<UniformVariableValueVector<glm::vec3>>(name.c_str(), program.get(), value.data(), value.size());
				}
				else if (*itUniformType == "ivec3") {
					std::vector<glm::ivec3> value;
					for (const auto& jv : *itValue) {
						if (glm::ivec3 v; toVec(jv, v)) {
							value.push_back(v);
						}
					}
					bindable = std::make_shared<UniformVariableValueVector<glm::ivec3>>(name.c_str(), program.get(), value.data(), value.size());
				}
				else if (*itUniformType == "vec4") {
					std::vector<glm::vec4> value;
					for (const auto& jv : *itValue) {
						if (glm::vec4 v; toVec(jv, v)) {
							value.push_back(v);
						}
					}
					bindable = std::make_shared<UniformVariableValueVector<glm::vec4>>(name.c_str(), program.get(), value.data(), value.size());
				}
				else if (*itUniformType == "ivec4") {
					std::vector<glm::ivec4> value;
					for (const auto& jv : *itValue) {
						if (glm::ivec4 v; toVec(jv, v)) {
							value.push_back(v);
						}
					}
					bindable = std::make_shared<UniformVariableValueVector<glm::ivec4>>(name.c_str(), program.get(), value.data(), value.size());
				}
				else if (*itUniformType == "mat3") {
					std::vector<glm::mat3> value;
					for (const auto& jv : *itValue) {
						if (glm::mat3 v; toMat(jv, v)) {
							value.push_back(v);
						}
					}
					bindable = std::make_shared<UniformVariableValueVector<glm::mat3>>(name.c_str(), program.get(), value.data(), value.size());
				}
				else if (*itUniformType == "mat4") {
					std::vector<glm::mat4> value;
					for (const auto& jv : *itValue) {
						if (glm::mat4 v; toMat(jv, v)) {
							value.push_back(v);
						}
					}
					bindable = std::make_shared<UniformVariableValueVector<glm::mat4>>(name.c_str(), program.get(), value.data(), value.size());
				}
				else if (*itUniformType == "mat3x4") {
					std::vector<glm::mat3x4> value;
					for (const auto& jv : *itValue) {
						if (glm::mat3x4 v; toMat(jv, v)) {
							value.push_back(v);
						}
					}
					bindable = std::make_shared<UniformVariableValueVector<glm::mat3x4>>(name.c_str(), program.get(), value.data(), value.size());
				}
			}
			else if (*itType == "Texture") {
				auto itName = jBindable.find("name");
				if (itName == jBindable.end()) {
					return Result(false, "Failed to parse bindables[" + std::to_string(i) + "]: Texture missing \"name\" property");
				}
				auto resource = scene.repository.findByName<Texture>(itName->get<std::string>().c_str());
				if (!resource) {
					return Result(false, "Failed to parse bindables[" + std::to_string(i) + "]: Texture with \"name\"=" + itName->get<std::string>() + " not found");
				}
				bindable = resource.get();
				stepSPtr->addResource(std::move(resource), false);
			}
			else if (*itType == "Program") {
				auto itName = jBindable.find("name");
				if (itName == jBindable.end()) {
					return Result(false, "Failed to parse bindables[" + std::to_string(i) + "]: Program missing \"name\" property");
				}
				auto resource = scene.repository.findByName<Program>(itName->get<std::string>().c_str());
				if (!resource) {
					return Result(false, "Failed to parse bindables[" + std::to_string(i) + "]: Program with \"name\"=" + itName->get<std::string>() + " not found");
				}
				bindable = resource.get();
				stepSPtr->addResource(std::move(resource), false);
			}
			else if (*itType == "SetOperation") {
				auto itOperation = jBindable.find("operation");
				if (itOperation == jBindable.end()) {
					return Result(false, "Failed to parse bindables[" + std::to_string(i) + "]: SetOperation missing \"operation\" property");
				}
				auto itActive = jBindable.find("active");
				if (itActive == jBindable.end()) {
					return Result(false, "Failed to parse bindables[" + std::to_string(i) + "]: SetOperation missing \"active\" property");
				}
				bindable = std::make_shared<SetOperation>(static_cast<Operation>(itOperation->get<int>()), *itActive);
			}
			else {
				return Result(false, "Failed to parse bindables[" + std::to_string(i) + "]: Wrong \"type\" property = " + itType->get<std::string>());
			}

			stepSPtr->addBindable(std::move(bindable));
		}

		step = scene.repository.insert(std::move(stepSPtr));
		return Result();
	}


	template <>
	void serializeResource<RenderableShader>(const Repository::ResourceRef<RenderableShader>& shader, SerializeData&, nlohmann::json& json, std::ostream&)
	{
		auto stepsVJson = nlohmann::json::array();
		shader->processSteps([&](const auto& step) {
			stepsVJson.push_back(step.getResource().getName());
		});
		json["steps"] = std::move(stepsVJson);
	}

	template <>
	Result deserializeResource<RenderableShader>(const nlohmann::json& json, Repository::ResourceRef<RenderableShader>& shader, DeserializeData&, Scene& scene)
	{
		auto itSteps = json.find("steps");
		if (itSteps == json.end()) {
			return Result(false, "Missing \"steps\" property");
		}

		auto shaderSPtr = std::make_shared<RenderableShader>(scene.application.getEventManager());
		for (std::size_t i = 0; i < itSteps->size(); ++i) {
			std::string stepName = (*itSteps)[i].get<std::string>();
			auto step = scene.repository.findByName<RenderableShaderStep>(stepName.c_str());
			if (!step) {
				return Result(false, "Failed to parse steps[" + std::to_string(i) + "]: Name " + stepName + " not found");
			}

			shaderSPtr->addStep(step);
		}

		shader = scene.repository.insert(std::move(shaderSPtr));
		return Result();
	}


	template <>
	void serializeResource<Force>(const Repository::ResourceRef<Force>& force, SerializeData&, nlohmann::json& json, std::ostream&)
	{
		if (auto gravity = std::dynamic_pointer_cast<Gravity>(force.get()); gravity) {
			json["type"] = "Gravity";
			json["value"] = gravity->getValue();
		}
		else if (auto directional = std::dynamic_pointer_cast<DirectionalForce>(force.get()); directional) {
			json["type"] = "DirectionalForce";
			json["value"] = toJson(directional->getValue());
		}
		else if (auto punctual = std::dynamic_pointer_cast<PunctualForce>(force.get()); punctual) {
			json["type"] = "PunctualForce";
			json["value"] = toJson(punctual->getValue());
			json["point"] = toJson(punctual->getPoint());
		}
	}

	template <>
	Result deserializeResource<Force>(const nlohmann::json& json, Repository::ResourceRef<Force>& force, DeserializeData&, Scene& scene)
	{
		std::shared_ptr<Force> forceSPtr;

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

			forceSPtr = std::make_shared<Gravity>(itValue->get<float>());
		}
		else if (type == "DirectionalForce") {
			auto itValue = json.find("value");
			if (itValue == json.end()) {
				return Result(false, "DirectionalForce missing \"value\" property");
			}

			glm::vec3 value;
			if (!toVec(*itValue, value)) {
				return Result(false, "Wrong \"value\" property");
			}

			forceSPtr = std::make_shared<DirectionalForce>(value);
		}
		else if (type == "PunctualForce") {
			auto itValue = json.find("value");
			if (itValue == json.end()) {
				return Result(false, "PunctualForce missing \"value\" property");
			}

			glm::vec3 value;
			if (!toVec(*itValue, value)) {
				return Result(false, "Wrong \"value\" property");
			}

			auto itPoint = json.find("point");
			if (itPoint == json.end()) {
				return Result(false, "PunctualForce missing \"point\" property");
			}

			glm::vec3 point;
			if (!toVec(*itPoint, point)) {
				return Result(false, "Wrong \"point\" property");
			}

			forceSPtr = std::make_shared<PunctualForce>(value, point);
		}
		else {
			return Result(false, "Wrong \"type\" value = " + type);
		}

		force = scene.repository.insert(std::move(forceSPtr));
		return Result();
	}


	template <>
	void serializeResource<ParticleEmitter>(const Repository::ResourceRef<ParticleEmitter>& emitter, SerializeData&, nlohmann::json& json, std::ostream&)
	{
		json["maxParticles"] = emitter->maxParticles;
		json["duration"] = emitter->duration;
		json["loop"] = emitter->loop;
		json["initialVelocity"] = emitter->initialVelocity;
		json["initialPositionRandomFactor"] = emitter->initialPositionRandomFactor;
		json["initialVelocityRandomFactor"] = emitter->initialVelocityRandomFactor;
		json["initialRotationRandomFactor"] = emitter->initialRotationRandomFactor;
		json["scale"] = emitter->scale;
		json["initialScaleRandomFactor"] = emitter->initialScaleRandomFactor;
		json["lifeLength"] = emitter->lifeLength;
		json["lifeLengthRandomFactor"] = emitter->lifeLengthRandomFactor;
		json["gravity"] = emitter->gravity;
	}

	template <>
	Result deserializeResource<ParticleEmitter>(const nlohmann::json& json, Repository::ResourceRef<ParticleEmitter>& emitter, DeserializeData&, Scene& scene)
	{
		auto emitterSPtr = std::make_shared<ParticleEmitter>();

		auto itMaxParticles = json.find("maxParticles");
		if (itMaxParticles != json.end()) {
			emitterSPtr->maxParticles = *itMaxParticles;
		}
		auto itDuration = json.find("duration");
		if (itDuration != json.end()) {
			emitterSPtr->duration = *itDuration;
		}
		auto itLoop = json.find("loop");
		if (itLoop != json.end()) {
			emitterSPtr->loop = *itLoop;
		}
		auto itInitialVelocity = json.find("initialVelocity");
		if (itInitialVelocity != json.end()) {
			emitterSPtr->initialVelocity = *itInitialVelocity;
		}
		auto itInitialPositionRandomFactor = json.find("initialPositionRandomFactor");
		if (itInitialPositionRandomFactor != json.end()) {
			emitterSPtr->initialPositionRandomFactor = *itInitialPositionRandomFactor;
		}
		auto itInitialVelocityRandomFactor = json.find("initialVelocityRandomFactor");
		if (itInitialVelocityRandomFactor != json.end()) {
			emitterSPtr->initialVelocityRandomFactor = *itInitialVelocityRandomFactor;
		}
		auto itInitialRotationRandomFactor = json.find("initialRotationRandomFactor");
		if (itInitialRotationRandomFactor != json.end()) {
			emitterSPtr->initialRotationRandomFactor = *itInitialRotationRandomFactor;
		}
		auto itInitialScaleRandomFactor = json.find("initialScaleRandomFactor");
		if (itInitialScaleRandomFactor != json.end()) {
			emitterSPtr->initialScaleRandomFactor = *itInitialScaleRandomFactor;
		}
		auto itScale = json.find("scale");
		if (itScale != json.end()) {
			emitterSPtr->scale = *itScale;
		}
		auto itLifeLength = json.find("lifeLength");
		if (itLifeLength != json.end()) {
			emitterSPtr->lifeLength = *itLifeLength;
		}
		auto itLifeLengthRandomFactor = json.find("lifeLengthRandomFactor");
		if (itLifeLengthRandomFactor != json.end()) {
			emitterSPtr->lifeLengthRandomFactor = *itLifeLengthRandomFactor;
		}
		auto itGravity = json.find("gravity");
		if (itGravity != json.end()) {
			emitterSPtr->gravity = *itGravity;
		}

		emitter = scene.repository.insert(std::move(emitterSPtr));
		return Result();
	}


	template <>
	void serializeResource<Buffer>(const Repository::ResourceRef<Buffer>& buffer, SerializeData&, nlohmann::json& json, std::ostream&)
	{
		std::string path = buffer.getResource().getPath();
		if (!path.empty()) {
			json["path"] = path;
		}
	}

	template <>
	Result deserializeResource<Buffer>(const nlohmann::json& json, Repository::ResourceRef<Buffer>& buffer, DeserializeData&, Scene& scene)
	{
		auto itPath = json.find("path");
		if (itPath == json.end()) {
			return Result(false, "Missing \"path\"");
		}

		std::string path = *itPath;
		AudioFile<float> audioFile;
		if (!audioFile.load(path)) {
			return Result(false, "Failed to read the audio file");
		}

		buffer = scene.repository.emplace<Buffer>(
			audioFile.samples[0].data(), audioFile.samples[0].size() * sizeof(float),
			FormatId::MonoFloat, audioFile.getSampleRate()
		);
		buffer.getResource().setPath(path.c_str());

		return Result();
	}

// Components
	template <typename T>
	nlohmann::json serializeComponent(const T& component, SerializeData& data, std::ostream& dataStream);

	template <typename T>
	ResultOptional<T> deserializeComponent(const nlohmann::json& json, DeserializeData& data, Scene& scene);


	template <>
	nlohmann::json serializeComponent<TagComponent>(const TagComponent& tag, SerializeData&, std::ostream&)
	{
		nlohmann::json json;
		json["name"] = tag.getName();
		return json;
	}

	template <>
	ResultOptional<TagComponent> deserializeComponent<TagComponent>(const nlohmann::json& json, DeserializeData&, Scene&)
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
	ResultOptional<TransformsComponent> deserializeComponent<TransformsComponent>(const nlohmann::json& json, DeserializeData&, Scene&)
	{
		TransformsComponent transforms;

		auto itPosition = json.find("position");
		if (itPosition != json.end()) {
			toVec(*itPosition, transforms.position);
		}

		auto itVelocity = json.find("velocity");
		if (itVelocity != json.end()) {
			toVec(*itVelocity, transforms.velocity);
		}

		auto itOrientation = json.find("orientation");
		if (itOrientation != json.end()) {
			toQuat(*itOrientation, transforms.orientation);
		}

		auto itScale = json.find("scale");
		if (itScale != json.end()) {
			toVec(*itScale, transforms.scale);
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
	ResultOptional<CameraComponent> deserializeComponent<CameraComponent>(const nlohmann::json& json, DeserializeData&, Scene&)
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
	nlohmann::json serializeComponent<MeshComponent>(const MeshComponent& mesh, SerializeData&, std::ostream&)
	{
		nlohmann::json json;

		auto rMeshesJson = nlohmann::json::array();
		mesh.processRenderableIndices([&](std::size_t iMesh) {
			auto shadersVJson = nlohmann::json::array();
			mesh.processRenderableShaders(iMesh, [&](const auto& shader) {
				shadersVJson.push_back(shader.getResource().getName());
			});

			nlohmann::json meshJson;
			meshJson["hasSkinning"] = mesh.hasSkinning(iMesh);
			meshJson["mesh"] = mesh.getMesh(iMesh).getResource().getName();
			meshJson["primitive"] = static_cast<int>(mesh.get(iMesh).getPrimitiveType());
			meshJson["shaders"] = std::move(shadersVJson);
			rMeshesJson.emplace_back(std::move(meshJson));
		});
		json["rMeshes"] = std::move(rMeshesJson);

		return json;
	}

	template <>
	ResultOptional<MeshComponent> deserializeComponent<MeshComponent>(const nlohmann::json& json, DeserializeData&, Scene& scene)
	{
		MeshComponent mesh;

		auto itRMeshes = json.find("rMeshes");
		if (itRMeshes == json.end()) {
			return { Result(false, "Missing \"rMeshes\" property"), std::nullopt };
		}

		for (std::size_t i = 0; i < itRMeshes->size(); ++i) {
			const auto& rMesh = (*itRMeshes)[i];
			auto itHasSkinning = rMesh.find("hasSkinning");
			auto itMesh = rMesh.find("mesh");
			auto itPrimitive = rMesh.find("primitive");
			auto itShaders = rMesh.find("shaders");
			if (itHasSkinning == rMesh.end()) {
				return { Result(false, "Failed to parse rMeshes[" + std::to_string(i) + "]: Missing \"hasSkinning\" property"), std::nullopt };
			}
			if (itMesh == rMesh.end()) {
				return { Result(false, "Failed to parse rMeshes[" + std::to_string(i) + "]: Missing \"mesh\" property"), std::nullopt };
			}
			if (itPrimitive == rMesh.end()) {
				return { Result(false, "Failed to parse rMeshes[" + std::to_string(i) + "]: Missing \"primitive\" property"), std::nullopt };
			}
			if (itShaders == rMesh.end()) {
				return { Result(false, "Failed to parse rMeshes[" + std::to_string(i) + "]: Missing \"shaders\" property"), std::nullopt };
			}

			std::string key = *itMesh;
			auto value = scene.repository.findByName<Mesh>(key.c_str());
			if (!value) {
				return { Result(false, "Failed to parse rMeshes[" + std::to_string(i) + "]: key=" + key + " not found"), std::nullopt };
			}

			PrimitiveType primitive = static_cast<PrimitiveType>(itPrimitive->get<int>());
			bool hasSkinning = *itHasSkinning;
			std::size_t iMesh = mesh.add(hasSkinning, value, primitive);

			for (std::size_t j = 0; j < itShaders->size(); ++j) {
				auto jShader = (*itShaders)[j];

				std::string shaderKey = jShader.get<std::string>();
				auto shader = scene.repository.findByName<RenderableShader>(shaderKey.c_str());
				if (!shader) {
					return { Result(false, "Failed to parse rMeshes[" + std::to_string(i) + "]: shader=" + shaderKey + " not found"), std::nullopt };
				}

				mesh.addRenderableShader(iMesh, std::move(shader));
			}
		}

		return { Result(), std::move(mesh) };
	}


	template <>
	nlohmann::json serializeComponent<TerrainComponent>(const TerrainComponent& terrain, SerializeData&, std::ostream&)
	{
		nlohmann::json json;

		json["size"] = terrain.get().getSize();
		json["maxHeight"] = terrain.get().getMaxHeight();
		json["lodDistances"] = terrain.get().getLodDistances();

		auto shadersVJson = nlohmann::json::array();
		terrain.processRenderableShaders([&](const auto& shader) {
			shadersVJson.push_back(shader.getResource().getName());
		});
		json["shaders"] = std::move(shadersVJson);

		return json;
	}

	template <>
	ResultOptional<TerrainComponent> deserializeComponent<TerrainComponent>(const nlohmann::json& json, DeserializeData&, Scene& scene)
	{
		auto itSize = json.find("size");
		if (itSize == json.end()) {
			return { Result(false, "Missing \"size\" property"), std::nullopt };
		}

		auto itMaxHeight = json.find("maxHeight");
		if (itMaxHeight == json.end()) {
			return { Result(false, "Missing \"maxHeight\" property"), std::nullopt };
		}

		auto itLodDistances = json.find("lodDistances");
		if (itLodDistances == json.end()) {
			return { Result(false, "Missing \"lodDistances\" property"), std::nullopt };
		}

		auto itShaders = json.find("shaders");
		if (itShaders == json.end()) {
			return { Result(false, "Missing \"shaders\" property"), std::nullopt };
		}

		float size = *itSize;
		float maxHeight = *itMaxHeight;
		std::vector<float> lodDistances = *itLodDistances;
		TerrainComponent terrain(size, maxHeight, lodDistances);

		for (std::size_t i = 0; i < itShaders->size(); ++i) {
			std::string shaderKey = (*itShaders)[i];
			if (auto shader = scene.repository.findByName<RenderableShader>(shaderKey.c_str())) {
				terrain.addRenderableShader(shader);
			}
			else {
				return { Result(false, "RenderableShader \"" + shaderKey + "\" not found at shaders[" + std::to_string(i) + "]"), std::nullopt };
			}
		}

		return { Result(), std::move(terrain) };
	}


	template <>
	nlohmann::json serializeComponent<LightComponent>(const LightComponent& light, SerializeData&, std::ostream&)
	{
		nlohmann::json json;

		if (light.getSource()) {
			json["sourceName"] = light.getSource().getResource().getName();
		}

		return json;
	}

	template <>
	ResultOptional<LightComponent> deserializeComponent<LightComponent>(const nlohmann::json& json, DeserializeData&, Scene& scene)
	{
		LightComponent light;

		auto itSourceName = json.find("sourceName");
		if (itSourceName != json.end()) {
			std::string name = *itSourceName;
			if (auto source = scene.repository.findByName<LightSource>(name.c_str())) {
				light.setSource(source);
			}
			else {
				return { Result(false, "sourceName \"" + name + "\" not found"), std::nullopt };
			}
		}

		return { Result(), std::move(light) };
	}


	template <>
	nlohmann::json serializeComponent<LightProbeComponent>(const LightProbeComponent& lightProbe, SerializeData&, std::ostream&)
	{
		nlohmann::json json;

		if (lightProbe.irradianceMap) {
			json["irradianceMap"] = lightProbe.irradianceMap.getResource().getName();
		}

		if (lightProbe.prefilterMap) {
			json["prefilterMap"] = lightProbe.prefilterMap.getResource().getName();
		}

		return json;
	}

	template <>
	ResultOptional<LightProbeComponent> deserializeComponent<LightProbeComponent>(const nlohmann::json& json, DeserializeData&, Scene& scene)
	{
		LightProbeComponent lightProbe;

		auto itIrradianceMap = json.find("irradianceMap");
		if (itIrradianceMap != json.end()) {
			std::string keyIrradianceMap = *itIrradianceMap;
			if (auto irradianceMap = scene.repository.findByName<Texture>(keyIrradianceMap.c_str())) {
				lightProbe.irradianceMap = irradianceMap;
			}
			else {
				return { Result(false, "irradianceMap \"" + keyIrradianceMap + "\" not found"), std::nullopt };
			}
		}

		auto itPrefilterMap = json.find("prefilterMap");
		if (itPrefilterMap != json.end()) {
			std::string keyPrefilterMap = *itPrefilterMap;
			if (auto prefilterMap = scene.repository.findByName<Texture>(keyPrefilterMap.c_str())) {
				lightProbe.prefilterMap = prefilterMap;
			}
			else {
				return { Result(false, "prefilterMap \"" + keyPrefilterMap + "\" not found"), std::nullopt };
			}
		}

		return { Result(), std::move(lightProbe) };
	}


	nlohmann::json serializeCollider(const Collider& collider, SerializeData& data, std::ostream& dataStream)
	{
		nlohmann::json json;

		json["layers"] = collider.getLayers().to_string();
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
		else if (auto triangleMesh = dynamic_cast<const TriangleMeshCollider*>(&collider)) {
			nlohmann::json vertexBufferJson;
			serializeBuffer(
				reinterpret_cast<const std::byte*>( glm::value_ptr(triangleMesh->getVertices()[0]) ),
				3 * triangleMesh->getNumVertices() * sizeof(float),
				vertexBufferJson, dataStream
			);
			data.buffersJson.emplace_back(std::move(vertexBufferJson));

			nlohmann::json indexBufferJson;
			serializeBuffer(
				reinterpret_cast<const std::byte*>(triangleMesh->getIndices()),
				triangleMesh->getNumIndices() * sizeof(unsigned short),
				indexBufferJson, dataStream
			);
			data.buffersJson.emplace_back(std::move(indexBufferJson));

			json["type"] = "TriangleMeshCollider";
			json["vertices"] = data.buffersJson.size() - 2;
			json["indices"] = data.buffersJson.size() - 1;
		}
		else if (auto composite = dynamic_cast<const CompositeCollider*>(&collider)) {
			auto colliderPartsJson = nlohmann::json::array();
			composite->processParts([&](const Collider& part) {
				colliderPartsJson.emplace_back( serializeCollider(part, data, dataStream) );
			});

			json["type"] = "CompositeCollider";
			json["parts"] = std::move(colliderPartsJson);
		}

		return json;
	}

	ResultOptional<std::unique_ptr<Collider>> deserializeCollider(const nlohmann::json& json, DeserializeData& data)
	{
		std::unique_ptr<Collider> collider;

		auto itLayers = json.find("layers"), itType = json.find("type");
		if (itLayers == json.end()) {
			return { Result(false, "Missing \"layers\" property"), std::nullopt };
		}
		if (itType == json.end()) {
			return { Result(false, "Missing \"type\" property"), std::nullopt };
		}

		if (*itType == "BoundingBox") {
			auto itLengths = json.find("lengths");
			if (itLengths == json.end()) {
				return { Result(false, "Missing BoundingBox \"lengths\" property"), std::nullopt };
			}

			glm::vec3 lengths;
			if (!toVec(*itLengths, lengths)) {
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
				if (!toVec((*itVertices)[i], vertices[i])) {
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
				return { Result(false, "Missing TerrainCollider \"heights\" property"), std::nullopt };
			}

			std::size_t heights = *itHeights;
			if (heights >= data.buffersJson.size()) {
				return { Result(false, "Heights buffer " + std::to_string(heights) + " out of bounds"), std::nullopt };
			}

			auto itXSize = json.find("xSize");
			if (itXSize == json.end()) {
				return { Result(false, "Missing TerrainCollider \"xSize\" property"), std::nullopt };
			}

			auto itZSize = json.find("zSize");
			if (itZSize == json.end()) {
				return { Result(false, "Missing TerrainCollider \"zSize\" property"), std::nullopt };
			}

			std::vector<std::byte> buffer;
			deserializeBuffer(data.buffersJson[heights], data.dataStream, buffer);

			auto terrain = std::make_unique<TerrainCollider>();
			terrain->setHeights(reinterpret_cast<const float*>(buffer.data()), *itXSize, *itZSize);
			collider = std::move(terrain);
		}
		else if (*itType == "TriangleMeshCollider") {
			auto itVertices = json.find("vertices");
			if (itVertices == json.end()) {
				return { Result(false, "Missing TriangleMeshCollider \"vertices\" property"), std::nullopt };
			}

			std::size_t vertices = *itVertices;
			if (vertices >= data.buffersJson.size()) {
				return { Result(false, "Vertices buffer " + std::to_string(vertices) + " out of bounds"), std::nullopt };
			}

			std::vector<std::byte> vertexBuffer;
			deserializeBuffer(data.buffersJson[vertices], data.dataStream, vertexBuffer);

			auto itIndices = json.find("indices");
			if (itIndices == json.end()) {
				return { Result(false, "Missing TriangleMeshCollider \"indices\" property"), std::nullopt };
			}

			std::size_t indices = *itIndices;
			if (indices >= data.buffersJson.size()) {
				return { Result(false, "Indices buffer " + std::to_string(indices) + " out of bounds"), std::nullopt };
			}

			std::vector<std::byte> indexBuffer;
			deserializeBuffer(data.buffersJson[indices], data.dataStream, indexBuffer);

			auto triangleMesh = std::make_unique<TriangleMeshCollider>(
				reinterpret_cast<glm::vec3*>(vertexBuffer.data()), vertexBuffer.size() / sizeof(glm::vec3),
				reinterpret_cast<unsigned short*>(indexBuffer.data()), indexBuffer.size() / sizeof(unsigned short)
			);
			collider = std::move(triangleMesh);
		}
		else if (*itType == "CompositeCollider") {
			auto itParts = json.find("parts");
			if (itParts == json.end()) {
				return { Result(false, "Missing CompositeCollider \"parts\" property"), std::nullopt };
			}

			std::vector<std::unique_ptr<Collider>> parts;
			for (std::size_t i = 0; i < itParts->size(); ++i) {
				auto jsonPart = (*itParts)[i];
				auto [result, part] = deserializeCollider(jsonPart, data);
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

		std::bitset<Collider::kMaxLayers> layerMask(itLayers->get<std::string>());
		collider->setLayers(layerMask);

		return { Result(), std::move(collider) };
	}

	template <>
	nlohmann::json serializeComponent<RigidBodyComponent>(const RigidBodyComponent& rigidBody, SerializeData& data, std::ostream& dataStream)
	{
		auto forces = nlohmann::json::array();
		rigidBody.processForces([&](const auto& force) {
			forces.push_back(force.getResource().getName());
		});

		nlohmann::json json;
		json["type"] = static_cast<int>(rigidBody.get().getProperties().type);
		json["invertedMass"] = rigidBody.get().getProperties().invertedMass;
		json["invertedInertiaTensor"] = toJson(rigidBody.get().getProperties().invertedInertiaTensor);
		json["linearDrag"] = rigidBody.get().getProperties().linearDrag;
		json["angularDrag"] = rigidBody.get().getProperties().angularDrag;
		json["frictionCoefficient"] = rigidBody.get().getProperties().frictionCoefficient;
		json["sleepMotion"] = rigidBody.get().getProperties().sleepMotion;

		int bitmapStatus = rigidBody.get().getStatus(RigidBody::Status::Sleeping)? 1 : 0
			+ rigidBody.get().getStatus(RigidBody::Status::PropertiesChanged)?	1 << 1 : 0
			+ rigidBody.get().getStatus(RigidBody::Status::StateChanged)?		1 << 2 : 0
			+ rigidBody.get().getStatus(RigidBody::Status::ColliderChanged)?	1 << 3 : 0
			+ rigidBody.get().getStatus(RigidBody::Status::ForcesChanged)?		1 << 4 : 0;
		json["status"] = bitmapStatus;

		if (const Collider* collider = rigidBody.get().getCollider()) {
			nlohmann::json jsonCollider = serializeCollider(*collider, data, dataStream);
			jsonCollider["colliderLocalTransforms"] = toJson(rigidBody.get().getColliderLocalTransforms());
			json["collider"] = std::move(jsonCollider);
		}

		json["forces"] = std::move(forces);

		return json;
	}

	template <>
	ResultOptional<RigidBodyComponent> deserializeComponent<RigidBodyComponent>(const nlohmann::json& json, DeserializeData& data, Scene& scene)
	{
		RigidBodyProperties rbProperties;

		auto itType = json.find("type"),
			itInvertedMass = json.find("invertedMass"), itInvertedInertiaTensor = json.find("invertedInertiaTensor"),
			itLinearDrag = json.find("linearDrag"), itAngularDrag = json.find("angularDrag"),
			itFrictionCoefficient = json.find("frictionCoefficient"), itSleepMotion = json.find("sleepMotion"),
			itStatus = json.find("status"), itCollider = json.find("collider"), itForces = json.find("forces");

		if ((itType == json.end())
			|| (itInvertedMass == json.end()) || (itInvertedInertiaTensor == json.end())
			|| (itLinearDrag == json.end()) || (itAngularDrag == json.end())
			|| (itFrictionCoefficient == json.end()) || (itSleepMotion == json.end())
			|| (itStatus == json.end()) || (itForces == json.end())
		) {
			return { Result(false, "Missing properties"), std::nullopt };
		}

		rbProperties.type = static_cast<RigidBodyProperties::Type>(itType->get<int>());
		if (rbProperties.type == RigidBodyProperties::Type::Dynamic) {
			rbProperties.invertedMass = *itInvertedMass;
			if (!toMat(*itInvertedInertiaTensor, rbProperties.invertedInertiaTensor)) {
				return { Result(false, "Failed to parse the invertedInertiaTensor"), std::nullopt };
			}

			rbProperties.linearDrag = *itLinearDrag;
			rbProperties.angularDrag = *itAngularDrag;
		}

		rbProperties.frictionCoefficient = *itFrictionCoefficient;
		rbProperties.sleepMotion = *itSleepMotion;

		std::unique_ptr<Collider> collider;
		glm::mat4 colliderLocalTransforms(1.0f);
		if (itCollider != json.end()) {
			auto [result, collider2] = deserializeCollider(*itCollider, data);
			if (!result) {
				return { Result(false, "Failed to parse the collider: " + std::string(result.description())), std::nullopt };
			}
			collider = std::move(*collider2);

			auto itColliderLocalTransforms = itCollider->find("colliderLocalTransforms");
			if (itColliderLocalTransforms != itCollider->end()) {
				if (!toMat(*itColliderLocalTransforms, colliderLocalTransforms)) {
					return { Result(false, "Failed to parse colliderLocalTransforms"), std::nullopt };
				}
			}
		}

		RigidBodyComponent rigidBodyComponent(rbProperties);
		rigidBodyComponent.get().setCollider(std::move(collider));
		rigidBodyComponent.get().setColliderLocalTrasforms(colliderLocalTransforms);

		for (std::size_t i = 0; i < itForces->size(); ++i) {
			std::string forceJson = (*itForces)[i];
			auto force = scene.repository.findByName<Force>(forceJson.c_str());
			if (force) {
				rigidBodyComponent.addForce(force);
			}
			else {
				return { Result(false, "Failed to parse Force[" + std::to_string(i) + "]: Name " + forceJson + " not found"), std::nullopt };
			}
		}

		int bitmapStatus = *itStatus;
		rigidBodyComponent.get().setStatus(RigidBody::Status::Sleeping, (bitmapStatus & (1 << 0)) > 0);
		rigidBodyComponent.get().setStatus(RigidBody::Status::PropertiesChanged, (bitmapStatus & (1 << 1)) > 0);
		rigidBodyComponent.get().setStatus(RigidBody::Status::StateChanged, (bitmapStatus & (1 << 2)) > 0);
		rigidBodyComponent.get().setStatus(RigidBody::Status::ColliderChanged, (bitmapStatus & (1 << 3)) > 0);
		rigidBodyComponent.get().setStatus(RigidBody::Status::ForcesChanged, (bitmapStatus & (1 << 4)) > 0);

		return { Result(), std::move(rigidBodyComponent) };
	}


	template <>
	nlohmann::json serializeComponent<SkinComponent>(const SkinComponent& skin, SerializeData& data, std::ostream&)
	{
		nlohmann::json json;

		auto itNodeIndex = data.nodeIndexMap.find(skin.getRoot());
		if ((itNodeIndex != data.nodeIndexMap.end()) && skin.getSkin()) {
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
			json["skin"] = skin.getSkin().getResource().getName();
		}

		return json;
	}

	template <>
	ResultOptional<SkinComponent> deserializeComponent<SkinComponent>(const nlohmann::json& json, DeserializeData& data, Scene& scene)
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

		auto skin = scene.repository.findByName<Skin>(itSkin->get<std::string>().c_str());
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
			animation.processSAnimators([&](const auto& sAnimator) {
				animatorsJson.emplace_back(sAnimator.getResource().getName());
			});

			json["root"] = itNodeIndex->second;
			json["animators"] = std::move(animatorsJson);
		}

		return json;
	}

	template <>
	ResultOptional<AnimationComponent> deserializeComponent<AnimationComponent>(const nlohmann::json& json, DeserializeData& data, Scene& scene)
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
			auto animator = scene.repository.findByName<SkeletonAnimator>((*itAnimators)[i].get<std::string>().c_str());
			if (!animator) {
				return { Result(false, "SkeletonAnimator " + (*itAnimators)[i].get<std::string>() + " not found"), std::nullopt };
			}

			animation.addAnimator(animator);
		}
		return { Result(), std::move(animation) };
	}


	template <>
	nlohmann::json serializeComponent<ParticleSystemComponent>(const ParticleSystemComponent& particleSystem, SerializeData&, std::ostream&)
	{
		nlohmann::json json;

		if (auto mesh = particleSystem.getMesh()) {
			json["mesh"] = mesh.getResource().getName();
		}

		auto shadersVJson = nlohmann::json::array();
		particleSystem.processRenderableShaders([&](const auto& shader) {
			shadersVJson.push_back(shader.getResource().getName());
		});
		json["shaders"] = std::move(shadersVJson);

		if (particleSystem.getEmitter()) {
			json["emitter"] = particleSystem.getEmitter().getResource().getName();
		}

		return json;
	}

	template <>
	ResultOptional<ParticleSystemComponent> deserializeComponent<ParticleSystemComponent>(const nlohmann::json& json, DeserializeData&, Scene& scene)
	{
		ParticleSystemComponent particleSystem;

		auto itMesh = json.find("mesh");
		if (itMesh == json.end()) {
			return { Result(false, "Missing \"mesh\" property"), std::nullopt };
		}

		if (auto mesh = scene.repository.findByName<Mesh>(itMesh->get<std::string>().c_str())) {
			particleSystem.setMesh(mesh);
		}

		auto itShaders = json.find("shaders");
		if (itShaders == json.end()) {
			return { Result(false, "Missing \"shaders\" property"), std::nullopt };
		}

		for (std::size_t i = 0; i < itShaders->size(); ++i) {
			std::string shaderName = (*itShaders)[i];

			if (auto shader = scene.repository.findByName<RenderableShader>(shaderName.c_str())) {
				particleSystem.addRenderableShader(shader);
			}
			else {
				return { Result(false, "RenderableShader \"" + shaderName + "\" not found at shaders[" + std::to_string(i) + "]"), std::nullopt };
			}
		}

		auto itEmitter = json.find("emitter");
		if (itEmitter == json.end()) {
			return { Result(false, "Missing \"emitter\" property"), std::nullopt };
		}

		if (auto emitter = scene.repository.findByName<ParticleEmitter>(itEmitter->get<std::string>().c_str())) {
			particleSystem.setEmitter(emitter);
		}

		return { Result(), std::move(particleSystem) };
	}


	template <>
	nlohmann::json serializeComponent<AudioSourceComponent>(const AudioSourceComponent& audioSource, SerializeData&, std::ostream&)
	{
		nlohmann::json json;

		if (auto buffer = audioSource.getBuffer()) {
			json["bufferName"] = buffer.getResource().getName();
		}

		return json;
	}

	template <>
	ResultOptional<AudioSourceComponent> deserializeComponent<AudioSourceComponent>(const nlohmann::json& json, DeserializeData&, Scene& scene)
	{
		AudioSourceComponent audioSource;

		auto itBufferName = json.find("bufferName");
		if (itBufferName != json.end()) {
			std::string bufferName = *itBufferName;
			if (auto buffer = scene.repository.findByName<Buffer>(bufferName.c_str())) {
				audioSource.setBuffer(buffer);
			}
			else {
				return { Result(false, "Buffer \"" + bufferName + "\" not found"), std::nullopt };
			}
		}

		return { Result(), std::move(audioSource) };
	}


	template <>
	nlohmann::json serializeComponent<ScriptComponent>(const ScriptComponent& scriptC, SerializeData&, std::ostream&)
	{
		nlohmann::json json;

		if (auto script = scriptC.getScript()) {
			json["scriptName"] = script.getResource().getName();
		}

		return json;
	}

	template <>
	ResultOptional<ScriptComponent> deserializeComponent<ScriptComponent>(const nlohmann::json& json, DeserializeData&, Scene& scene)
	{
		ScriptComponent scriptC;

		auto itScriptName = json.find("scriptName");
		if (itScriptName != json.end()) {
			std::string scriptName = *itScriptName;
			if (auto script = scene.repository.findByName<Script>(scriptName.c_str())) {
				scriptC.setScript(script);
			}
			else {
				return { Result(false, "Script \"" + scriptName + "\" not found"), std::nullopt };
			}
		}

		return { Result(), std::move(scriptC) };
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
				toVec(*itPosition, nodeData.localTransforms.position);
			}
			auto itOrientation = itLocal->find("orientation");
			if (itOrientation != itLocal->end()) {
				toQuat(*itOrientation, nodeData.localTransforms.orientation);
			}
			auto itScale = itLocal->find("scale");
			if (itScale != itLocal->end()) {
				toVec(*itScale, nodeData.localTransforms.scale);
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

		data.scene.repository.iterate<T>([&](const Repository::ResourceRef<T>& r) {
			nlohmann::json resourceJson;
			resourceJson["name"] = r.getResource().getName();
			if (r.getResource().isLinked()) {
				resourceJson["linkedFile"] = r.getResource().getLinkedFile();
			}
			else {
				serializeResource<T>(r, data, resourceJson, dataStream);
			}
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
				Repository::ResourceRef<T> r;

				auto itName = resourceJson.find("name");
				if (itName == resourceJson.end()) {
					return Result(false, "Failed to deserialize " + tag + "[" + std::to_string(i) + "]: Missing name");
				}

				auto itLinkedFile = resourceJson.find("linkedFile");
				if (itLinkedFile != resourceJson.end()) {
					std::size_t linkedFile = *itLinkedFile;
					linkedFile = data.linkedScenesMap[linkedFile];
					r.getResource().setLinkedFile(linkedFile);

					return Result(false, "Failed to deserialize " + tag + "[" + std::to_string(i) + "]: TODO: LINKED FILES NOT IMPLEMENTED YET");
				}
				else {
					if (auto result = deserializeResource<T>(resourceJson, r, data, scene); !result) {
						return Result(false, "Failed to deserialize " + tag + "[" + std::to_string(i) + "]: " + result.description());
					}

					r.getResource().setName(itName->get<std::string>().c_str());
				}

				r.getResource().setName(itName->get<std::string>().c_str());
				r.setFakeUser();
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
			bool componentEnabled = data.scene.application.getEntityDatabase().hasComponentsEnabled<T>(entity);

			if (component) {
				nlohmann::json componentJson = serializeComponent<T>(*component, data, dataStream);
				componentJson["entity"] = index;
				componentJson["enabled"] = componentEnabled;
				componentsVJson.emplace_back(std::move(componentJson));
			}
		}

		if (!componentsVJson.empty()) {
			json[tag] = std::move(componentsVJson);
		}
	}

	template <typename T>
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

				auto itEnabled = componentJson.find("enabled");
				if (itEnabled == componentJson.end()) {
					return Result(false, "Failed to deserialize " + tag + "[" + std::to_string(i) + "]: Missing \"enabled\" property");
				}

				bool enabled = itEnabled->get<bool>();
				auto [result, component] = deserializeComponent<T>(componentJson, data, scene);
				if (!result) {
					return Result(false, "Failed to deserialize " + tag + "[" + std::to_string(i) + "]: " + result.description());
				}
				scene.application.getEntityDatabase().addComponent<T>(itEntity2->second, std::move(*component), enabled);
			}
		}

		return Result();
	}


	void serializeLinkedFiles(SerializeData& data, nlohmann::json& json)
	{
		json["linkedFiles"] = data.scene.linkedScenePaths;
	}

	Result deserializeLinkedFiles(DeserializeData& data, Scene& scene)
	{
		auto itLinkedFiles = data.json.find("linkedFiles");
		if (itLinkedFiles == data.json.end()) {
			return Result(false, "Missing \"linkedFiles\" property");
		}

		std::vector<std::string> linkedFiles = *itLinkedFiles;
		for (std::size_t i = 0; i < linkedFiles.size(); ++i) {
			auto itFile = std::find(scene.linkedScenePaths.begin(), scene.linkedScenePaths.end(), linkedFiles[i]);
			if (itFile != scene.linkedScenePaths.end()) {
				data.linkedScenesMap.emplace(i, std::distance(scene.linkedScenePaths.begin(), itFile));
			}
			else {
				scene.linkedScenePaths.push_back(linkedFiles[i]);
				data.linkedScenesMap.emplace(i, scene.linkedScenePaths.size() - 1);
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
		serializeRVector<Texture>("textures", data, json, dataStream);
		serializeRVector<Program>("programs", data, json, dataStream);
		serializeRVector<RenderableShaderStep>("steps", data, json, dataStream);
		serializeRVector<RenderableShader>("shaders", data, json, dataStream);
		serializeRVector<Force>("forces", data, json, dataStream);
		serializeRVector<ParticleEmitter>("particleEmitter", data, json, dataStream);
		serializeRVector<Buffer>("audioBuffers", data, json, dataStream);
	}

	Result deserializeRepository(DeserializeData& data, Scene& scene)
	{
		if (auto result = deserializeRVector<Mesh>("meshes", data, scene); !result) { return result; }
		if (auto result = deserializeRVector<Skin>("skins", data, scene); !result) { return result; }
		if (auto result = deserializeRVector<SkeletonAnimator>("skeletonAnimators", data, scene); !result) { return result; }
		if (auto result = deserializeRVector<LightSource>("lightSources", data, scene); !result) { return result; }
		if (auto result = deserializeRVector<Texture>("textures", data, scene); !result) { return result; }
		if (auto result = deserializeRVector<Program>("programs", data, scene); !result) { return result; }
		if (auto result = deserializeRVector<RenderableShaderStep>("steps", data, scene); !result) { return result; }
		if (auto result = deserializeRVector<RenderableShader>("shaders", data, scene); !result) { return result; }
		if (auto result = deserializeRVector<Force>("forces", data, scene); !result) { return result; }
		if (auto result = deserializeRVector<ParticleEmitter>("particleEmitter", data, scene); !result) { return result; }
		if (auto result = deserializeRVector<Buffer>("audioBuffers", data, scene); !result) { return result; }
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
		serializeCVector<TerrainComponent>("terrainComponents", data, json, dataStream);
		serializeCVector<LightComponent>("lights", data, json, dataStream);
		serializeCVector<LightProbeComponent>("lightProbes", data, json, dataStream);
		serializeCVector<RigidBodyComponent>("rigidBodies", data, json, dataStream);
		serializeCVector<SkinComponent>("skinComponents", data, json, dataStream);
		serializeCVector<AnimationComponent>("animationComponents", data, json, dataStream);
		serializeCVector<ParticleSystemComponent>("particleSystemComponents", data, json, dataStream);
		serializeCVector<AudioSourceComponent>("audioSourceComponents", data, json, dataStream);
		serializeCVector<ScriptComponent>("scriptComponents", data, json, dataStream);
	}

	Result deserializeComponents(DeserializeData& data, Scene& scene)
	{
		if (auto result = deserializeCVector<TagComponent>("tags", data, scene); !result) { return result; }
		if (auto result = deserializeCVector<TransformsComponent>("transforms", data, scene); !result) { return result; }
		if (auto result = deserializeCVector<CameraComponent>("cameras", data, scene); !result) { return result; }
		if (auto result = deserializeCVector<MeshComponent>("meshComponents", data, scene); !result) { return result; }
		if (auto result = deserializeCVector<TerrainComponent>("terrainComponents", data, scene); !result) { return result; }
		if (auto result = deserializeCVector<LightComponent>("lights", data, scene); !result) { return result; }
		if (auto result = deserializeCVector<LightProbeComponent>("lightProbes", data, scene); !result) { return result; }
		if (auto result = deserializeCVector<RigidBodyComponent>("rigidBodies", data, scene); !result) { return result; }
		if (auto result = deserializeCVector<SkinComponent>("skinComponents", data, scene); !result) { return result; }
		if (auto result = deserializeCVector<AnimationComponent>("animationComponents", data, scene); !result) { return result; }
		if (auto result = deserializeCVector<ParticleSystemComponent>("particleSystemComponents", data, scene); !result) { return result; }
		if (auto result = deserializeCVector<AudioSourceComponent>("audioSourceComponents", data, scene); !result) { return result; }
		if (auto result = deserializeCVector<ScriptComponent>("scriptComponents", data, scene); !result) { return result; }
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
		serializeLinkedFiles(data, outputJson);
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
		DeserializeData data = { std::move(json), std::move(dataStream), {}, {}, {}, buffers, accessors };
		if (auto result = deserializeLinkedFiles(data, output); !result) {
			return Result(false, "Failed to deserialized the linked files: " + std::string(result.description()));
		}
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
