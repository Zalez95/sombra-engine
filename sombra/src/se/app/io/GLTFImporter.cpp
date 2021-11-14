#include <sstream>
#include <fstream>
#include <algorithm>
#include "GLMJSON.h"
#include "GLTFImporter.h"
#include "se/utils/MathUtils.h"
#include "se/app/TagComponent.h"
#include "se/app/TransformsComponent.h"
#include "se/app/AnimationComponent.h"
#include "se/app/io/ImageReader.h"
#include "se/app/graphics/RawMesh.h"
#include "se/app/events/ContainerEvent.h"
#include "se/animation/StepAnimations.h"
#include "se/animation/LinearAnimations.h"
#include "se/animation/CubicSplineAnimations.h"
#include "se/animation/TransformationAnimator.h"
#include "se/animation/AnimationEngine.h"

namespace se::app {

	struct GLTFImporter::Accessor
	{
		union BoundsComponentType { float f; int i; };
		using BoundsType = utils::FixedVector<BoundsComponentType, 16>;

		std::size_t bufferViewId, byteOffset, count, numComponents;
		graphics::TypeId componentTypeId;
		bool normalized;
		BoundsType minimum, maximum;
	};


	struct GLTFImporter::BufferView
	{
		std::size_t bufferId, length, offset, stride;
		enum class Target { Array, ElementArray, Undefined } target;
	};


	struct GLTFImporter::Sampler
	{
		graphics::TextureFilter filters[2];
		graphics::TextureWrap wraps[2];
	};


	struct GLTFImporter::MaterialShader
	{
		std::string name;
		Material material;
		ShaderRef shader;
		ShaderRef shaderSkin;
	};


	struct GLTFImporter::Node
	{
		animation::NodeData nodeData;
		std::vector<std::size_t> children;
		Entity entity = kNullEntity;
		bool hasSkin = false;
		std::size_t skinIndex = 0;
		animation::AnimationNode* animationNode = nullptr;
	};


	struct GLTFImporter::PrimitiveMeshData
	{
		std::array<bool, MeshAttributes::NumAttributes> hasAttribute = {};
		std::array<std::size_t, MeshAttributes::NumAttributes> attributeAccessor = {};
		std::size_t indicesIndex;

		friend bool operator==(
			const PrimitiveMeshData& lhs, const PrimitiveMeshData& rhs
		) {
			return (lhs.hasAttribute == rhs.hasAttribute)
				&& (lhs.attributeAccessor == rhs.attributeAccessor)
				&& (lhs.indicesIndex == rhs.indicesIndex);
		};

		struct MyHash
		{
			std::size_t operator()(const PrimitiveMeshData& v) const noexcept
			{
				std::size_t ret = 0;
				for (std::size_t i = 0; i < MeshAttributes::NumAttributes; ++i) {
					utils::hash_combine(ret, v.hasAttribute[i]);
					utils::hash_combine(ret, v.attributeAccessor[i]);
				}
				utils::hash_combine(ret, v.indicesIndex);
				return ret;
			}
		};
	};


	struct GLTFImporter::PrimitiveData
	{
		MeshRef mesh;
		graphics::PrimitiveType primitiveType = graphics::PrimitiveType::Triangle;
		bool hasSkin = false;
		ShaderRef shader;
	};


	struct GLTFImporter::GLTFData
	{
		std::string fileName;
		std::string basePath;
		Scene& scene;

		std::vector<Accessor> accessors;
		std::vector<Buffer> buffers;
		std::vector<BufferView> bufferViews;
		std::vector<Sampler> samplers;
		std::vector<Image<unsigned char>> images;
		std::vector<TextureRef> textures;
		std::vector<MaterialShader> materials;
		std::vector<MeshRef> meshes;
		std::unordered_map<
			PrimitiveMeshData, std::size_t,
			PrimitiveMeshData::MyHash
		> primitiveMeshes;
		std::vector<std::vector<PrimitiveData>> primitives;
		std::vector<LightSourceRef> lightSources;
		std::vector<SkinRef> skins;
		std::vector<IndexVector> jointIndices;
		std::vector<CameraComponent> cameraComponents;
		std::vector<Node> nodes;
		std::vector<SceneUPtr> scenes;
		std::vector<SAnimatorRef> skeletonAnimators;

		GLTFData(Scene& scene) : scene(scene) {};
	};


	enum class InterpolationType { Linear, Step, CubicSpline };


	constexpr bool toTypeId(int code, graphics::TypeId& typeId)
	{
		bool ret = true;
		switch (code) {
			case 5120:	typeId = graphics::TypeId::Byte;			break;
			case 5121:	typeId = graphics::TypeId::UnsignedByte;	break;
			case 5122:	typeId = graphics::TypeId::Short;			break;
			case 5123:	typeId = graphics::TypeId::UnsignedShort;	break;
			case 5125:	typeId = graphics::TypeId::UnsignedInt;		break;
			case 5126:	typeId = graphics::TypeId::Float;			break;
			default:	ret = false;
		}
		return ret;
	}

	constexpr bool toTypeSize(int code, std::size_t& size)
	{
		bool ret = true;
		switch (code) {
			case 5120:	size = 1;	break;
			case 5121:	size = 1;	break;
			case 5122:	size = 2;	break;
			case 5123:	size = 2;	break;
			case 5125:	size = 4;	break;
			case 5126:	size = 4;	break;
			default:	ret = false;
		}
		return ret;
	}

	constexpr bool toTextureFilter(int code, graphics::TextureFilter& textureFilter)
	{
		bool ret = true;
		switch (code) {
			case 9728:	textureFilter = graphics::TextureFilter::Nearest;				break;
			case 9729:	textureFilter = graphics::TextureFilter::Linear;				break;
			case 9984:	textureFilter = graphics::TextureFilter::NearestMipMapNearest;	break;
			case 9985:	textureFilter = graphics::TextureFilter::LinearMipMapNearest;	break;
			case 9986:	textureFilter = graphics::TextureFilter::NearestMipMapLinear;	break;
			case 9987:	textureFilter = graphics::TextureFilter::LinearMipMapLinear;	break;
			default:	ret = false;
		}
		return ret;
	}

	constexpr bool toTextureWrap(int code, graphics::TextureWrap& textureWrap)
	{
		bool ret = true;
		switch (code) {
			case 10497:	textureWrap = graphics::TextureWrap::Repeat;			break;
			case 33648:	textureWrap = graphics::TextureWrap::MirroredRepeat;	break;
			case 33071:	textureWrap = graphics::TextureWrap::ClampToEdge;		break;
			default:	ret = false;
		}
		return ret;
	}

	inline bool toMeshAttribute(const std::string& text, unsigned int& meshAttribute)
	{
		bool ret = true;
		if (text == "POSITION")			{ meshAttribute = MeshAttributes::PositionAttribute; }
		else if (text == "NORMAL")		{ meshAttribute = MeshAttributes::NormalAttribute; }
		else if (text == "TANGENT")		{ meshAttribute = MeshAttributes::TangentAttribute; }
		else if (text == "TEXCOORD_0")	{ meshAttribute = MeshAttributes::TexCoordAttribute0; }
		else if (text == "TEXCOORD_1")	{ meshAttribute = MeshAttributes::TexCoordAttribute1; }
		else if (text == "COLOR_0")		{ meshAttribute = MeshAttributes::ColorAttribute; }
		else if (text == "JOINTS_0")	{ meshAttribute = MeshAttributes::JointIndexAttribute; }
		else if (text == "WEIGHTS_0")	{ meshAttribute = MeshAttributes::JointWeightAttribute; }
		else { ret = false; }
		return ret;
	}

	inline bool toPrimitiveType(int mode, graphics::PrimitiveType& primitiveType)
	{
		bool ret = true;
		switch (mode) {
			case 0:	primitiveType = graphics::PrimitiveType::Point;			break;
			case 1:	primitiveType = graphics::PrimitiveType::Line;			break;
			case 2:	primitiveType = graphics::PrimitiveType::LineLoop;		break;
			case 3:	primitiveType = graphics::PrimitiveType::LineStrip;		break;
			case 4:	primitiveType = graphics::PrimitiveType::Triangle;		break;
			case 5:	primitiveType = graphics::PrimitiveType::TriangleStrip;	break;
			case 6:	primitiveType = graphics::PrimitiveType::TriangleFan;	break;
			default:	ret = false;
		}
		return ret;
	}

	inline bool toNumComponents(const std::string& text, std::size_t& numComponents)
	{
		bool ret = true;
		if (text == "SCALAR")		{ numComponents = 1; }
		else if (text == "VEC2")	{ numComponents = 2; }
		else if (text == "VEC3")	{ numComponents = 3; }
		else if (text == "VEC4")	{ numComponents = 4; }
		else if (text == "MAT2")	{ numComponents = 4; }
		else if (text == "MAT3")	{ numComponents = 9; }
		else if (text == "MAT4")	{ numComponents = 16; }
		else { ret = false; }
		return ret;
	}

