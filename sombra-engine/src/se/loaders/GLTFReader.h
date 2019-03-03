#ifndef GLTF_READER_H
#define GLTF_READER_H

#include <string>
#include <cstddef>
#include <nlohmann/json_fwd.hpp>
#include "se/loaders/SceneReader.h"
#include "se/utils/Image.h"
#include "se/graphics/Texture.h"

namespace se::loaders {

	/**
	 * Class GLTFReader, it's used to create meshes from the given texts
	 */
	class GLTFReader : public SceneReader
	{
	private:	// Nested types
		using Buffer = std::vector<std::byte>;
		using PrimitiveIndices = std::vector<std::size_t>;

		/** Struct FileFormat, holds the version of a valid GLTF file format */
		struct FileFormat
		{
			static constexpr int sVersion = 2;
			static constexpr int sRevision = 0;
		};

		/** Struct BufferView, holds the data of a GLTF bufferView */
		struct BufferView
		{
			std::size_t bufferId, length, offset;
			int stride;
			enum class Type { Array, ElementArray, Undefined } type;
		};

		/** Struct Accessor, holds the data of a GLTF accessor */
		struct Accessor
		{
			std::size_t bufferViewId, byteOffset, count;
			int componentSize;
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

		/** Struct GLTFData, it holds validated GLTF data */
		struct GLTFData
		{
			std::vector<Buffer> buffers;
			std::vector<BufferView> bufferViews;
			std::vector<Accessor> accessors;
			std::vector<Sampler> samplers;
			std::vector<std::unique_ptr<utils::Image>> images;
			std::vector<std::shared_ptr<graphics::Texture>> textures;
			std::vector<std::shared_ptr<graphics::Material>> materials;
			std::vector<PrimitiveIndices> meshPrimitivesIndices;
		};

	private:	// Attributes
		/** All the temporally generated GLTF data */
		GLTFData mGLTFData;

		/** The base path of the file to parse */
		std::string mBasePath;

	public:		// Functions
		/** Parses the GLTF Scene located at the given file
		 *
		 * @param	path the path to the file to parse
		 * @return	a DataHolder with the Scene data
		 * @throw	runtime_error in case of any error while parsing */
		DataHolder load(const std::string& path) override;
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
		 * mGLTFData
		 *
		 * @param	jsonBuffer the json object with the buffer data to load
		 * @return	true if the Buffer was created successfully, false otherwise
		 * @note	it only supports buffers in GLB files */
		void parseBuffer(const nlohmann::json& jsonBuffer);

		/** Reads the given GLTF JSON BufferView and appends it to mGLTFData
		 *
		 * @param	jsonBufferView the json object with the Buffer View data to
		 *			load
		 * @return	true if the BufferView was loaded successfully, false
		 *			otherwise */
		void parseBufferView(const nlohmann::json& jsonBufferView);

		/** Loads the Array/Index Buffer from the given GLTF JSON Accessor and
		 * appends it to mGLTFData
		 *
		 * @param	jsonAccessor the json object with the Accessor data to load
		 * @return	true if the Accessor was loaded successfully, false
		 *			otherwise */
		void parseAccessor(const nlohmann::json& jsonAccessor);

		/** Loads the texture Sampler from the given GLTF JSON Sampler and
		 * appends it to mGLTFData
		 *
		 * @param	jsonSampler the json object with the Sampler data to load
		 * @return	true if the Sampler was loaded successfully, false
		 *			otherwise */
		void parseSampler(const nlohmann::json& jsonSampler);

		/** Loads the given GLTF JSON image and appends it to mGLTFData
		 *
		 * @param	jsonImage the json object with the Image data to load
		 * @throw	runtime_error in case of any error while parsing the Image
		 * @note	bufferView images aren't supported yet */
		void parseImage(const nlohmann::json& jsonImage);

		/** Loads the texture from the given GLTF JSON Texture and appends it
		 * to mGLTFData
		 *
		 * @param	jsonTexture the json object with the Texture data to load
		 * @throw	runtime_error in case of any error while parsing the
		 *			Texture */
		void parseTexture(const nlohmann::json& jsonTexture);

		/** Creates a new Material from the given GLTF JSON Material and appends
		 * it to mGLTFData
		 *
		 * @param	jsonMaterial the JSON object with the Material to parse
		 * @throw	runtime_error in case of any error while parsing the
		 *			Material */
		void parseMaterial(const nlohmann::json& jsonMaterial);

		/** Creates a new Renderable3D from the given GLTF JSON primitive and
		 * appends it to mGLTFData
		 *
		 * @param	jsonPrimitive the JSON object with the Primitive to parse
		 * @param	output the DataHolder where the loaded Renderable3D will be
		 *			stored
		 * @throw	runtime_error in case of any error while parsing the
		 *			Primitive */
		void parsePrimitive(
			const nlohmann::json& jsonMesh, DataHolder& output
		) const;

		/** Creates Renderable3Ds from the given GLTF JSON Mesh and appends
		 * them to the DataHolder. The indices to the primitives/Renderable3Ds
		 * of the parsed Mesh will be appended to mGLTFData
		 *
		 * @param	jsonMesh the JSON object with the Mesh to parse
		 * @param	output the DataHolder where the loaded Mesh will be stored
		 * @throw	runtime_error in case of any error while parsing the Mesh
		 * @note	Morph targets arent supported yet */
		void parseMesh(const nlohmann::json& jsonMesh, DataHolder& output);

		/** Creates a new Camera from the given GLTF JSON Camera and appends
		 * it to the DataHolder
		 *
		 * @param	jsonCamera the JSON object with the Camera to parse
		 * @param	output the DataHolder where the loaded Camera will be stored
		 * @throw	runtime_error in case of any error while parsing the
		 *			Camera */
		void parseCamera(
			const nlohmann::json& jsonCamera, DataHolder& output
		) const;

		/** Creates a new Entity from the given GLTF JSON Node and appends it to
		 * the DataHolder
		 *
		 * @param	jsonNode the JSON object with the Node to parse
		 * @param	output the DataHolder where the loaded Node will be stored
		 * @throw	runtime_error in case of any error while parsing the Node */
		void parseNode(
			const nlohmann::json& jsonNode, DataHolder& output
		) const;

		// void loadTextureInfo(const nlohmann::json& jsonObject) {};
		// void loadAnimation(const nlohmann::json& jsonObject) {};
		// void loadExtension(const nlohmann::json& jsonObject) {};
		// void loadExtras(const nlohmann::json& jsonObject) {};
		// void loadScene(const nlohmann::json& jsonObject) {};
		// void loadSkin(const nlohmann::json& jsonObject) {};
	};

}

#endif		// GLTF_READER_H
