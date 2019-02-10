#ifndef GLTF_READER_H
#define GLTF_READER_H

#include <string>
#include <cstddef>
#include <nlohmann/json_fwd.hpp>
#include "../app/Entity.h"
#include "../utils/Image.h"
#include "../graphics/Texture.h"
#include "../graphics/3D/Mesh.h"
#include "../graphics/3D/Camera.h"
#include "../graphics/3D/Material.h"

namespace se::loaders {

	/**
	 * Class GLTFReader, it's used to create meshes from the given texts
	 */
	class GLTFReader
	{
	private:	// Nested types
		/** Struct FileFormat, it holds the name, version and other data of
		 * our Entity file format */
		struct FileFormat
		{
			static constexpr int sVersion = 2;
			static constexpr int sRevision = 0;
		};

		/** Struct GLTFData, it holds validated GLTF raw data */
		struct GLTFData
		{
			using Buffer = std::vector<std::byte>;

			/** Struct BufferView, holds the data of a GLTF bufferView */
			struct BufferView
			{
				std::size_t bufferId, length, offset, stride;
				enum class Type { Array, ElementArray, Undefined } type;
			};

			/** Struct Accessor, holds the data of a GLTF accessor */
			struct Accessor
			{
				std::size_t bufferViewId, byteOffset, count, componentSize;
				graphics::TypeId componentTypeId;
				bool normalized;
			};

			/** Struct Sampler, holds the data of a GLTF sampler */
			struct Sampler
			{
				bool enableFilter[2], enableWrap[2];
				graphics::TextureFilter filters[2];
				graphics::TextureWrap wraps[2];
			};

			std::vector<Buffer> buffers;
			std::vector<BufferView> bufferViews;
			std::vector<Accessor> accessors;
			std::vector<Sampler> samplers;
		};

		/** Struct LoadedData, it holds all the data loaded by the Reader */
		struct LoadedData
		{
			/** The attribute indices of the VAOs */
			enum class MeshAttributes : unsigned int
			{
				PositionAttribute = 0,
				NormalAttribute,
				UVAttribute,
				JointWeightAttribute,
				JointIndexAttribute
			};

			std::vector<std::unique_ptr<app::Entity>> entities;
			std::vector<std::unique_ptr<utils::Image>> images;
			std::vector<std::unique_ptr<graphics::Mesh>> meshes;
			std::vector<std::unique_ptr<graphics::Camera>> cameras;
			std::vector<std::unique_ptr<graphics::Texture>> textures;
			std::vector<std::unique_ptr<graphics::Material>> materials;
		};

	private:	// Attributes
		/** All the validated GLTF raw data */
		GLTFData mGLTFData;

		/** All the data loaded by the Reader */
		LoadedData mLoadedData;

		/** The GraphicsManager that will store the readed graphics data */
		// app::GraphicsManager& mGraphicsManager;

	public:		// Functions
		/** Creates a new EntityReader
		 *
		 * @param	graphicsManager the GraphicsManager where we will store
		 *			the readed graphics data */
		// GLTFReader(app::GraphicsManager& graphicsManager) :
		// 	mGraphicsManager(graphicsManager) {};

		/** Parses the Entities in the given stream
		 *
		 * @param	input the input stream to parse
		 * @return	true if the input was parsed successfully, false
		 *			otherwise */
		bool load(std::istream& input);
	private:
		/** Checks the version of the given GLTF JSON asset
		 *
		 * @param	jsonAsset the GLTF JSON asset to check
		 * @param	version the expected version number of the asset
		 * @param	revision the expected revision number of the asset
		 * @return	true if the JSON asset version is accepted, false
		 *			otherwise */
		static bool checkAssetVersion(
			const nlohmann::json& jsonAsset, int version, int revision
		);

		/** Reads the Buffer from the given GLTF JSON Buffer and appends it to
		 * the GLTFData
		 *
		 * @param	jsonBuffer the json object with the buffer data to load
		 * @return	true if the Buffer was created successfully, false otherwise
		 * @note	it only supports buffers in GLB files */
		bool parseBuffer(const nlohmann::json& jsonBuffer);

		/** Reads the given GLTF JSON BufferView and appends it to the
		 * GLTFData
		 *
		 * @param	jsonBufferView the json object with the Buffer View data to
		 *			load
		 * @return	true if the BufferView was loaded successfully, false
		 *			otherwise */
		bool parseBufferView(const nlohmann::json& jsonBufferView);

		/** Loads the Array/Index Buffer from the given GLTF JSON Accessor and
		 * appends it to the GLTFData
		 *
		 * @param	jsonAccessor the json object with the Accessor data to load
		 * @return	true if the Accessor was loaded successfully, false
		 *			otherwise */
		bool parseAccessor(const nlohmann::json& jsonAccessor);

		/** Loads the given GLTF JSON image and appends it to the LoadedData
		 *
		 * @param	jsonImage the json object with the Image data to load
		 * @return	true if the Image was loaded successfully, false
		 *			otherwise
		 * @note	bufferView images aren't supported yet */
		bool parseImage(const nlohmann::json& jsonImage);

		/** Loads the texture Sampler from the given GLTF JSON Sampler and
		 * appends it to the GLTFData
		 *
		 * @param	jsonSampler the json object with the Sampler data to load
		 * @return	true if the Sampler was loaded successfully, false
		 *			otherwise */
		bool parseSampler(const nlohmann::json& jsonSampler);

		/** Loads the texture from the given GLTF JSON Texture and appends it
		 * to the LoadedData
		 *
		 * @param	jsonTexture the json object with the Texture data to load
		 * @return	true if the Texture was loaded successfully, false
		 *			otherwise */
		bool parseTexture(const nlohmann::json& jsonTexture);

		// void loadTextureInfo(const nlohmann::json& jsonObject) {};
		// void loadAnimation(const nlohmann::json& jsonObject) {};
		// void loadAsset(const nlohmann::json& jsonObject) {};
		// void loadExtension(const nlohmann::json& jsonObject) {};
		// void loadExtras(const nlohmann::json& jsonObject) {};

		/** Creates a new Camera from the given GLTF JSON Camera and appends
		 * it to the LoadedData
		 *
		 * @param	jsonCamera the JSON object with the Camera to parse
		 * @return	true if the Camera was created successfully, false
		 *			otherwise */
		bool parseCamera(const nlohmann::json& jsonCamera);

		/** Creates a new Material from the given GLTF JSON Material and appends
		 * it to the LoadedData
		 *
		 * @param	jsonMaterial the JSON object with the Material to parse
		 * @return	true if the Material was created successfully, false
		 *			otherwise */
		bool parseMaterial(const nlohmann::json& jsonMaterial);

		/** Creates a new Mesh from the given GLTF JSON Mesh and appends it to
		 * the LoadedData
		 *
		 * @param	jsonMesh the JSON object with the Mesh to parse
		 * @return	true if the Mesh was created successfully, false otherwise
		 * @note	Morph targets arent supported yet */
		bool parseMesh(const nlohmann::json& jsonMesh);

		/** Creates a new Entity from the given GLTF JSON Node and appends it to
		 * the LoadedData
		 *
		 * @param	jsonNode the JSON object with the Node to parse
		 * @return	true if the Entity was created successfully, false
		 *			otherwise */
		bool parseNode(const nlohmann::json& jsonNode);

		// void loadSampler(const nlohmann::json& jsonObject) {};
		// void loadScene(const nlohmann::json& jsonObject) {};
		// void loadSkin(const nlohmann::json& jsonObject) {};
	};

}

#endif		// GLTF_READER_H