	inline bool toAlphaMode(const std::string& text, graphics::AlphaMode& alphaMode)
	{
		bool ret = true;
		if (text == "OPAQUE")		{ alphaMode = graphics::AlphaMode::Opaque; }
		else if (text == "MASK")	{ alphaMode = graphics::AlphaMode::Mask; }
		else if (text == "BLEND")	{ alphaMode = graphics::AlphaMode::Blend; }
		else { ret = false; }
		return ret;
	}

	inline bool toInterpolationType(const std::string& text, InterpolationType& interpolationType)
	{
		bool ret = true;
		if (text == "LINEAR")			{ interpolationType = InterpolationType::Linear; }
		else if (text == "STEP")		{ interpolationType = InterpolationType::Step; }
		else if (text == "CUBICSPLINE")	{ interpolationType = InterpolationType::CubicSpline; }
		else { ret = false; }
		return ret;
	}

	inline bool toTransformationType(const std::string& text, animation::TransformationAnimator::TransformationType& transformationType)
	{
		bool ret = true;
		if (text == "translation")		{ transformationType = animation::TransformationAnimator::TransformationType::Translation; }
		else if (text == "rotation")	{ transformationType = animation::TransformationAnimator::TransformationType::Rotation; }
		else if (text == "scale")		{ transformationType = animation::TransformationAnimator::TransformationType::Scale; }
		else { ret = false; }
		return ret;
	}


	GLTFImporter::GLTFImporter(ShaderBuilder& shaderBuilder) : SceneImporter(shaderBuilder) {}


	GLTFImporter::~GLTFImporter() {}


	Result GLTFImporter::load(const std::string& path, Scene& output)
	{
		Result result;

		// Create the temporary data of the GLTF file
		mGLTFData = std::make_unique<GLTFData>(output);
		mGLTFData->basePath = path.substr(0, path.find_last_of("/\\") + 1);
		mGLTFData->fileName = path.substr(mGLTFData->basePath.size(), path.size());
		createDefaultShaders(mGLTFData->scene);

		// Parse the file
		nlohmann::json jsonGLTF;
		if (!(result = readJSON(path, jsonGLTF)) || !(result = parseGLTF(jsonGLTF))) {
			result = Result(false, "Error while parsing the GLTF file \"" + path + "\": " + result.description());
		}

		// Clean the temporary data
		mGLTFData = nullptr;

		return result;
	}

// Private functions
	Result GLTFImporter::readJSON(const std::string& path, nlohmann::json& output)
	{
		std::ifstream inputstream(path);
		if (!inputstream.good()) {
			return Result(false, "Can't open the GLTF file");
		}

		try {
			inputstream >> output;
		}
		catch(const nlohmann::json::parse_error& e) {
			return Result(false, "Failed to parse the JSON file: " + std::string(e.what()));
		}

		return Result();
	}


	Result GLTFImporter::parseGLTF(const nlohmann::json& jsonGLTF)
	{
		auto itAsset = jsonGLTF.find("asset");
		if (itAsset == jsonGLTF.end()) {
			return Result(false, "GLTF file must have an asset property");
		}

		if (!checkAssetVersion(*itAsset, FileFormat::sVersion, FileFormat::sRevision)) {
			return Result(false, "Asset version not supported");
		}

		if (auto itBuffers = jsonGLTF.find("buffers"); itBuffers != jsonGLTF.end()) {
			mGLTFData->buffers.reserve(itBuffers->size());
			for (std::size_t bufferId = 0; bufferId < itBuffers->size(); ++bufferId) {
				Result result = parseBuffer( (*itBuffers)[bufferId] );
				if (!result) {
					return Result(false, "Failed to read the buffers property at buffer " + std::to_string(bufferId) + ": " + result.description());
				}
			}
		}

		if (auto itBufferViews = jsonGLTF.find("bufferViews"); itBufferViews != jsonGLTF.end()) {
			mGLTFData->bufferViews.reserve(itBufferViews->size());
			for (std::size_t bufferViewId = 0; bufferViewId < itBufferViews->size(); ++bufferViewId) {
				Result result = parseBufferView( (*itBufferViews)[bufferViewId] );
				if (!result) {
					return Result(false, "Failed to read the bufferViews property at bufferView " + std::to_string(bufferViewId) + ": " + result.description());
				}
			}
		}

		if (auto itAccessors = jsonGLTF.find("accessors"); itAccessors != jsonGLTF.end()) {
			mGLTFData->accessors.reserve(itAccessors->size());
			for (std::size_t accessorId = 0; accessorId < itAccessors->size(); ++accessorId) {
				Result result = parseAccessor( (*itAccessors)[accessorId] );
				if (!result) {
					return Result(false, "Failed to read the accessors property at accessor " + std::to_string(accessorId) + ": " + result.description());
				}
			}
		}

		if (auto itSamplers = jsonGLTF.find("samplers"); itSamplers != jsonGLTF.end()) {
			mGLTFData->samplers.reserve(itSamplers->size());
			for (std::size_t samplerId = 0; samplerId < itSamplers->size(); ++samplerId) {
				Result result = parseSampler( (*itSamplers)[samplerId] );
				if (!result) {
					return Result(false, "Failed to read the samplers property at sampler " + std::to_string(samplerId) + ": " + result.description());
				}
			}
		}

		if (auto itImages = jsonGLTF.find("images"); itImages != jsonGLTF.end()) {
			mGLTFData->images.reserve(itImages->size());
			for (std::size_t imageId = 0; imageId < itImages->size(); ++imageId) {
				Result result = parseImage((*itImages)[imageId]);
				if (!result) {
					return Result(false, "Failed to read the images property at image " + std::to_string(imageId) + ": " + result.description());
				}
			}
		}

		if (auto itTextures = jsonGLTF.find("textures"); itTextures != jsonGLTF.end()) {
			mGLTFData->textures.reserve(itTextures->size());
			for (std::size_t textureId = 0; textureId < itTextures->size(); ++textureId) {
				Result result = parseTexture((*itTextures)[textureId]);
				if (!result) {
					return Result(false, "Failed to read the textures property at texture " + std::to_string(textureId) + ": " + result.description());
				}
			}
		}

		if (auto itMaterials = jsonGLTF.find("materials"); itMaterials != jsonGLTF.end()) {
			mGLTFData->materials.reserve(itMaterials->size());
			for (std::size_t materialId = 0; materialId < itMaterials->size(); ++materialId) {
				Result result = parseMaterial((*itMaterials)[materialId]);
				if (!result) {
					return Result(false, "Failed to read the materials property at material " + std::to_string(materialId) + ": " + result.description());
				}
			}
		}

		if (auto itMeshes = jsonGLTF.find("meshes"); itMeshes != jsonGLTF.end()) {
			for (std::size_t meshId = 0; meshId < itMeshes->size(); ++meshId) {
				Result result = parseMesh((*itMeshes)[meshId]);
				if (!result) {
					return Result(false, "Failed to read the meshes property at mesh " + std::to_string(meshId) + ": " + result.description());
				}
			}
		}

		if (auto itExtensions = jsonGLTF.find("extensions"); itExtensions != jsonGLTF.end()) {
			if (auto itKHRLights = itExtensions->find("KHR_lights_punctual"); itKHRLights != itExtensions->end()) {
				Result result = parseKHRLights(*itKHRLights);
				if (!result) {
					return Result(false, std::string("Failed to read the KHR_lights_punctual extension: ") + result.description());
				}
			}
		}

		if (auto itSkins = jsonGLTF.find("skins"); itSkins != jsonGLTF.end()) {
			mGLTFData->skins.reserve(itSkins->size());
			mGLTFData->jointIndices.reserve(itSkins->size());
			for (std::size_t skinId = 0; skinId < itSkins->size(); ++skinId) {
				Result result = parseSkin( (*itSkins)[skinId] );
				if (!result) {
					return Result(false, "Failed to read the skins property at skin " + std::to_string(skinId) + ": " + result.description());
				}
			}
		}

		if (auto itCameras = jsonGLTF.find("cameras"); itCameras != jsonGLTF.end()) {
			mGLTFData->cameraComponents.reserve(itCameras->size());
			for (std::size_t cameraId = 0; cameraId < itCameras->size(); ++cameraId) {
				Result result = parseCamera((*itCameras)[cameraId]);
				if (!result) {
					return Result(false, "Failed to read the cameras property at camera " + std::to_string(cameraId) + ": " + result.description());
				}
			}
		}

		if (auto itNodes = jsonGLTF.find("nodes"); itNodes != jsonGLTF.end()) {
			mGLTFData->nodes.reserve(itNodes->size());
			for (std::size_t nodeId = 0; nodeId < itNodes->size(); ++nodeId) {
				Result result = parseNode((*itNodes)[nodeId]);
				if (!result) {
					return Result(false, "Failed to read the nodes property at node " + std::to_string(nodeId) + ": " + result.description());
				}
			}
		}

		if (auto itScenes = jsonGLTF.find("scenes"); itScenes != jsonGLTF.end()) {
			mGLTFData->scenes.reserve(itScenes->size());
			for (std::size_t sceneId = 0; sceneId < itScenes->size(); ++sceneId) {
				Result result = parseScene((*itScenes)[sceneId]);
				if (!result) {
					return Result(false, "Failed to read the scenes property at scene " + std::to_string(sceneId) + ": " + result.description());
				}
			}
		}

		if (auto itAnimations = jsonGLTF.find("animations"); itAnimations != jsonGLTF.end()) {
			mGLTFData->skeletonAnimators.reserve(itAnimations->size());
			for (std::size_t animationId = 0; animationId < itAnimations->size(); ++animationId) {
				Result result = parseAnimation((*itAnimations)[animationId]);
				if (!result) {
					return Result(false, "Failed to read the animations property at animation " + std::to_string(animationId) + ": " + result.description());
				}
			}
		}

		// Add the SkinComponents to the node Entities
		for (Node& node : mGLTFData->nodes) {
			if (node.hasSkin) {
				SkinComponent::MapNodeJoint nodeJointMap;
				for (std::size_t jointIndex = 0; jointIndex < mGLTFData->jointIndices[node.skinIndex].size(); ++jointIndex) {
					std::size_t nodeIndex = mGLTFData->jointIndices[node.skinIndex][jointIndex];
					nodeJointMap.emplace_back(mGLTFData->nodes[nodeIndex].animationNode, jointIndex);
				}

				mGLTFData->scene.application.getEntityDatabase().emplaceComponent<SkinComponent>(
					node.entity,
					node.animationNode, mGLTFData->skins[node.skinIndex], std::move(nodeJointMap)
				);
			}
		}

		return Result();
	}


