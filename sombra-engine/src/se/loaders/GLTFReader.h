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
		using MaterialSPtr = std::shared_ptr<graphics::Material>;
		using TextureSPtr = std::shared_ptr<graphics::Texture>;
		using Buffer = std::vector<std::byte>;

		/** Struct FileFormat, holds the version of a valid GLTF file format */
		struct FileFormat
		{
			static constexpr int sVersion = 2;
			static constexpr int sRevision = 0;
		};

		/** Struct Accessor, holds the data of a GLTF accessor */
		struct Accessor
		{
			std::size_t bufferViewId, byteOffset, count;
			int componentSize;
			graphics::TypeId componentTypeId;
			bool normalized;
		};

		/** Struct BufferView, holds the data of a GLTF bufferView */
		struct BufferView
		{
			std::size_t bufferId, length, offset;
			int stride;
			enum class Target { Array, ElementArray, Undefined } target;
		};

		/** Struct Sampler, holds the data of a GLTF sampler */
		struct Sampler
		{
			graphics::TextureFilter filters[2];
			graphics::TextureWrap wraps[2];
		};

		/** Struct Skin, holds the data of a GLTF Skin */
		struct Skin
		{
			std::string name;
			int inverseBindMatrices, skeleton;
			bool hasInverseBindMatrices, hasSkeleton;
			std::vector<int> joints;
		};

		/** Struct Node, holds the data of a Node in the GLTF Node hierarchy */
		struct Node
		{
			int camera, mesh, skin;
			bool hasCamera, hasMesh, hasSkin;
			std::vector<int> children;
			animation::NodeData nodeData;
		};

		/** Struct GLTFData, it holds validated GLTF data */
		struct GLTFData
		{
			std::vector<Accessor> accessors;
			std::vector<Buffer> buffers;
			std::vector<BufferView> bufferViews;
			std::vector<Sampler> samplers;
			std::vector<utils::Image> images;
			std::vector<TextureSPtr> textures;
			std::vector<MaterialSPtr> materials;
			std::vector<Skin> skins;
			std::vector<Node> nodes;
		};

	private:	// Attributes
		/** The base path of the file to parse */
		std::string mBasePath;

		/** The default GLTF material */
		MaterialSPtr mDefaultMaterial;

		/** The temporarily read GLTF data */
		GLTFData mGLTFData;

	public:		// Functions
		/** Creates a new GLTFReader */
		GLTFReader();

		/** Parses the GLTF Scene located at the given file
		 *
		 * @param	path the path to the GLTF file to parse
		 * @param	output the DataHolder where the Scene data will be stored
		 * @return	a Result object with the result of the load operation */
		Result load(const std::string& path, DataHolder& output) override;
	private:
		/** Reads the JSON file located at the given path
		 *
		 * @param	path the path where the file to read is located
		 * @param	output the JSON object where the data will be stored
		 * @return	a Result object with the result of the read operation */
		static Result readJSON(const std::string& path, nlohmann::json& output);

		/** Parses the given JSON object to the given output object
		 *
		 * @param	jsonGLTF the JSON object where the source data is stored
		 * @param	output the DataHolder where the Scene data will be stored
		 * @return	a Result object with the result of the parse operation */
		Result parseGLTF(const nlohmann::json& jsonGLTF, DataHolder& output);

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
		 * @return	a Result object with the result of the parse operation
		 * @note	it only supports buffers in GLB files */
		Result parseBuffer(const nlohmann::json& jsonBuffer);

		/** Reads the given GLTF JSON BufferView and appends it to mGLTFData
		 *
		 * @param	jsonBufferView the json object with the BufferView data to
		 *			load
		 * @return	a Result object with the result of the parse operation */
		Result parseBufferView(const nlohmann::json& jsonBufferView);

		/** Loads the Array/Index Buffer from the given GLTF JSON Accessor and
		 * appends it to mGLTFData
		 *
		 * @param	jsonAccessor the json object with the Accessor data to load
		 * @return	a Result object with the result of the parse operation */
		Result parseAccessor(const nlohmann::json& jsonAccessor);

		/** Loads the texture Sampler from the given GLTF JSON Sampler and
		 * appends it to mGLTFData
		 *
		 * @param	jsonSampler the json object with the Sampler data to load
		 * @return	a Result object with the result of the parse operation */
		Result parseSampler(const nlohmann::json& jsonSampler);

		/** Loads the given GLTF JSON image and appends it to mGLTFData
		 *
		 * @param	jsonImage the json object with the Image data to load
		 * @return	a Result object with the result of the parse operation
		 * @note	bufferView images aren't supported yet */
		Result parseImage(const nlohmann::json& jsonImage);

		/** Loads the texture from the given GLTF JSON Texture and appends it
		 * to mGLTFData
		 *
		 * @param	jsonTexture the json object with the Texture data to load
		 * @return	a Result object with the result of the parse operation */
		Result parseTexture(const nlohmann::json& jsonTexture);

		/** Creates a new Material from the given GLTF JSON Material and appends
		 * it to mGLTFData
		 *
		 * @param	jsonMaterial the JSON object with the Material to parse
		 * @return	a Result object with the result of the parse operation */
		Result parseMaterial(const nlohmann::json& jsonMaterial);

		/** Creates a new Renderable3D from the given GLTF JSON primitive and
		 * appends it to mGLTFData
		 *
		 * @param	jsonPrimitive the JSON object with the Primitive to parse
		 * @param	output the DataHolder where the loaded Renderable3D will be
		 *			stored
		 * @return	a Result object with the result of the parse operation */
		Result parsePrimitive(
			const nlohmann::json& jsonPrimitive, DataHolder& output
		) const;

		/** Creates Renderable3Ds from the given GLTF JSON Mesh and appends
		 * them to the DataHolder. The indices to the primitives/Renderable3Ds
		 * of the parsed Mesh will be appended to mGLTFData
		 *
		 * @param	jsonMesh the JSON object with the Mesh to parse
		 * @param	output the DataHolder where the loaded Mesh will be stored
		 * @return	a Result object with the result of the parse operation
		 * @note	Morph targets arent supported yet */
		Result parseMesh(const nlohmann::json& jsonMesh, DataHolder& output);

		/** Creates a new Skin from the given GLTF JSON Node and appends
		 * it to the mGLTFData
		 *
		 * @param	jsonSkin the JSON object with the Skin to parse
		 * @return	a Result object with the result of the parse operation */
		Result parseSkin(const nlohmann::json& jsonSkin);

		/** Creates a new Camera from the given GLTF JSON Camera and appends
		 * it to the DataHolder
		 *
		 * @param	jsonCamera the JSON object with the Camera to parse
		 * @param	output the DataHolder where the loaded Camera will be stored
		 * @return	a Result object with the result of the parse operation */
		Result parseCamera(
			const nlohmann::json& jsonCamera, DataHolder& output
		) const;

		/** Creates a new Entity from the given GLTF JSON Node and appends it to
		 * the DataHolder
		 *
		 * @param	jsonNode the JSON object with the Node to parse
		 * @return	a Result object with the result of the parse operation */
		Result parseNode(const nlohmann::json& jsonNode);

		/** Creates a new Scene from the given GLTF JSON Scene and appends it
		 * to the DataHolder
		 *
		 * @param	jsonNode the JSON object with the Scene to parse
		 * @param	output the DataHolder where the loaded Scene will be stored
		 * @return	a Result object with the result of the parse operation */
		Result parseScene(
			const nlohmann::json& jsonNode, DataHolder& output
		) const;

		/** Creates a new Animation from the given GLTF JSON Node and appends
		 * it to the DataHolder
		 *
		 * @param	jsonNode the JSON object with the Node to parse
		 * @param	output the DataHolder where the loaded Animation will be
		 *			stored
		 * @return	a Result object with the result of the parse operation *
		Result parseAnimation(
			const nlohmann::json& jsonNode, DataHolder& output
		) const;*/

		// Result loadTextureInfo(const nlohmann::json& jsonObject) {};
		// Result loadAnimation(const nlohmann::json& jsonObject) {};
		// Result loadExtension(const nlohmann::json& jsonObject) {};
		// Result loadExtras(const nlohmann::json& jsonObject) {};
		// Result loadScene(const nlohmann::json& jsonObject) {};
		// Result loadSkin(const nlohmann::json& jsonObject) {};
	};

}

#endif		// GLTF_READER_H
