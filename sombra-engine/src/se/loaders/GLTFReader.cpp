#include <sstream>
#include <fstream>
#include <nlohmann/json.hpp>
#include "GLTFReader.h"
#include "se/loaders/ImageReader.h"

namespace se::loaders {

	DataHolder GLTFReader::load(const std::string& path)
	{
		DataHolder output;
		mBasePath = path.substr(0, path.find_last_of("/\\") + 1);

		try {
			std::ifstream inputstream(path);
			if (!inputstream.good()) {
				throw std::runtime_error("Can't open the GLTF file");
			}

			nlohmann::json jsonGLTF;
			inputstream >> jsonGLTF;
			if (inputstream.fail()) {
				throw std::runtime_error("Failed to read the GLTF file");
			}

			auto itAsset = jsonGLTF.find("asset");
			if (itAsset == jsonGLTF.end()) {
				throw std::runtime_error("GLTF file must have an asset property");
			}

			if (!checkAssetVersion(*itAsset, FileFormat::sVersion, FileFormat::sRevision)) {
				throw std::runtime_error("Asset version not supported");
			}

			if (auto itBuffers = jsonGLTF.find("buffers"); itBuffers != jsonGLTF.end()) {
				mGLTFData.buffers.reserve(itBuffers->size());
				for (std::size_t bufferId = 0; bufferId < itBuffers->size(); ++bufferId) {
					try {
						parseBuffer( (*itBuffers)[bufferId] );
					}
					catch (const std::exception& e) {
						throw std::runtime_error("Failed to read the buffers property at buffer " + std::to_string(bufferId) + ": " + e.what());
					}
				}
			}

			if (auto itBufferViews = jsonGLTF.find("bufferViews"); itBufferViews != jsonGLTF.end()) {
				mGLTFData.bufferViews.reserve(itBufferViews->size());
				for (std::size_t bufferViewId = 0; bufferViewId < itBufferViews->size(); ++bufferViewId) {
					try {
						parseBufferView( (*itBufferViews)[bufferViewId] );
					}
					catch (const std::exception& e) {
						throw std::runtime_error("Failed to read the bufferViews property at bufferView " + std::to_string(bufferViewId) + ": " + e.what());
					}
				}
			}

			if (auto itAccessors = jsonGLTF.find("accessors"); itAccessors != jsonGLTF.end()) {
				mGLTFData.accessors.reserve(itAccessors->size());
				for (std::size_t accessorId = 0; accessorId < itAccessors->size(); ++accessorId) {
					try {
						parseAccessor( (*itAccessors)[accessorId] );
					}
					catch (const std::exception& e) {
						throw std::runtime_error("Failed to read the accessors property at accessor " + std::to_string(accessorId) + ": " + e.what());
					}
				}
			}

			if (auto itSamplers = jsonGLTF.find("samplers"); itSamplers != jsonGLTF.end()) {
				mGLTFData.samplers.reserve(itSamplers->size());
				for (std::size_t samplerId = 0; samplerId < itSamplers->size(); ++samplerId) {
					try {
						parseSampler( (*itSamplers)[samplerId] );
					}
					catch (const std::exception& e) {
						throw std::runtime_error("Failed to read the samplers property at sampler " + std::to_string(samplerId) + ": " + e.what());
					}
				}
			}

			if (auto itImages = jsonGLTF.find("images"); itImages != jsonGLTF.end()) {
				output.images.reserve(itImages->size());
				for (std::size_t imageId = 0; imageId < itImages->size(); ++imageId) {
					try {
						parseImage((*itImages)[imageId], output);
					}
					catch (const std::exception& e) {
						throw std::runtime_error("Failed to read the images property at image " + std::to_string(imageId) + ": " + e.what());
					}
				}
			}

			if (auto itTextures = jsonGLTF.find("textures"); itTextures != jsonGLTF.end()) {
				output.textures.reserve(itTextures->size());
				for (std::size_t textureId = 0; textureId < itTextures->size(); ++textureId) {
					try {
						parseTexture((*itTextures)[textureId], output);
					}
					catch (const std::exception& e) {
						throw std::runtime_error("Failed to read the textures property at texture " + std::to_string(textureId) + ": " + e.what());
					}
				}
			}

			if (auto itCameras = jsonGLTF.find("cameras"); itCameras != jsonGLTF.end()) {
				output.cameras.reserve(itCameras->size());
				for (std::size_t cameraId = 0; cameraId < itCameras->size(); ++cameraId) {
					try {
						parseCamera((*itCameras)[cameraId], output);
					}
					catch (const std::exception& e) {
						throw std::runtime_error("Failed to read the cameras property at camera " + std::to_string(cameraId) + ": " + e.what());
					}
				}
			}

			if (auto itMaterials = jsonGLTF.find("materials"); itMaterials != jsonGLTF.end()) {
				output.materials.reserve(itMaterials->size());
				for (std::size_t materialId = 0; materialId < itMaterials->size(); ++materialId) {
					try {
						parseMaterial((*itMaterials)[materialId], output);
					}
					catch (const std::exception& e) {
						throw std::runtime_error("Failed to read the materials property at material " + std::to_string(materialId) + ": " + e.what());
					}
				}
			}

			if (auto itMeshes = jsonGLTF.find("meshes"); itMeshes != jsonGLTF.end()) {
				output.meshes.reserve(itMeshes->size());
				for (std::size_t meshId = 0; meshId < itMeshes->size(); ++meshId) {
					try {
						parseMesh((*itMeshes)[meshId], output);
					}
					catch (const std::exception& e) {
						throw std::runtime_error("Failed to read the meshes property at mesh " + std::to_string(meshId) + ": " + e.what());
					}
				}
			}

			if (auto itNodes = jsonGLTF.find("nodes"); itNodes != jsonGLTF.end()) {
				output.entities.reserve(itNodes->size());
				for (std::size_t nodeId = 0; nodeId < itNodes->size(); ++nodeId) {
					try {
						parseNode((*itNodes)[nodeId], output);
					}
					catch (const std::exception& e) {
						throw std::runtime_error("Failed to read the nodes property at node " + std::to_string(nodeId) + ": " + e.what());
					}
				}
			}
		}
		catch (const std::exception& e) {
			throw std::runtime_error("Error while parsing the GLTF file \"" + path + "\": " + e.what());
		}

		return output;
	}