	bool GLTFImporter::checkAssetVersion(const nlohmann::json& jsonAsset, int version, int revision)
	{
		bool valid = false;

		auto itVersion = jsonAsset.find("version");
		if (itVersion != jsonAsset.end()) {
			int assetVersion, assetRevision;
			char dot;
			std::istringstream(itVersion->get<std::string>()) >> assetVersion >> dot >> assetRevision;
			if (dot == '.') {
				if (assetVersion < version) {
					valid = true;
				}
				else if ((assetVersion == version) && (assetRevision <= revision)) {
					valid = true;
				}
			}
		}

		return valid;
	}


	Result GLTFImporter::parseBuffer(const nlohmann::json& jsonBuffer)
	{
		auto itByteLength = jsonBuffer.find("byteLength");
		auto itUri = jsonBuffer.find("uri");

		if ((itByteLength != jsonBuffer.end()) && (itUri != jsonBuffer.end())) {
			std::size_t size = *itByteLength;
			std::string path = *itUri;

			std::ifstream dataFileStream(mGLTFData->basePath + path, std::ios::in | std::ios::binary);
			if (dataFileStream.good()) {
				Buffer buffer(size);
				dataFileStream.read(reinterpret_cast<char*>(buffer.data()), size);
				if (!dataFileStream.fail()) {
					mGLTFData->buffers.emplace_back(std::move(buffer));
					return Result();
				}
				else {
					return Result(false, "Failed to read buffer file");
				}
			}
			else {
				return Result(false, "Can't open buffer file located at \"" + mGLTFData->basePath + path + "\"");
			}
		}
		else {
			return Result(false, "Missing buffer properties");
		}
	}


	Result GLTFImporter::parseBufferView(const nlohmann::json& jsonBufferView)
	{
		auto itBuffer = jsonBufferView.find("buffer");
		auto itByteLength = jsonBufferView.find("byteLength");
		auto itByteOffset = jsonBufferView.find("byteOffset");
		auto itByteStride = jsonBufferView.find("byteStride");
		auto itTarget = jsonBufferView.find("target");
		if ((itBuffer != jsonBufferView.end()) && (itByteLength != jsonBufferView.end())) {
			std::size_t bufferId = *itBuffer;
			std::size_t byteLength = *itByteLength;
			std::size_t byteOffset = (itByteOffset != jsonBufferView.end())? itByteOffset->get<std::size_t>() : 0;
			std::size_t byteStride = (itByteStride != jsonBufferView.end())? itByteStride->get<std::size_t>() : 0;

			BufferView::Target target = BufferView::Target::Undefined;
			if (itTarget != jsonBufferView.end()) {
				int targetId = *itTarget;
				if (targetId == 34962) {
					target = BufferView::Target::Array;
				}
				else if (targetId == 34963) {
					target = BufferView::Target::ElementArray;
				}
				else {
					return Result(false, "Invalid BufferView target " + std::to_string(targetId));
				}
			}

			if (bufferId >= mGLTFData->buffers.size()) {
				return Result(false, "Buffer index " + std::to_string(bufferId) + " out of range");
			}

			mGLTFData->bufferViews.push_back({ bufferId, byteLength, byteOffset, byteStride, target });
			return Result();
		}
		else {
			return Result(false, "Missing BufferView properties");
		}
	}


	Result GLTFImporter::parseAccessor(const nlohmann::json& jsonAccessor)
	{
		auto itBufferView = jsonAccessor.find("bufferView");
		auto itByteOffset = jsonAccessor.find("byteOffset");
		auto itComponentType = jsonAccessor.find("componentType");
		auto itNormalized = jsonAccessor.find("normalized");
		auto itCount = jsonAccessor.find("count");
		auto itType = jsonAccessor.find("type");
		auto itMin = jsonAccessor.find("min");
		auto itMax = jsonAccessor.find("max");
		if ((itBufferView != jsonAccessor.end()) && (itComponentType != jsonAccessor.end())
			&& (itCount != jsonAccessor.end()) && (itType != jsonAccessor.end())
		) {
			std::size_t bufferViewId	= *itBufferView;
			std::size_t byteOffset		= (itByteOffset != jsonAccessor.end())? itByteOffset->get<std::size_t>() : 0;
			bool normalized				= (itNormalized != jsonAccessor.end())? itNormalized->get<bool>() : false;
			std::size_t count			= *itCount;

			graphics::TypeId typeId;
			std::size_t typeSize;
			if (!toTypeId(*itComponentType, typeId) || !toTypeSize(*itComponentType, typeSize)) {
				return Result(false, "Invalid component type" + itComponentType->get<std::string>());
			}

			std::size_t numComponents = 0;
			if (!toNumComponents(*itType, numComponents)) {
				return Result(false, "Invalid component size" + itComponentType->get<std::string>());
			}

			if (bufferViewId >= mGLTFData->bufferViews.size()) {
				return Result(false, "BufferView index " + std::to_string(bufferViewId) + " out of range");
			}

			Accessor::BoundsType minimum;
			if (itMin != jsonAccessor.end()) {
				for (auto componentMin : *itMin) {
					auto& componentMinDst = minimum.emplace_back();
					if (typeId == graphics::TypeId::Float) {
						componentMinDst.f = componentMin.get<float>();
					}
					else {
						componentMinDst.i = componentMin.get<int>();
					}
				}
			}

			Accessor::BoundsType maximum;
			if (itMax != jsonAccessor.end()) {
				for (auto componentMax : *itMax) {
					auto& componentMaxDst = maximum.emplace_back();
					if (typeId == graphics::TypeId::Float) {
						componentMaxDst.f = componentMax.get<float>();
					}
					else {
						componentMaxDst.i = componentMax.get<int>();
					}
				}
			}

			mGLTFData->accessors.push_back({ bufferViewId, byteOffset, count, numComponents, typeId, normalized, minimum, maximum });
			return Result();
		}
		else {
			return Result(false, "Missing accessor properties");
		}
	}


	Result GLTFImporter::parseSampler(const nlohmann::json& jsonSampler)
	{
		Sampler sampler;

		// Filters
		auto doFilter = [&](int filter, int idx) {
			if (!toTextureFilter(filter, sampler.filters[idx])) {
				return Result(false, "Invalid filter " + std::to_string(filter));
			}
			return Result();
		};

		auto itMinFilter = jsonSampler.find("minFilter");
		auto itMagFilter = jsonSampler.find("magFilter");
		int minFilter = (itMinFilter != jsonSampler.end())? itMinFilter->get<int>() : 9728;
		int magFilter = (itMagFilter != jsonSampler.end())? itMagFilter->get<int>() : 9728;
		doFilter(minFilter, 0);
		doFilter(magFilter, 1);

		// Wraps
		auto doWrap = [&](int wrap, int idx) {
			if (!toTextureWrap(wrap, sampler.wraps[idx])) {
				return Result(false, "Invalid wrap mode " + std::to_string(wrap));
			}
			return Result();
		};

		auto itWrapS = jsonSampler.find("wrapS");
		auto itWrapT = jsonSampler.find("wrapT");
		int wrapS = (itWrapS != jsonSampler.end())? itWrapS->get<int>() : 10497;
		int wrapT = (itWrapT != jsonSampler.end())? itWrapS->get<int>() : 10497;
		Result result;
		if (!(result = doWrap(wrapS, 0))) {
			return result;
		}
		if (!(result = doWrap(wrapT, 1))) {
			return result;
		}

		mGLTFData->samplers.push_back(sampler);
		return result;
	}


