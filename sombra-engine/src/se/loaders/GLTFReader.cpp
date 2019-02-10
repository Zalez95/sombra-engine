#include <map>
#include <sstream>
#include <fstream>
#include <nlohmann/json.hpp>
#include "se/loaders/GLTFReader.h"
#include "se/loaders/ImageReader.h"

namespace se::loaders {

	bool GLTFReader::load(std::istream& input)
	{
		bool success = false;

		nlohmann::json jsonGLTF;
		input >> jsonGLTF;

		if (!input.fail()) {
			auto itAsset = jsonGLTF.find("asset");
			if ((itAsset != jsonGLTF.end())
				&& checkAssetVersion(*itAsset, FileFormat::sVersion, FileFormat::sRevision)
			) {
				success = true;
				LoadedData loadedData;

				auto itBuffers = jsonGLTF.find("buffers");
				if ((itBuffers != jsonGLTF.end()) && success) {
					mGLTFData.buffers.reserve(itBuffers->size());
					for (const nlohmann::json& jsonBuffer : *itBuffers) {
						if (!parseBuffer(jsonBuffer)) {
							success = false;
							break;
						}
					}
				}

				auto itBufferViews = jsonGLTF.find("bufferViews");
				if ((itBufferViews != jsonGLTF.end()) && success) {
					mGLTFData.bufferViews.reserve(itBufferViews->size());
					for (const nlohmann::json& jsonBufferView : *itBufferViews) {
						if (!parseBufferView(jsonBufferView)) {
							success = false;
							break;
						}
					}
				}

				auto itAccessors = jsonGLTF.find("accessors");
				if ((itAccessors != jsonGLTF.end()) && success) {
					mGLTFData.accessors.reserve(itAccessors->size());
					for (const nlohmann::json& jsonAccessor : *itAccessors) {
						if (!parseAccessor(jsonAccessor)) {
							success = false;
							break;
						}
					}
				}

				auto itImages = jsonGLTF.find("images");
				if ((itImages != jsonGLTF.end()) && success) {
					mLoadedData.images.reserve(itImages->size());
					for (const nlohmann::json& jsonImage : *itImages) {
						if (!parseImage(jsonImage)) {
							success = false;
							break;
						}
					}
				}

				auto itSamplers = jsonGLTF.find("samplers");
				if ((itSamplers != jsonGLTF.end()) && success) {
					mGLTFData.samplers.reserve(itSamplers->size());
					for (const nlohmann::json& jsonSampler : *itSamplers) {
						if (!parseSampler(jsonSampler)) {
							success = false;
							break;
						}
					}
				}

				auto itTextures = jsonGLTF.find("textures");
				if ((itTextures != jsonGLTF.end()) && success) {
					mLoadedData.textures.reserve(itTextures->size());
					for (const nlohmann::json& jsonTexture : *itTextures) {
						if (!parseTexture(jsonTexture)) {
							success = false;
							break;
						}
					}
				}

				auto itCameras = jsonGLTF.find("cameras");
				if ((itCameras != jsonGLTF.end()) && success) {
					mLoadedData.cameras.reserve(itCameras->size());
					for (const nlohmann::json& jsonCamera : *itCameras) {
						if (!parseCamera(jsonCamera)) {
							success = false;
							break;
						}
					}
				}

				auto itMaterials = jsonGLTF.find("materials");
				if ((itMaterials != jsonGLTF.end()) && success) {
					mLoadedData.materials.reserve(itMaterials->size());
					for (const nlohmann::json& jsonMaterial : *itMaterials) {
						if (!parseMaterial(jsonMaterial)) {
							success = false;
							break;
						}
					}
				}

				auto itMeshes = jsonGLTF.find("meshes");
				if ((itMeshes != jsonGLTF.end()) && success) {
					mLoadedData.meshes.reserve(itMaterials->size());
					for (const nlohmann::json& jsonMesh : *itMeshes) {
						if (!parseMesh(jsonMesh)) {
							success = false;
							break;
						}
					}
				}

				auto itNodes = jsonGLTF.find("nodes");
				if ((itNodes != jsonGLTF.end()) && success) {
					mLoadedData.entities.reserve(itNodes->size());
					for (const nlohmann::json& jsonNode : *itNodes) {
						if (!parseNode(jsonNode)) {
							success = false;
							break;
						}
					}
				}
			}
		}

		return success;
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


	bool GLTFReader::parseBuffer(const nlohmann::json& jsonBuffer)
	{
		bool success = false;

		auto itByteLength = jsonBuffer.find("byteLength");
		auto itUri = jsonBuffer.find("uri");

		if ((itByteLength != jsonBuffer.end()) && (itUri != jsonBuffer.end())) {
			std::size_t size = *itByteLength;
			std::string path = *itUri;

			std::ifstream dataFileStream(path, std::ios::in | std::ios::binary);
			if (dataFileStream.good()) {
				GLTFData::Buffer buffer(size);
				dataFileStream.read(reinterpret_cast<char*>(buffer.data()), size);

				if (!dataFileStream.fail()) {
					mGLTFData.buffers.emplace_back(std::move(buffer));
					success = true;
				}
			}
		}

		return success;
	}


	bool GLTFReader::parseBufferView(const nlohmann::json& jsonBufferView)
	{
		bool success = false;

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
				success = true;
			}
		}