	bool GLTFReader::checkAssetVersion(const nlohmann::json& jsonAsset, int version, int revision)
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


	void GLTFReader::parseBuffer(const nlohmann::json& jsonBuffer)
	{
		auto itByteLength = jsonBuffer.find("byteLength");
		auto itUri = jsonBuffer.find("uri");

		if ((itByteLength != jsonBuffer.end()) && (itUri != jsonBuffer.end())) {
			std::size_t size = *itByteLength;
			std::string path = *itUri;

			std::ifstream dataFileStream(mBasePath + path, std::ios::in | std::ios::binary);
			if (dataFileStream.good()) {
				GLTFData::Buffer buffer(size);
				dataFileStream.read(reinterpret_cast<char*>(buffer.data()), size);
				if (!dataFileStream.fail()) {
					mGLTFData.buffers.emplace_back(std::move(buffer));
				}
				else {
					throw std::runtime_error("Failed to read buffer file");
				}
			}
			else {
				throw std::runtime_error("Can't open buffer file");
			}
		}
		else {
			throw std::runtime_error("Missing buffer properties");
		}
	}


	void GLTFReader::parseBufferView(const nlohmann::json& jsonBufferView)
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
			std::size_t byteStride = (itByteStride != jsonBufferView.end())? itByteOffset->get<std::size_t>() : 1;
			GLTFData::BufferView::Type type = GLTFData::BufferView::Type::Undefined;
			if (itTarget != jsonBufferView.end()) {
				if (*itTarget == 34962) { type = GLTFData::BufferView::Type::Array; }
				if (*itTarget == 34963) { type = GLTFData::BufferView::Type::ElementArray; }
			}