	Result GLTFImporter::parseImage(const nlohmann::json& jsonImage)
	{
		auto itUri = jsonImage.find("uri");
		if (itUri != jsonImage.end()) {
			std::string path = mGLTFData->basePath + itUri->get<std::string>();

			Image<unsigned char>& image = mGLTFData->images.emplace_back();
			Result result = ImageReader::read(path.c_str(), image);
			if (result) {
				return Result();
			}
			else {
				return Result(false, "Error while reading the image \"" + path + "\": " + result.description());
			}
		}
		else {
			return Result(false, "Missing uri property");
		}
	}


	Result GLTFImporter::parseTexture(const nlohmann::json& jsonTexture)
	{
		auto texture = std::make_shared<graphics::Texture>(graphics::TextureTarget::Texture2D);
		if (!texture) {
			return Result(false, "Failed to create the texture");
		}

		std::string name = mGLTFData->fileName + "_texture" + std::to_string(mGLTFData->textures.size());
		auto itName = jsonTexture.find("name");
		if (itName != jsonTexture.end()) {
			name = mGLTFData->fileName + "_" + itName->get<std::string>();
		}

		auto itSource = jsonTexture.find("source");
		if (itSource != jsonTexture.end()) {
			std::size_t sourceId = *itSource;
			if (sourceId >= mGLTFData->images.size()) {
				return Result(false, "Source index " + std::to_string(sourceId) + " out of range");
			}

			const Image<unsigned char>& image = mGLTFData->images[sourceId];

			graphics::ColorFormat format = graphics::ColorFormat::RGB;
			switch (image.channels) {
				case 1:	format = graphics::ColorFormat::R;		break;
				case 2:	format = graphics::ColorFormat::RG;		break;
				case 3:	format = graphics::ColorFormat::RGB;	break;
				case 4:	format = graphics::ColorFormat::RGBA;	break;
			}

			texture->setImage(image.pixels.get(), se::graphics::TypeId::UnsignedByte, format, format, image.width, image.height);
		}

		auto itSampler = jsonTexture.find("sampler");
		if (itSampler != jsonTexture.end()) {
			std::size_t samplerId = *itSampler;
			if (samplerId >= mGLTFData->samplers.size()) {
				return Result(false, "Sampler index " + std::to_string(samplerId) + " out of range");
			}

			const Sampler& sampler = mGLTFData->samplers[samplerId];
			if ((sampler.filters[0] == graphics::TextureFilter::NearestMipMapNearest)
				|| (sampler.filters[1] == graphics::TextureFilter::NearestMipMapNearest)
				|| (sampler.filters[0] == graphics::TextureFilter::LinearMipMapNearest)
				|| (sampler.filters[1] == graphics::TextureFilter::LinearMipMapNearest)
				|| (sampler.filters[0] == graphics::TextureFilter::NearestMipMapLinear)
				|| (sampler.filters[1] == graphics::TextureFilter::NearestMipMapLinear)
				|| (sampler.filters[0] == graphics::TextureFilter::LinearMipMapLinear)
				|| (sampler.filters[1] == graphics::TextureFilter::LinearMipMapLinear)
			) {
				texture->generateMipMap();
			}

			texture->setFiltering(sampler.filters[0], sampler.filters[1]);
			texture->setWrapping(sampler.wraps[0], sampler.wraps[1]);
		}

		auto textureRef = mGLTFData->scene.repository.insert(std::move(texture), name.c_str());
		if (!textureRef) {
			return Result(false, "Can't add Texture with name " + name);
		}

		mGLTFData->textures.emplace_back(std::move(textureRef));

		return Result();
	}


	Result GLTFImporter::parseMaterial(const nlohmann::json& jsonMaterial)
	{
		Material material;

		std::string name = mGLTFData->fileName + "_material" + std::to_string(mGLTFData->materials.size());
		auto itName = jsonMaterial.find("name");
		if (itName != jsonMaterial.end()) {
			name = mGLTFData->fileName + "_" + itName->get<std::string>();
		}

		material.pbrMetallicRoughness.baseColorFactor = glm::vec4(1.0f);
		material.pbrMetallicRoughness.metallicFactor = 1.0f;
		material.pbrMetallicRoughness.roughnessFactor = 1.0f;
		auto itPBRMetallicRoughness = jsonMaterial.find("pbrMetallicRoughness");
		if (itPBRMetallicRoughness != jsonMaterial.end()) {
			auto itBaseColorFactor = itPBRMetallicRoughness->find("baseColorFactor");
			if (itBaseColorFactor != itPBRMetallicRoughness->end()) {
				toVec(*itBaseColorFactor, material.pbrMetallicRoughness.baseColorFactor);
			}

			auto itBaseColorTexture = itPBRMetallicRoughness->find("baseColorTexture");
			if (itBaseColorTexture != itPBRMetallicRoughness->end()) {
				auto itIndex = itBaseColorTexture->find("index");
				if (itIndex == itBaseColorTexture->end()) {
					return Result(false, "Base color texture missing index property");
				}

				std::size_t index = *itIndex;
				if (index >= mGLTFData->textures.size()) {
					return Result(false, "Base color texture index " + std::to_string(index) + " out of range");
				}

				material.pbrMetallicRoughness.baseColorTexture = mGLTFData->textures[index];
			}

			auto itMetallicFactor = itPBRMetallicRoughness->find("metallicFactor");
			if (itMetallicFactor != itPBRMetallicRoughness->end()) {
				material.pbrMetallicRoughness.metallicFactor = *itMetallicFactor;
			}

			auto itRoughnessFactor = itPBRMetallicRoughness->find("roughnessFactor");
			if (itRoughnessFactor != itPBRMetallicRoughness->end()) {
				material.pbrMetallicRoughness.roughnessFactor = *itRoughnessFactor;
			}

			auto itMetallicRoughnessTexture = itPBRMetallicRoughness->find("metallicRoughnessTexture");
			if (itMetallicRoughnessTexture != itPBRMetallicRoughness->end()) {
				auto itIndex = itMetallicRoughnessTexture->find("index");
				if (itIndex == itMetallicRoughnessTexture->end()) {
					return Result(false, "Metallic roughness texture missing index property");
				}

				std::size_t index = *itIndex;
				if (index >= mGLTFData->textures.size()) {
					return Result(false, "Metallic roughness texture index " + std::to_string(index) + " out of range");
				}

				material.pbrMetallicRoughness.metallicRoughnessTexture = mGLTFData->textures[index];
			}
		}

		material.normalScale = 1.0f;
		auto itNormalTexture = jsonMaterial.find("normalTexture");
		if (itNormalTexture != jsonMaterial.end()) {
			auto itIndex = itNormalTexture->find("index");
			if (itIndex == itNormalTexture->end()) {
				return Result(false, "Normal texture missing index property");
			}

			std::size_t index = *itIndex;
			if (index >= mGLTFData->textures.size()) {
				return Result(false, "Normal texture index " + std::to_string(index) + " out of range");
			}

			material.normalTexture = mGLTFData->textures[index];

			auto itScale = itNormalTexture->find("strength");
			if (itScale != itNormalTexture->end()) {
				material.normalScale = *itScale;
			}
		}

		material.occlusionStrength = 1.0f;
		auto itOcclusionTexture = jsonMaterial.find("occlusionTexture");
		if (itOcclusionTexture != jsonMaterial.end()) {
			auto itIndex = itOcclusionTexture->find("index");
			if (itIndex == itOcclusionTexture->end()) {
				return Result(false, "Occlusion texture missing index property");
			}

			std::size_t index = *itIndex;
			if (index >= mGLTFData->textures.size()) {
				return Result(false, "Occlusion texture index " + std::to_string(index) + " out of range");
			}

			material.occlusionTexture = mGLTFData->textures[index];

			auto itStrength = itOcclusionTexture->find("strength");
			if (itStrength != itOcclusionTexture->end()) {
				material.occlusionStrength = *itStrength;
			}
		}

		auto itEmissiveTexture = jsonMaterial.find("emissiveTexture");
		if (itEmissiveTexture != jsonMaterial.end()) {
			auto itIndex = itEmissiveTexture->find("index");
			if (itIndex == itEmissiveTexture->end()) {
				return Result(false, "Emissive texture missing index property");
			}

			std::size_t index = *itIndex;
			if (index >= mGLTFData->textures.size()) {
				return Result(false, "Emissive texture index " + std::to_string(index) + " out of range");
			}

			material.emissiveTexture = mGLTFData->textures[index];
		}

		material.emissiveFactor = glm::vec3(0.0f);
		auto itEmissiveFactor = jsonMaterial.find("emissiveFactor");
		if (itEmissiveFactor != jsonMaterial.end()) {
			toVec(*itEmissiveFactor, material.emissiveFactor);
		}

		material.alphaMode = graphics::AlphaMode::Opaque;
		auto itAlphaMode = jsonMaterial.find("alphaMode");
		if (itAlphaMode != jsonMaterial.end()) {
			if (!toAlphaMode(*itAlphaMode, material.alphaMode)) {
				return Result(false, "Invalid AlphaMode " + itAlphaMode->get<std::string>());
			}
		}

		material.alphaCutoff = 0.5f;
		auto itAlphaCutoff = jsonMaterial.find("alphaCutoff");
		if (itAlphaCutoff != jsonMaterial.end()) {
			material.alphaCutoff = *itAlphaCutoff;
		}

		material.doubleSided = false;
		auto itDoubleSided = jsonMaterial.find("doubleSided");
		if (itDoubleSided != jsonMaterial.end()) {
			material.doubleSided = *itDoubleSided;
		}

		MaterialShader materialShader = { std::move(name), std::move(material), nullptr, nullptr };
		mGLTFData->materials.emplace_back(std::move(materialShader));
		return Result();
	}