		return success;
	}


	bool GLTFReader::parseAccessor(const nlohmann::json& jsonAccessor)
	{
		bool success = false;

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
			std::size_t byteOffset		= *itByteOffset;
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

			if ((bufferViewId < mGLTFData.bufferViews.size())
				&& (componentTypeId != graphics::TypeId::Int) && (componentSize > 0)
			) {
				mGLTFData.accessors.push_back({ bufferViewId, byteOffset, count, componentSize, componentTypeId, normalized });
				success = true;
			}
		}

		return success;
	}


	bool GLTFReader::parseImage(const nlohmann::json& jsonImage)
	{
		bool success = false;

		auto itUri = jsonImage.find("uri");
		if (itUri != jsonImage.end()) {
			mLoadedData.images.emplace_back();
			*mLoadedData.images.back() = loaders::ImageReader::read(*itUri, utils::ImageFormat::RGB);
			success = true;
		}

		return success;
	}


	bool GLTFReader::parseSampler(const nlohmann::json& jsonSampler)
	{
		bool success = true;
		GLTFData::Sampler sampler;

		// Filters
		auto doFilter = [&](int filter, int idx) {
			if (sampler.enableFilter[idx] && success) {
				switch (filter) {
					case 9728:	sampler.filters[idx] = graphics::TextureFilter::Nearest;	break;
					case 9729:	sampler.filters[idx] = graphics::TextureFilter::Linear;		break;
					default:	success = false;
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
			if (sampler.enableWrap[idx] && success) {
				switch (wrap) {
					case 33071:	sampler.wraps[idx] = graphics::TextureWrap::ClampToEdge;	break;
					case 10497:	sampler.wraps[idx] = graphics::TextureWrap::Repeat;			break;
					default:	success = false;
				}
			}
		};

		auto itWrapS = jsonSampler.find("wrapS");
		auto itWrapT = jsonSampler.find("wrapT");
		sampler.enableWrap[0] = (itWrapS != jsonSampler.end());
		sampler.enableWrap[1] = (itWrapT != jsonSampler.end());
		doWrap(*itWrapS, 0);
		doWrap(*itWrapT, 1);

		return success;
	}


	bool GLTFReader::parseTexture(const nlohmann::json& jsonTexture)
	{
		bool success = true;

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
				success = false;
			}
		}

		auto itSource = jsonTexture.find("source");
		if (itSource != jsonTexture.end()) {
			std::size_t sourceId = *itSource;
			if (sourceId < mLoadedData.images.size()){
				const utils::Image& image = *mLoadedData.images[sourceId];

				graphics::ColorFormat format;
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
				success = false;
			}
		}

		mLoadedData.textures.push_back( std::move(texture) );

		return success;
	}


	bool GLTFReader::parseCamera(const nlohmann::json& jsonCamera)
	{
		bool success = false;

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
					mLoadedData.cameras.emplace_back(std::move(camera));
					success = true;
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
					mLoadedData.cameras.emplace_back(std::move(camera));
					success = true;
				}
			}
		}

		return success;
	}


	bool GLTFReader::parseMaterial(const nlohmann::json& jsonMaterial)
	{
		bool success = false;

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

		return success;
	}


	bool GLTFReader::parseMesh(const nlohmann::json& jsonMesh)
	{
		static const std::map<std::string, LoadedData::MeshAttributes> kAttributeMap = {
			{ "POSITION", LoadedData::MeshAttributes::PositionAttribute },
			{ "NORMAL", LoadedData::MeshAttributes::NormalAttribute },
			{ "TEXCOORD_0", LoadedData::MeshAttributes::UVAttribute },
			{ "WEIGHTS_0", LoadedData::MeshAttributes::JointWeightAttribute },
			{ "JOINTS_0", LoadedData::MeshAttributes::JointIndexAttribute }
		};

		bool success = false;

		graphics::VertexArray vao;
		std::vector<graphics::VertexBuffer> vbos;
		auto itAttributes = jsonMesh.find("attributes");
		if (itAttributes != jsonMesh.end()) {
			for (auto attributePair : kAttributeMap) {
				auto itAttribute = itAttributes->find(attributePair.first);
				std::size_t accessorId = (itAttribute != itAttributes->end())? itAttribute->get<std::size_t>() : -1;
				if (accessorId < mGLTFData.accessors.size()) {
					const GLTFData::Accessor& a		= mGLTFData.accessors[accessorId];
					const GLTFData::BufferView& bv	= mGLTFData.bufferViews[a.bufferViewId];
					const GLTFData::Buffer& b		= mGLTFData.buffers[bv.bufferId];
					vbos.emplace_back(b.data() + bv.offset + a.byteOffset, bv.length);
					vao.addBuffer(
						static_cast<int>(attributePair.second), vbos.back(),
						a.componentTypeId, a.normalized, a.componentSize, bv.stride
					);
				}
			}
		}

		auto itIndices = jsonMesh.find("indices");
		if (itIndices != jsonMesh.end()) {
			std::size_t accessorId = *itIndices;
			const GLTFData::Accessor&	a = mGLTFData.accessors[accessorId];
			const GLTFData::BufferView&	bv = mGLTFData.bufferViews[a.bufferViewId];
			const GLTFData::Buffer&		b = mGLTFData.buffers[bv.bufferId];
			graphics::IndexBuffer ibo(b.data() + bv.offset + a.byteOffset, bv.length, a.componentTypeId, a.count);

			auto mesh = std::make_unique<graphics::Mesh>(std::move(vbos), std::move(ibo), std::move(vao));
			mLoadedData.meshes.emplace_back(std::move(mesh));
			// TODO: renderable - material
			success = true;
		}

		return success;
	}


	bool GLTFReader::parseNode(const nlohmann::json& jsonNode)
	{
		bool success = false;

		auto itName = jsonNode.find("name");
		if (itName != jsonNode.end()) {
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

			mLoadedData.entities.emplace_back(std::move(entity));
			success = true;
		}

		return success;
	}

}