			if (bufferId < mGLTFData.buffers.size()) {
				mGLTFData.bufferViews.push_back({ bufferId, byteLength, byteOffset, byteStride, type });
			}
			else {
				throw std::runtime_error("Buffer index " + std::to_string(bufferId) + " out of range");
			}
		}
		else {
			throw std::runtime_error("Missing BufferView properties");
		}
	}


	void GLTFReader::parseAccessor(const nlohmann::json& jsonAccessor)
	{
		auto itBufferView = jsonAccessor.find("bufferView");
		auto itByteOffset = jsonAccessor.find("byteOffset");
		auto itComponentType = jsonAccessor.find("componentType");
		auto itNormalized = jsonAccessor.find("normalized");
		auto itCount = jsonAccessor.find("count");
		auto itType = jsonAccessor.find("type");
		if ((itBufferView != jsonAccessor.end()) && (itComponentType != jsonAccessor.end())
			&& (itCount != jsonAccessor.end()) && (itType != jsonAccessor.end())
		) {
			std::size_t bufferViewId	= *itBufferView;
			std::size_t byteOffset		= (itByteOffset != jsonAccessor.end())? itNormalized->get<std::size_t>() : 0;
			std::size_t componentType	= *itComponentType;
			bool normalized				= (itNormalized != jsonAccessor.end())? itNormalized->get<bool>() : false;
			std::size_t count			= *itCount;
			std::string type			= *itType;

			graphics::TypeId componentTypeId = (componentType == 5120)? graphics::TypeId::Byte :
				(componentType == 5121)? graphics::TypeId::UnsignedByte :
				(componentType == 5122)? graphics::TypeId::Short :
				(componentType == 5123)? graphics::TypeId::UnsignedShort :
				(componentType == 5125)? graphics::TypeId::UnsignedInt :
				(componentType == 5126)? graphics::TypeId::Float : graphics::TypeId::Int;
			std::size_t componentSize = (type == "SCALAR")? 1 :
				(type == "VEC2")? 2 : (type == "VEC3")? 3 : (type == "VEC4")? 4 :
				(type == "MAT2")? 4 : (type == "MAT3")? 9 : (type == "MAT4")? 16 : 0;

			if (bufferViewId < mGLTFData.bufferViews.size()) {
				if ((componentTypeId != graphics::TypeId::Int) && (componentSize > 0)) {
					mGLTFData.accessors.push_back({ bufferViewId, byteOffset, count, componentSize, componentTypeId, normalized });
				}
				else {
					throw std::runtime_error("Invalid component data");
				}
			}
			else {
				throw std::runtime_error("BufferView index " + std::to_string(bufferViewId) + " out of range");
			}
		}
		else {
			throw std::runtime_error("Missing accessor properties");
		}
	}


	void GLTFReader::parseSampler(const nlohmann::json& jsonSampler)
	{
		GLTFData::Sampler sampler;

		// Filters
		auto doFilter = [&](int filter, int idx) {
			if (sampler.enableFilter[idx]) {
				switch (filter) {
					case 9728:	sampler.filters[idx] = graphics::TextureFilter::Nearest;	break;
					case 9729:	sampler.filters[idx] = graphics::TextureFilter::Linear;		break;
					default:	throw std::runtime_error("Invalid filter " + std::to_string(filter));
				}
			}
		};

		auto itMinFilter = jsonSampler.find("minFilter");
		auto itMagFilter = jsonSampler.find("magFilter");
		sampler.enableFilter[0] = (itMinFilter != jsonSampler.end());
		sampler.enableFilter[1] = (itMagFilter != jsonSampler.end());
		doFilter(*itMinFilter, 0);
		doFilter(*itMagFilter, 1);

		// Wraps
		auto doWrap = [&](int wrap, int idx) {
			if (sampler.enableWrap[idx]) {
				switch (wrap) {
					case 33071:	sampler.wraps[idx] = graphics::TextureWrap::ClampToEdge;	break;
					case 10497:	sampler.wraps[idx] = graphics::TextureWrap::Repeat;			break;
					default:	throw std::runtime_error("Invalid wrap " + std::to_string(wrap));
				}
			}
		};

		auto itWrapS = jsonSampler.find("wrapS");
		auto itWrapT = jsonSampler.find("wrapT");
		sampler.enableWrap[0] = (itWrapS != jsonSampler.end());
		sampler.enableWrap[1] = (itWrapT != jsonSampler.end());
		doWrap(*itWrapS, 0);
		doWrap(*itWrapT, 1);
	}


	void GLTFReader::parseImage(const nlohmann::json& jsonImage, DataHolder& output) const
	{
		auto itUri = jsonImage.find("uri");
		if (itUri != jsonImage.end()) {
			output.images.emplace_back();
			*output.images.back() = loaders::ImageReader::read(mBasePath + itUri->get<std::string>(), utils::ImageFormat::RGB);
		}
		else {
			throw std::runtime_error("Missing uri property");
		}
	}


	void GLTFReader::parseTexture(const nlohmann::json& jsonTexture, DataHolder& output) const
	{
		auto texture = std::make_unique<graphics::Texture>();

		auto itSampler = jsonTexture.find("sampler");
		if (itSampler != jsonTexture.end()) {
			std::size_t samplerId = *itSampler;
			if (samplerId < mGLTFData.samplers.size()) {
				const GLTFData::Sampler& sampler = mGLTFData.samplers[samplerId];
				if (sampler.enableFilter[0] && sampler.enableFilter[1]) {
					texture->setFiltering(sampler.filters[0], sampler.filters[1]);
				}
				if (sampler.enableWrap[0] && sampler.enableWrap[1]) {
					texture->setWrapping(sampler.wraps[0], sampler.wraps[1]);
				}
			}
			else {
				throw std::runtime_error("Sampler index " + std::to_string(samplerId) + " out of range");
			}
		}

		auto itSource = jsonTexture.find("source");
		if (itSource != jsonTexture.end()) {
			std::size_t sourceId = *itSource;
			if (sourceId < output.images.size()){
				const utils::Image& image = *output.images[sourceId];

				graphics::ColorFormat format = graphics::ColorFormat::RGB;
				switch (image.format)
				{
					case utils::ImageFormat::RGB:	format = graphics::ColorFormat::RGB;	break;
					case utils::ImageFormat::RGBA:	format = graphics::ColorFormat::RGBA;	break;
					case utils::ImageFormat::L:		format = graphics::ColorFormat::Red;	break;
					case utils::ImageFormat::LA:	format = graphics::ColorFormat::Alpha;	break;
				}

				texture->setImage(image.pixels.get(), se::graphics::TypeId::UnsignedByte, format, image.width, image.height);
			}
			else {
				throw std::runtime_error("Source index " + std::to_string(sourceId) + " out of range");
			}
		}

		output.textures.push_back( std::move(texture) );
	}


	void GLTFReader::parseCamera(const nlohmann::json& jsonCamera, DataHolder& output) const
	{
		auto itType = jsonCamera.find("name");
		auto itPerspective = jsonCamera.find("perspective");
		auto itOrthographic = jsonCamera.find("orthographic");

		if (itType != jsonCamera.end()) {
			if ((*itType == "perspective") && (itPerspective != jsonCamera.end())) {
				auto itAspectRatio	= itPerspective->find("aspectRatio");
				auto itYFov			= itPerspective->find("yfov");
				auto itZFar			= itPerspective->find("zfar");
				auto itZNear		= itPerspective->find("znear");
				if ((itAspectRatio != jsonCamera.end()) && (itYFov != jsonCamera.end())
					&& (itZFar != jsonCamera.end()) && (itZNear != jsonCamera.end())
				) {
					auto camera = std::make_unique<graphics::Camera>();
					camera->setPerspectiveProjectionMatrix(*itYFov, *itAspectRatio, *itZNear, *itZFar);
					output.cameras.emplace_back(std::move(camera));
				}
				else {
					throw std::runtime_error("Missing perspective properties");
				}
			}
			else if ((*itType == "orthographic") && (itOrthographic != jsonCamera.end())) {
				auto itXMag			= itOrthographic->find("xmag");
				auto itYMag			= itOrthographic->find("ymag");
				auto itZFar			= itOrthographic->find("zfar");
				auto itZNear		= itOrthographic->find("znear");
				if ((itXMag != jsonCamera.end()) && (itYMag != jsonCamera.end())
					&& (itZFar != jsonCamera.end()) && (itZNear != jsonCamera.end())
				) {
					auto camera = std::make_unique<graphics::Camera>();
					camera->setOrthographicProjectionMatrix(*itXMag, *itYMag, *itZNear, *itZFar);
					output.cameras.emplace_back(std::move(camera));
				}
				else {
					throw std::runtime_error("Missing orthographic properties");
				}
			}
			else {
				throw std::runtime_error("Invalid type property \"" + itType->get<std::string>() + "\"");
			}
		}
		else {
			throw std::runtime_error("Missing type property");
		}
	}


	void GLTFReader::parseMaterial(const nlohmann::json& jsonMaterial, DataHolder& /*output*/) const
	{
		auto itName = jsonMaterial.find("name");
		if (itName != jsonMaterial.end()) {
			// material->name = *itName;
		}

		// TODO: auto itExtensions = jsonMaterial.find("extensions");
		// TODO: auto itExtras = jsonMaterial.find("extras");
		// TODO: auto itPBRMetallicRoughness = jsonMaterial.find("pbrMetallicRoughness");

		auto itNormalTexture = jsonMaterial.find("normalTexture");
		if (itNormalTexture != jsonMaterial.end()) {
			// TODO: auto normalTextureInfo = *itNormalTexture;
		}

		auto itOcclusionTexture = jsonMaterial.find("occlusionTexture");
		if (itOcclusionTexture != jsonMaterial.end()) {
			// TODO: auto occlusionTextureInfo = *itOcclusionTexture;
		}

		auto itEmissiveTexture = jsonMaterial.find("emissiveTexture");
		if (itEmissiveTexture != jsonMaterial.end()) {
			// TODO: auto emissiveTextureInfo = *itEmissiveTexture;
		}

		auto itEmissiveFactor = jsonMaterial.find("emissiveFactor");
		if (itEmissiveFactor != jsonMaterial.end()) {
			// std::vector<float> fVector = *itEmissiveFactor;
			// if (fVector.size() >= 3) {
			// 	glm::vec3 emissiveFactor = *reinterpret_cast<glm::vec3*>(fVector.data());
			// }
		}

		auto itAlphaMode = jsonMaterial.find("alphaMode");
		if (itAlphaMode != jsonMaterial.end()) {
			if (*itAlphaMode == "OPAQUE") {
			}
			else if (*itAlphaMode == "MASK") {
			}
			else if (*itAlphaMode == "BLEND") {
			}
		}

		auto itAlphaCutoff = jsonMaterial.find("alphaCutoff");
		if (itAlphaCutoff != jsonMaterial.end()) {
			// TODO: float alphaCutoff = *itAlphaCutoff;
		}

		auto itDoubleSide = jsonMaterial.find("doubleSide");
		if (itDoubleSide != jsonMaterial.end()) {
			// TODO: bool doubleSide = *itDoubleSide;
		}
	}


	void GLTFReader::parseMesh(const nlohmann::json& jsonMesh, DataHolder& output) const
	{
		static const std::map<std::string, MeshAttributes> kAttributeMap = {
			{ "POSITION", MeshAttributes::PositionAttribute },
			{ "NORMAL", MeshAttributes::NormalAttribute },
			{ "TEXCOORD_0", MeshAttributes::UVAttribute },
			{ "WEIGHTS_0", MeshAttributes::JointWeightAttribute },
			{ "JOINTS_0", MeshAttributes::JointIndexAttribute }
		};

		auto itPrimitives = jsonMesh.find("primitives");
		if ((itPrimitives != jsonMesh.end()) && !itPrimitives->empty()) {
			auto itFirstPrimitive = itPrimitives->front();

			graphics::VertexArray vao;
			std::vector<graphics::VertexBuffer> vbos;
			auto itAttributes = itFirstPrimitive.find("attributes");
			if (itAttributes != itFirstPrimitive.end()) {
				for (auto itAttribute = itAttributes->begin(); itAttribute != itAttributes->end(); ++itAttribute) {
					if (auto itAttributePair = kAttributeMap.find(itAttribute.key()); itAttributePair != kAttributeMap.end()) {
						std::size_t accessorId = *itAttribute;
						if (accessorId < mGLTFData.accessors.size()) {
							const GLTFData::Accessor& a		= mGLTFData.accessors[accessorId];
							const GLTFData::BufferView& bv	= mGLTFData.bufferViews[a.bufferViewId];
							const GLTFData::Buffer& b		= mGLTFData.buffers[bv.bufferId];
							vbos.emplace_back(b.data() + bv.offset + a.byteOffset, bv.length);
							vao.addBuffer(
								static_cast<unsigned int>(itAttributePair->second), vbos.back(),
								a.componentTypeId, a.normalized, a.componentSize, bv.stride
							);
						}
						else {
							throw std::runtime_error("Attribute index " + std::to_string(accessorId) + " out of range");
						}
					}
					else {
						throw std::runtime_error("Invalid attribute \"" + itAttribute.key() + "\"");
					}
				}
			}
			else {
				throw std::runtime_error("Missing attributes property");
			}

			auto itIndices = itFirstPrimitive.find("indices");
			if (itIndices != itFirstPrimitive.end()) {
				std::size_t accessorId = *itIndices;
				if (accessorId < mGLTFData.accessors.size()) {
					const GLTFData::Accessor& a		= mGLTFData.accessors[accessorId];
					const GLTFData::BufferView& bv	= mGLTFData.bufferViews[a.bufferViewId];
					const GLTFData::Buffer& b		= mGLTFData.buffers[bv.bufferId];
					graphics::IndexBuffer ibo(b.data() + bv.offset + a.byteOffset, bv.length, a.componentTypeId, a.count);

					auto mesh = std::make_unique<graphics::Mesh>(std::move(vbos), std::move(ibo), std::move(vao));
					output.meshes.emplace_back(std::move(mesh));
				}
				else {
					throw std::runtime_error("Accessor index " + std::to_string(accessorId) + " out of range");
				}
			}
		}
		else {
			throw std::runtime_error("Missing primitives property");
		}
	}


	void GLTFReader::parseNode(const nlohmann::json& jsonNode, DataHolder& output) const
	{
		auto itName = jsonNode.find("name");
		if (itName == jsonNode.end()) {
			throw std::runtime_error("Name property is needed");
		}

		auto entity = std::make_unique<app::Entity>(*itName);

		auto itRotation = jsonNode.find("rotation");
		if (itRotation != jsonNode.end()) {
			std::vector<float> fVector = *itRotation;
			if (fVector.size() >= 3) {
				entity->orientation = *reinterpret_cast<glm::vec3*>(fVector.data());
			}
		}

		auto itScale = jsonNode.find("scale");
		if (itScale != jsonNode.end()) {
			std::vector<float> fVector = *itScale;
			if (fVector.size() >= 3) {
				entity->scale = *reinterpret_cast<glm::vec3*>(fVector.data());
			}
		}

		auto itTranslation = jsonNode.find("translation");
		if (itTranslation != jsonNode.end()) {
			std::vector<float> fVector = *itTranslation;
			if (fVector.size() >= 3) {
				entity->position = *reinterpret_cast<glm::vec3*>(fVector.data());
			}
		}

		output.entities.emplace_back(std::move(entity));
		std::size_t entityId = output.entities.size() - 1;

		auto itMesh = jsonNode.find("mesh");
		if (itMesh != jsonNode.end()) {
			std::size_t meshId = *itMesh;
			if (meshId < output.meshes.size()) {
				output.entityMeshMap.emplace(entityId, meshId);
			}
			else {
				throw std::runtime_error("Mesh index " + std::to_string(meshId) + " out of range");
			}
		}
	}

}