	Result GLTFImporter::parsePrimitive(const nlohmann::json& jsonPrimitive, PrimitiveData& out)
	{
		PrimitiveMeshData primitiveMesh;

		// Read the PrimitiveMeshData object from the json
		auto itAttributes = jsonPrimitive.find("attributes");
		if (itAttributes != jsonPrimitive.end()) {
			for (auto itAttribute = itAttributes->begin(); itAttribute != itAttributes->end(); ++itAttribute) {
				unsigned int meshAttribute;
				if (toMeshAttribute(itAttribute.key(), meshAttribute)) {
					out.hasSkin |= (meshAttribute == MeshAttributes::JointIndexAttribute);
					out.hasSkin |= (meshAttribute == MeshAttributes::JointWeightAttribute);

					primitiveMesh.hasAttribute[meshAttribute] = true;
					primitiveMesh.attributeAccessor[meshAttribute] = *itAttribute;
				}
				else {
					return Result(false, "Invalid attribute \"" + itAttribute.key() + "\"");
				}
			}
		}
		else {
			return Result(false, "Missing attributes property");
		}

		auto itIndices = jsonPrimitive.find("indices");
		if (itIndices != jsonPrimitive.end()) {
			primitiveMesh.indicesIndex = *itIndices;
		}
		else {
			return Result(false, "Meshes without indices aren't supported");
		}

		// Create a Mesh from the PrimitiveMeshData object
		Result result = createMesh(primitiveMesh, out.mesh);
		if (!result) {
			return Result(false, std::string("Couldn't create the Mesh: ") + result.description());
		}

		auto itMaterial = jsonPrimitive.find("material");
		if (itMaterial != jsonPrimitive.end()) {
			std::size_t materialId = *itMaterial;
			if (materialId >= mGLTFData->materials.size()) {
				return Result(false, "Material index " + std::to_string(materialId) + " out of range");
			}

			auto& materialShader = mGLTFData->materials[materialId];
			if (out.hasSkin) {
				if (!materialShader.shaderSkin) {
					std::string materialName = materialShader.name + "_skin";
					materialShader.shaderSkin = mShaderBuilder.createShader(materialName.c_str(), materialShader.material, true);
					if (!materialShader.shaderSkin) {
						return Result(false, "Can't add MaterialShader skin with name " + materialName);
					}
				}
				out.shader = materialShader.shaderSkin;
			}
			else {
				if (!materialShader.shader) {
					materialShader.shader = mShaderBuilder.createShader(materialShader.name.c_str(), materialShader.material, false);
					if (!materialShader.shader) {
						return Result(false, "Can't add MaterialShader with name " + materialShader.name);
					}
				}
				out.shader = materialShader.shader;
			}
		}
		else {
			// Use the default shader
			const char* shaderKey = out.hasSkin? "shaderDefaultSkin" : "shaderDefault";
			out.shader = mGLTFData->scene.repository.findByName<RenderableShader>(shaderKey);
		}

		auto itMode = jsonPrimitive.find("mode");
		if (itMode != jsonPrimitive.end()) {
			if (!toPrimitiveType(*itMode, out.primitiveType)) {
				return Result(false, "Invalid primitive type " + std::to_string(itMode->get<int>()));
			}
		}

		return Result();
	}


	Result GLTFImporter::parseMesh(const nlohmann::json& jsonMesh)
	{
		auto itPrimitives = jsonMesh.find("primitives");
		if (itPrimitives == jsonMesh.end()) {
			return Result(false, "Missing primitives property");
		}
		if (itPrimitives->empty()) {
			return Result(false, "A mesh must containt at least one primitive");
		}
		if (itPrimitives->size() > MeshComponent::kMaxMeshes) {
			return Result(false, "The number of primitives (" + std::to_string(itPrimitives->size()) + ") exceeds the maximum allowed"
				+ " (" + std::to_string(MeshComponent::kMaxMeshes) + ")");
		}

		auto& primitives = mGLTFData->primitives.emplace_back();
		primitives.reserve(itPrimitives->size());
		for (std::size_t primitiveId = 0; primitiveId < itPrimitives->size(); ++primitiveId) {
			PrimitiveData primitive;
			Result result = parsePrimitive((*itPrimitives)[primitiveId], primitive);
			if (!result) {
				return Result(false, "Failed to read the primitives property at primitive " + std::to_string(primitiveId) + ": " + result.description());
			}

			primitives.emplace_back(std::move(primitive));
		}

		return Result();
	}


	Result GLTFImporter::parseSkin(const nlohmann::json& jsonSkin)
	{
		auto skin = std::make_shared<Skin>();
		IndexVector jointIndices;

		std::string name = mGLTFData->fileName + "_skin" + std::to_string(mGLTFData->skins.size());
		auto itName = jsonSkin.find("name");
		if (itName != jsonSkin.end()) {
			name = mGLTFData->fileName + "_" + itName->get<std::string>();
		}

		auto itInverseBindMatrices = jsonSkin.find("inverseBindMatrices");
		if (itInverseBindMatrices != jsonSkin.end()) {
			std::size_t accessorId = *itInverseBindMatrices;
			if (accessorId >= mGLTFData->accessors.size()) {
				return Result(false, "Accessor index " + std::to_string(accessorId) + " out of range");
			}

			const Accessor& a = mGLTFData->accessors[accessorId];
			const BufferView& bv = mGLTFData->bufferViews[a.bufferViewId];
			const Buffer& b = mGLTFData->buffers[bv.bufferId];

			if (a.count > Skin::kMaxJoints) {
				return Result(false, "The number of joints (" + std::to_string(a.count) + ") exceeds the maximum allowed"
					+ " (" + std::to_string(Skin::kMaxJoints) + ")");
			}

			const glm::mat4* mat4Ptr = reinterpret_cast<const glm::mat4*>(b.data() + bv.offset + a.byteOffset);
			skin->inverseBindMatrices = utils::FixedVector<glm::mat4, Skin::kMaxJoints>(mat4Ptr, mat4Ptr + a.count);
		}

		auto itJoints = jsonSkin.find("joints");
		if (itJoints != jsonSkin.end()) {
			jointIndices = itJoints->get<IndexVector>();
		}
		else {
			return Result(false, "A skin must have a joints property");
		}

		auto skinRef = mGLTFData->scene.repository.insert(std::move(skin), name.c_str());
		if (!skinRef) {
			return Result(false, "Can't add Skin with name " + name);
		}

		mGLTFData->skins.emplace_back(std::move(skinRef));
		mGLTFData->jointIndices.emplace_back(std::move(jointIndices));

		return Result();
	}


	Result GLTFImporter::parseCamera(const nlohmann::json& jsonCamera)
	{
		auto itType = jsonCamera.find("type");
		auto itPerspective = jsonCamera.find("perspective");
		auto itOrthographic = jsonCamera.find("orthographic");

		if (itType != jsonCamera.end()) {
			if ((*itType == "perspective") && (itPerspective != jsonCamera.end())) {
				auto itAspectRatio = itPerspective->find("aspectRatio");
				auto itYFov = itPerspective->find("yfov");
				auto itZFar = itPerspective->find("zfar");
				auto itZNear = itPerspective->find("znear");
				if ((itAspectRatio != jsonCamera.end()) && (itYFov != jsonCamera.end())
					&& (itZFar != jsonCamera.end()) && (itZNear != jsonCamera.end())
				) {
					auto& camera = mGLTFData->cameraComponents.emplace_back();
					camera.setPerspectiveProjection(*itYFov, *itAspectRatio, *itZNear, *itZFar);
					return Result();
				}
				else {
					return Result(false, "Missing perspective properties");
				}
			}
			else if ((*itType == "orthographic") && (itOrthographic != jsonCamera.end())) {
				auto itXMag = itOrthographic->find("xmag");
				auto itYMag = itOrthographic->find("ymag");
				auto itZFar = itOrthographic->find("zfar");
				auto itZNear = itOrthographic->find("znear");
				if ((itXMag != jsonCamera.end()) && (itYMag != jsonCamera.end())
					&& (itZFar != jsonCamera.end()) && (itZNear != jsonCamera.end())
				) {
					auto& camera = mGLTFData->cameraComponents.emplace_back();
					float halfWidth = *itXMag, halfHeight = *itYMag;
					camera.setOrthographicProjection(-halfWidth, halfWidth, -halfHeight, halfHeight, *itZNear, *itZFar);
					return Result();
				}
				else {
					return Result(false, "Missing orthographic properties");
				}
			}
			else {
				return Result(false, "Invalid type property \"" + itType->get<std::string>() + "\"");
			}
		}
		else {
			return Result(false, "Missing type property");
		}
	}


