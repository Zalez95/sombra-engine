#ifndef GLTF_READER_H
#define GLTF_READER_H

#include <string>
#include <istream>
#include <nlohmann/json.hpp>
#include "../app/Entity.h"
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

		/** Struct LoadedData, it holds all the data loaded by the Reader */
		struct LoadedData
		{
			std::vector<std::unique_ptr<app::Entity>> entities;
			std::vector<std::unique_ptr<graphics::Mesh>> meshes;
			std::vector<std::unique_ptr<graphics::Camera>> cameras;
			std::vector<std::unique_ptr<graphics::Texture>> textures;
			std::vector<std::unique_ptr<graphics::Material>> materials;
		};

	private:	// Attributes
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
		 * @param	version the version number of the asset
		 * @param	revision the revision number of the asset
		 * @return	true if the JSON asset version is accepted, false
		 *			otherwise */
		static bool checkAssetVersion(
			const nlohmann::json& jsonAsset, int version, int revision
		);

		// void loadAccessor(const nlohmann::json& jsonObject) {};
		// void loadAnimation(const nlohmann::json& jsonObject) {};
		// void loadAsset(const nlohmann::json& jsonObject) {};
		// void loadBuffer(const nlohmann::json& jsonObject) {};
		// void loadBufferView(const nlohmann::json& jsonObject) {};
		// void loadExtension(const nlohmann::json& jsonObject) {};
		// void loadExtras(const nlohmann::json& jsonObject) {};
		// void loadImage(const nlohmann::json& jsonObject) {};
		static std::unique_ptr<graphics::Camera> parseCamera(const nlohmann::json& jsonCamera);
		static std::unique_ptr<graphics::Material> parseMaterial(const nlohmann::json& jsonMaterial);
		static std::unique_ptr<graphics::Mesh> parseMesh(const nlohmann::json& jsonMesh);
		static std::unique_ptr<app::Entity> parseNode(const nlohmann::json& jsonNode);
		// void loadSampler(const nlohmann::json& jsonObject) {};
		// void loadScene(const nlohmann::json& jsonObject) {};
		// void loadSkin(const nlohmann::json& jsonObject) {};
		// void loadTexture(const nlohmann::json& jsonObject) {};
		// void loadTextureInfo(const nlohmann::json& jsonObject) {};
	};

}

#endif		// GLTF_READER_H