	Result GLTFImporter::parseAnimationSampler(
		const nlohmann::json& jsonSampler,
		std::unique_ptr<Vec3Animation>& out1, std::unique_ptr<QuatAnimation>& out2
	) const
	{
		InterpolationType interpolation = InterpolationType::Linear;
		auto itInterpolation = jsonSampler.find("interpolation");
		if (itInterpolation != jsonSampler.end()) {
			if (!toInterpolationType(*itInterpolation, interpolation)) {
				return Result(false, "Invalid interpolation " + itInterpolation->get<std::string>());
			}
		}

		auto itInput = jsonSampler.find("input");
		if (itInput == jsonSampler.end()) {
			return Result(false, "Missing input property");
		}

		std::size_t inputId = *itInput;
		if (inputId >= mGLTFData->accessors.size()) {
			return Result(false, "Input property out of bounds");
		}

		auto itOutput = jsonSampler.find("output");
		if (itOutput == jsonSampler.end()) {
			return Result(false, "Missing output property");
		}

		std::size_t outputId = *itOutput;
		if (outputId >= mGLTFData->accessors.size()) {
			return Result(false, "Output property out of bounds");
		}

		const Accessor& aInput = mGLTFData->accessors[inputId];
		const BufferView& bvInput = mGLTFData->bufferViews[aInput.bufferViewId];
		const Buffer& bInput = mGLTFData->buffers[bvInput.bufferId];
		const Accessor& aOutput = mGLTFData->accessors[outputId];
		const BufferView& bvOutput = mGLTFData->bufferViews[aOutput.bufferViewId];
		const Buffer& bOutput = mGLTFData->buffers[bvOutput.bufferId];

		if (aInput.componentTypeId != graphics::TypeId::Float) {
			return Result(false, "Input componentType must be FLOAT");
		}

		if (aOutput.componentTypeId != graphics::TypeId::Float) {
			return Result(false, "Output componentType must be FLOAT");
		}

		std::size_t numElements = aOutput.count;
		const float* inputPtr = reinterpret_cast<const float*>(bInput.data() + bvInput.offset + aInput.byteOffset);
		const float* outputPtr = reinterpret_cast<const float*>(bOutput.data() + bvOutput.offset + aOutput.byteOffset);

		switch (interpolation) {
			case InterpolationType::Linear:
				if (aInput.count != aOutput.count) {
					return Result(false, "Input number of elements doesn't match the output one");
				}
				else if ((aInput.numComponents == 1) && (aOutput.numComponents == 3)) {
					auto animVec3 = std::make_unique<animation::AnimationVec3Linear>();
					for (std::size_t i = 0; i < numElements; ++i) {
						animVec3->addKeyFrame({ *reinterpret_cast<const glm::vec3*>(outputPtr + i * aOutput.numComponents), inputPtr[i] });
					}
					out1 = std::move(animVec3);
					return Result();
				}
				else if ((aInput.numComponents == 1) && (aOutput.numComponents == 4)) {
					auto animQuat = std::make_unique<animation::AnimationQuatLinear>();
					for (std::size_t i = 0; i < numElements; ++i) {
						animQuat->addKeyFrame({ *reinterpret_cast<const glm::quat*>(outputPtr + i * aOutput.numComponents), inputPtr[i] });
					}
					out2 = std::move(animQuat);
					return Result();
				}
				else {
					return Result(false, "Invalid accessor component sizes");
				}
				break;
			case InterpolationType::Step:
				if (aInput.count != aOutput.count) {
					return Result(false, "Input number of elements doesn't match the output one");
				}
				else if ((aInput.numComponents == 1) && (aOutput.numComponents == 3)) {
					auto animVec3 = std::make_unique<animation::AnimationVec3Step>();
					for (std::size_t i = 0; i < numElements; ++i) {
						animVec3->addKeyFrame({ *reinterpret_cast<const glm::vec3*>(outputPtr + i * aOutput.numComponents), inputPtr[i] });
					}
					out1 = std::move(animVec3);
					return Result();
				}
				else if ((aInput.numComponents == 1) && (aOutput.numComponents == 4)) {
					auto animQuat = std::make_unique<animation::AnimationQuatStep>();
					for (std::size_t i = 0; i < numElements; ++i) {
						animQuat->addKeyFrame({ *reinterpret_cast<const glm::quat*>(outputPtr + i * aOutput.numComponents), inputPtr[i] });
					}
					out2 = std::move(animQuat);
					return Result();
				}
				else {
					return Result(false, "Invalid accessor component sizes");
				}
				break;
			case InterpolationType::CubicSpline:
				if (3 * aInput.count != aOutput.count) {
					return Result(false, "Input number of elements doesn't match the output one");
				}
				else if ((aInput.numComponents == 1) && (aOutput.numComponents == 3)) {
					auto animVec3 = std::make_unique<animation::AnimationVec3CubicSpline>();
					for (std::size_t i = 0; i < numElements; ++i) {
						animVec3->addKeyFrame({
							*reinterpret_cast<const glm::vec3*>(outputPtr + i * aOutput.numComponents),
							*reinterpret_cast<const glm::vec3*>(outputPtr + numElements * aOutput.numComponents + i * aOutput.numComponents),
							*reinterpret_cast<const glm::vec3*>(outputPtr + 2 * numElements * aOutput.numComponents + i * aOutput.numComponents),
							inputPtr[i] });
					}
					out1 = std::move(animVec3);
					return Result();
				}
				else if ((aInput.numComponents == 1) && (aOutput.numComponents == 4)) {
					auto animQuat = std::make_unique<animation::AnimationQuatCubicSpline>();
					for (std::size_t i = 0; i < numElements; ++i) {
						animQuat->addKeyFrame({
							*reinterpret_cast<const glm::quat*>(outputPtr + i * aOutput.numComponents),
							*reinterpret_cast<const glm::quat*>(outputPtr + numElements * aOutput.numComponents + i * aOutput.numComponents),
							*reinterpret_cast<const glm::quat*>(outputPtr + 2 * numElements * aOutput.numComponents + i * aOutput.numComponents),
							inputPtr[i] });
					}
					out2 = std::move(animQuat);
					return Result();
				}
				else {
					return Result(false, "Invalid accessor component sizes");
				}
				break;
		}

		return Result(false, "You shouldn't be here");
	}


	Result GLTFImporter::parseAnimationChannel(
		const nlohmann::json& jsonChannel,
		const std::unordered_map<std::size_t, Vec3AnimationSPtr>& vec3Animations,
		const std::unordered_map<std::size_t, QuatAnimationSPtr>& quatAnimations,
		animation::SkeletonAnimator& sAnimator
	) const
	{
		auto itSampler = jsonChannel.find("sampler");
		auto itTarget = jsonChannel.find("target");
		if ((itSampler != jsonChannel.end()) && (itTarget != jsonChannel.end())) {
			auto itNode = itTarget->find("node");
			auto itPath = itTarget->find("path");
			if ((itNode != itTarget->end()) && (itPath != itTarget->end())) {
				auto transformationType = animation::TransformationAnimator::TransformationType::Translation;
				if (!toTransformationType(*itPath, transformationType)) {
					return Result(false, "Invalid path " + itPath->get<std::string>());
				}

				std::size_t nodeId = *itNode;
				if (nodeId >= mGLTFData->nodes.size()) {
					return Result(false, "Node index " + std::to_string(nodeId) + " out of range");
				}

				std::size_t samplerId = *itSampler;
				std::unique_ptr<animation::TransformationAnimator> tAnimator = nullptr;
				auto itVec3Animation = vec3Animations.find(samplerId);
				auto itQuatAnimation = quatAnimations.find(samplerId);
				if (itVec3Animation != vec3Animations.end()) {
					tAnimator = std::make_unique<animation::Vec3Animator>(itVec3Animation->second);
				}
				else if (itQuatAnimation != quatAnimations.end()) {
					tAnimator = std::make_unique<animation::QuatAnimator>(itQuatAnimation->second);
				}
				else {
					return Result(false, "Sampler index " + std::to_string(samplerId) + " out of range");
				}

				if (sAnimator.getLoopTime() < tAnimator->getLoopTime()) {
					sAnimator.setLoopTime(tAnimator->getLoopTime());
				}
				sAnimator.addAnimator(mGLTFData->nodes[nodeId].nodeData.name.data(), transformationType, std::move(tAnimator));
				return Result();
			}
			else {
				return Result(false, "Missing target properties");
			}
		}
		else {
			return Result(false, "Missing channel properties");
		}
	}


	Result GLTFImporter::parseAnimation(const nlohmann::json& jsonAnimation)
	{
		std::string name = mGLTFData->fileName + "_animator" + std::to_string(mGLTFData->skeletonAnimators.size());
		auto itName = jsonAnimation.find("name");
		if (itName != jsonAnimation.end()) {
			name = mGLTFData->fileName + "_" + itName->get<std::string>();
		}

		auto itSamplers = jsonAnimation.find("samplers");
		if (itSamplers == jsonAnimation.end()) {
			return Result(false, "Missing \"samplers\" property");
		}

		auto itChannels = jsonAnimation.find("channels");
		if (itChannels == jsonAnimation.end()) {
			return Result(false, "Missing \"channels\" property");
		}

		std::unordered_map<std::size_t, Vec3AnimationSPtr> vec3Animations;
		std::unordered_map<std::size_t, QuatAnimationSPtr> quatAnimations;
		vec3Animations.reserve(itSamplers->size());
		quatAnimations.reserve(itSamplers->size());
		for (std::size_t samplerId = 0; samplerId < itSamplers->size(); ++samplerId) {
			std::unique_ptr<Vec3Animation> out1;
			std::unique_ptr<QuatAnimation> out2;
			Result result = parseAnimationSampler((*itSamplers)[samplerId], out1, out2);
			if (!result) {
				return Result(false, "Failed to read the samplers property at sampler " + std::to_string(samplerId) + ": " + result.description());
			}

			if (out1) {
				vec3Animations.emplace(samplerId, std::move(out1));
			}
			if (out2) {
				quatAnimations.emplace(samplerId, std::move(out2));
			}
		}

		auto skeletonAnimator = std::make_shared<animation::SkeletonAnimator>();

		for (std::size_t channelId = 0; channelId < itChannels->size(); ++channelId) {
			IAnimatorUPtr out;
			Result result = parseAnimationChannel((*itChannels)[channelId], vec3Animations, quatAnimations, *skeletonAnimator);
			if (!result) {
				return Result(false, "Failed to read the samplers property at channel " + std::to_string(channelId) + ": " + result.description());
			}
		}

		auto skeletonAnimatorRef = mGLTFData->scene.repository.insert(std::move(skeletonAnimator), name.c_str());
		if (!skeletonAnimatorRef) {
			return Result(false, "Can't add SkeletonAnimator with name " + name);
		}

		mGLTFData->skeletonAnimators.emplace_back(std::move(skeletonAnimatorRef));
		mGLTFData->scene.application.getExternalTools().animationEngine->addAnimator(skeletonAnimatorRef.get().get());

		return Result();
	}


	Result GLTFImporter::parseKHRLights(const nlohmann::json& jsonKHRLights)
	{
		if (auto itLights = jsonKHRLights.find("lights"); itLights != jsonKHRLights.end()) {
			mGLTFData->lightSources.reserve(itLights->size());
			for (std::size_t lightId = 0; lightId < itLights->size(); ++lightId) {
				Result result = parseLight( (*itLights)[lightId] );
				if (!result) {
					return Result(false, "Failed to read the lights property at light " + std::to_string(lightId) + ": " + result.description());
				}
			}
		}
		else {
			return Result(false, "Missing \"lights\" property");
		}

		return Result();
	}


	Result GLTFImporter::parseLight(const nlohmann::json& jsonLight)
	{
		std::shared_ptr<LightSource> lightSource;

		std::string name = mGLTFData->fileName + "_light" + std::to_string(mGLTFData->lightSources.size());
		auto itName = jsonLight.find("name");
		if (itName != jsonLight.end()) {
			name = mGLTFData->fileName + "_" + itName->get<std::string>();
		}

		auto itType = jsonLight.find("type");
		if (itType != jsonLight.end()) {
			if (*itType == "directional") {
				lightSource = std::make_shared<LightSource>(mGLTFData->scene.application.getEventManager(), LightSource::Type::Directional);
			}
			else if (*itType == "point") {
				lightSource = std::make_shared<LightSource>(mGLTFData->scene.application.getEventManager(), LightSource::Type::Point);

				auto itRange = jsonLight.find("range");
				lightSource->setRange((itRange != jsonLight.end())? itRange->get<float>() : std::numeric_limits<float>::max());
			}
			else if (*itType == "spot") {
				lightSource = std::make_shared<LightSource>(mGLTFData->scene.application.getEventManager(), LightSource::Type::Spot);

				auto itRange = jsonLight.find("range");
				lightSource->setRange((itRange != jsonLight.end())? itRange->get<float>() : std::numeric_limits<float>::max());

				auto itSpot = jsonLight.find("spot");
				if (itSpot == jsonLight.end()) {
					return Result(false, "A SpotLight must have a spot property");
				}

				auto itInnerConeAngle = itSpot->find("innerConeAngle");
				float innerConeAngle = (itInnerConeAngle != itSpot->end())? itInnerConeAngle->get<float>() : 0.0f;

				auto itOuterConeAngle = itSpot->find("outerConeAngle");
				float outerConeAngle = (itOuterConeAngle != itSpot->end())? itOuterConeAngle->get<float>() : glm::quarter_pi<float>();

				lightSource->setSpotLightRange(innerConeAngle, outerConeAngle);
			}
			else {
				return Result(false, "Invalid type property \"" + itType->get<std::string>() + "\"");
			}
		}
		else {
			return Result(false, "A light must have a type property");
		}

		glm::vec3 color(1.0f);
		auto itColor = jsonLight.find("color");
		if (itColor != jsonLight.end()) {
			toVec(*itColor, color);
		}
		lightSource->setColor(color);

		auto itIntensity = jsonLight.find("intensity");
		lightSource->setIntensity((itIntensity != jsonLight.end())? itIntensity->get<float>() : 1.0f);

		auto lightSourceRef = mGLTFData->scene.repository.insert(std::move(lightSource), name.c_str());
		if (!lightSourceRef) {
			return Result(false, "Can't add LightSource with name " + name);
		}

		mGLTFData->lightSources.emplace_back(std::move(lightSourceRef));

		return Result();
	}


	Result GLTFImporter::parseNode(const nlohmann::json& jsonNode)
	{
		Node node;

		auto itName = jsonNode.find("name");
		if (itName != jsonNode.end()) {
			std::string name = *itName;
			std::size_t length = name.size();
			if (length >= animation::NodeData::kMaxLength) {
				length = animation::NodeData::kMaxLength - 1;
			}
			name.copy(node.nodeData.name.data(), length);
		}

		auto itMatrix = jsonNode.find("matrix");
		if (itMatrix != jsonNode.end()) {
			glm::mat4 matrix;
			if (toMat(*itMatrix, matrix)) {
				utils::decompose(
					matrix,
					node.nodeData.localTransforms.position,
					node.nodeData.localTransforms.orientation,
					node.nodeData.localTransforms.scale
				);
			}
		}
		else {
			auto itRotation = jsonNode.find("rotation");
			if (itRotation != jsonNode.end()) {
				toQuat(*itRotation, node.nodeData.localTransforms.orientation);
			}

			auto itScale = jsonNode.find("scale");
			if (itScale != jsonNode.end()) {
				toVec(*itScale, node.nodeData.localTransforms.scale);
			}

			auto itTranslation = jsonNode.find("translation");
			if (itTranslation != jsonNode.end()) {
				toVec(*itTranslation, node.nodeData.localTransforms.position);
			}
		}

		auto itChildren = jsonNode.find("children");
		if (itChildren != jsonNode.end()) {
			node.children = itChildren->get<IndexVector>();
		}

		auto itCamera = jsonNode.find("camera");
		auto itMesh = jsonNode.find("mesh");
		auto itSkin = jsonNode.find("skin");
		auto itExtensions = jsonNode.find("extensions");
		if ((itCamera != jsonNode.end()) || (itMesh != jsonNode.end())
			|| (itSkin != jsonNode.end()) || (itExtensions != jsonNode.end())
		) {
			EntityDatabase& entityDB = mGLTFData->scene.application.getEntityDatabase();
			EventManager& eventManager = mGLTFData->scene.application.getEventManager();
			node.entity = entityDB.addEntity();
			mGLTFData->scene.entities.push_back(node.entity);

			entityDB.emplaceComponent<TransformsComponent>(node.entity);
			entityDB.emplaceComponent<TagComponent>(node.entity, node.nodeData.name.data());

			if (itCamera != jsonNode.end()) {
				std::size_t cameraIndex = *itCamera;
				if (cameraIndex < mGLTFData->cameraComponents.size()) {
					entityDB.emplaceComponent<CameraComponent>(node.entity, mGLTFData->cameraComponents[cameraIndex]);
				}
				else {
					return Result(false, "Camera index " + std::to_string(cameraIndex) + " out of range");
				}
			}

			if (itMesh != jsonNode.end()) {
				std::size_t meshIndex = *itMesh;
				if (meshIndex < mGLTFData->primitives.size()) {
					auto mesh = entityDB.emplaceComponent<MeshComponent>(node.entity);
					for (auto& primitive : mGLTFData->primitives[meshIndex]) {
						auto rIndex = mesh->add(primitive.hasSkin, primitive.mesh, primitive.primitiveType);
						mesh->addRenderableShader(rIndex, primitive.shader);
					}
				}
				else {
					return Result(false, "Mesh index " + std::to_string(meshIndex) + " out of range");
				}
			}

			if (itSkin != jsonNode.end()) {
				std::size_t skinIndex = *itSkin;
				if (skinIndex < mGLTFData->skins.size()) {
					node.hasSkin = true;
					node.skinIndex = skinIndex;
				}
				else {
					return Result(false, "Skin index " + std::to_string(skinIndex) + " out of range");
				}
			}

			if (itExtensions != jsonNode.end()) {
				auto itLightsPunctual = itExtensions->find("KHR_lights_punctual");
				if (itLightsPunctual != itExtensions->end()) {
					auto itLight = itLightsPunctual->find("light");
					if (itLight != itLightsPunctual->end()) {
						std::size_t lightIndex = *itLight;
						if (lightIndex < mGLTFData->lightSources.size()) {
							LightComponent lightComponent;
							lightComponent.setSource(mGLTFData->lightSources[lightIndex]);
							entityDB.addComponent(node.entity, std::move(lightComponent));
						}
						else {
							return Result(false, "Light index " + std::to_string(lightIndex) + " out of range");
						}
					}
				}

				auto itActiveCamera = itExtensions->find("active_camera");
				if ((itActiveCamera != itExtensions->end()) && (*itActiveCamera == true)) {
					eventManager.publish(new ContainerEvent<Topic::Camera, Entity>(node.entity));
				}
			}
		}

		mGLTFData->nodes.emplace_back(std::move(node));
		return Result();
	}


	Result GLTFImporter::parseScene(const nlohmann::json& jsonScene)
	{
		EntityDatabase& entityDB = mGLTFData->scene.application.getEntityDatabase();

		auto itNodes = jsonScene.find("nodes");
		if (itNodes != jsonScene.end()) {
			for (std::size_t rootNodeId : itNodes->get< std::vector<std::size_t> >()) {
				if (rootNodeId >= mGLTFData->nodes.size()) {
					return Result(false, "Node index " + std::to_string(rootNodeId) + " out of range while processing the scene");
				}

				// Create the AnimationNode of the root node
				Node& rootNode = mGLTFData->nodes[rootNodeId];
				auto itRootNode = mGLTFData->scene.rootNode.emplace(mGLTFData->scene.rootNode.cend(), rootNode.nodeData);
				if (itRootNode == mGLTFData->scene.rootNode.end()) {
					return Result(false, "Failed to create an AnimationNode for the node " + std::to_string(rootNodeId));
				}

				rootNode.animationNode = &(*itRootNode);
				animation::updateWorldTransforms(*rootNode.animationNode);
				if (rootNode.entity != kNullEntity) {
					auto [transforms] = entityDB.getComponents<TransformsComponent>(rootNode.entity);
					if (transforms) {
						transforms->position = rootNode.animationNode->getData().worldTransforms.position;
						transforms->orientation = rootNode.animationNode->getData().worldTransforms.orientation;
						transforms->scale = rootNode.animationNode->getData().worldTransforms.scale;
					}
					entityDB.emplaceComponent<AnimationComponent>(rootNode.entity, rootNode.animationNode);
				}

				// Build the tree
				std::vector<size_t> nodesToProcess = { rootNodeId };
				while (!nodesToProcess.empty()) {
					std::size_t nodeId = nodesToProcess.back();
					nodesToProcess.pop_back();

					auto& node = mGLTFData->nodes[nodeId];
					for (std::size_t childId : node.children) {
						if (childId >= mGLTFData->nodes.size()) {
							return Result(false, "Node index " + std::to_string(childId)
								+ " out of range while processing the children nodes of " + std::to_string(nodeId));
						}

						// Create the AnimationNode of the child node
						Node& child = mGLTFData->nodes[childId];
						auto itChild = node.animationNode->emplace(node.animationNode->cend(), child.nodeData);
						if (itChild == node.animationNode->end()) {
							return Result(false, "Failed to create an AnimationNode for the node " + std::to_string(childId));
						}

						child.animationNode = &(*itChild);
						animation::updateWorldTransforms(*child.animationNode);
						if (child.entity != kNullEntity) {
							auto [transforms] = entityDB.getComponents<TransformsComponent>(child.entity);
							if (transforms) {
								transforms->position = rootNode.animationNode->getData().worldTransforms.position;
								transforms->orientation = rootNode.animationNode->getData().worldTransforms.orientation;
								transforms->scale = rootNode.animationNode->getData().worldTransforms.scale;
							}
							entityDB.emplaceComponent<AnimationComponent>(child.entity, child.animationNode);
						}

						nodesToProcess.push_back(childId);
					}
				}
			}
		}

		return Result();
	}


	Result GLTFImporter::createMesh(const PrimitiveMeshData& primitiveMesh, MeshRef& out) const
	{
		// Check if the Mesh has already been created
		auto itMesh = mGLTFData->primitiveMeshes.find(primitiveMesh);
		if (itMesh != mGLTFData->primitiveMeshes.end()) {
			out = mGLTFData->meshes[itMesh->second];
			return Result();
		}

		// Create the VAO
		graphics::VertexArray vao;
		vao.bind();

		// Create the VBOs
		std::vector<graphics::VertexBuffer> vbos;
		glm::vec3 minPosition(0.0f), maxPosition(0.0f);
		for (unsigned int i = 0; i < MeshAttributes::NumAttributes; ++i) { if (primitiveMesh.hasAttribute[i]) {
			if (primitiveMesh.attributeAccessor[i] >= mGLTFData->accessors.size()) {
				return Result(false, "Attribute index " + std::to_string(primitiveMesh.attributeAccessor[i]) + " out of range");
			}

			const Accessor& a = mGLTFData->accessors[primitiveMesh.attributeAccessor[i]];
			const BufferView& bv = mGLTFData->bufferViews[a.bufferViewId];
			const Buffer& b = mGLTFData->buffers[bv.bufferId];

			auto& vbo = vbos.emplace_back();
			vbo.resizeAndCopy(b.data() + bv.offset + a.byteOffset, bv.length);

			// Add the VBO to the VAO
			vbo.bind();
			vao.enableAttribute(i);
			if (i == MeshAttributes::JointIndexAttribute) {
				vao.setVertexIntegerAttribute(i, a.componentTypeId, static_cast<int>(a.numComponents), bv.stride);
			}
			else {
				vao.setVertexAttribute(i, a.componentTypeId, a.normalized, static_cast<int>(a.numComponents), bv.stride);
			}

			if (i == MeshAttributes::PositionAttribute) {
				for (int j = 0; j < 3; ++j) {
					minPosition[j] = a.minimum[j].f;
					maxPosition[j] = a.maximum[j].f;
				}
			}
		}}

		// Create the IBO
		if (primitiveMesh.indicesIndex >= mGLTFData->accessors.size()) {
			return Result(false, "Accessor index " + std::to_string(primitiveMesh.indicesIndex) + " out of range");
		}

		const Accessor& a = mGLTFData->accessors[primitiveMesh.indicesIndex];
		const BufferView& bv = mGLTFData->bufferViews[a.bufferViewId];
		const Buffer& b = mGLTFData->buffers[bv.bufferId];

		if ((a.componentTypeId != graphics::TypeId::UnsignedByte)
			&& (a.componentTypeId != graphics::TypeId::UnsignedShort)
			&& (a.componentTypeId != graphics::TypeId::UnsignedInt)
		) {
			return Result(false, "Accessor " + std::to_string(primitiveMesh.indicesIndex) + " must be UByte or UShort or UInt");
		}
		if (a.numComponents != 1) {
			return Result(false, "Accessor " + std::to_string(primitiveMesh.indicesIndex) + " component size must be 1");
		}
		if ((bv.target != BufferView::Target::Undefined)
			&& (bv.target != BufferView::Target::ElementArray)
		) {
			return Result(false, "BufferView " + std::to_string(a.bufferViewId) + " (optional) target must be ElementArray");
		}

		graphics::IndexBuffer ibo;
		ibo.resizeAndCopy(b.data() + bv.offset + a.byteOffset, bv.length, a.componentTypeId, a.count);

		ibo.bind();

		// Create a new Mesh
		auto mesh = std::make_shared<graphics::Mesh>(std::move(vbos), std::move(ibo), std::move(vao));
		mesh->setBounds(minPosition, maxPosition);
		mGLTFData->meshes.emplace_back(out);
		mGLTFData->primitiveMeshes.emplace(primitiveMesh, mGLTFData->meshes.size() - 1);

		std::string name = mGLTFData->fileName + "_mesh" + std::to_string(PrimitiveMeshData::MyHash()(primitiveMesh));
		out = mGLTFData->scene.repository.insert(std::move(mesh), name.c_str());
		if (!out) {
			return Result(false, "Can't add Mesh with name " + name);
		}

		return Result();
	}

}
