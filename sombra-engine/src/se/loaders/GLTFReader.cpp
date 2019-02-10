#include <sstream>
#include <nlohmann/json.hpp>
#include "se/loaders/GLTFReader.h"

namespace se::loaders {

	bool GLTFReader::load(std::istream& input)
	{
		bool ret = true;

		nlohmann::json jsonGLTF;
		input >> jsonGLTF;

		auto itAsset = jsonGLTF.find("asset");
		if ((itAsset != jsonGLTF.end())
			&& checkAssetVersion(*itAsset, FileFormat::sVersion, FileFormat::sRevision)
		) {
			LoadedData loadedData;

			auto itCameras = jsonGLTF.find("cameras");
			if (itCameras != jsonGLTF.end()) {
				for (const nlohmann::json& jsonCamera : *itCameras) {
					if (auto camera = parseCamera(jsonCamera)) {
						loadedData.cameras.push_back(std::move(camera));
					}
				}
			}

			auto itMaterials = jsonGLTF.find("materials");
			if (itMaterials != jsonGLTF.end()) {
				for (const nlohmann::json& jsonMaterial : *itMaterials) {
					if (auto material = parseMaterial(jsonMaterial)) {
						loadedData.materials.push_back(std::move(material));
					}
				}
			}

			auto itMeshes = jsonGLTF.find("meshes");
			if (itMeshes != jsonGLTF.end()) {
				for (const nlohmann::json& jsonMesh : *itMeshes) {
					if (auto mesh = parseMesh(jsonMesh)) {
						loadedData.meshes.push_back(std::move(mesh));
					}
				}
			}

			auto itNodes = jsonGLTF.find("nodes");
			if (itNodes != jsonGLTF.end()) {
				for (const nlohmann::json& jsonNode : *itNodes) {
					if (auto entity = parseNode(jsonNode)) {
						loadedData.entities.push_back(std::move(entity));
					}
				}
			}
		}

		return ret;
	}


	bool GLTFReader::checkAssetVersion(const nlohmann::json& jsonAsset, int version, int revision)
	{
		bool ret = false;

		auto itVersion = jsonAsset.find("version");
		if (itVersion != jsonAsset.end()) {
			int assetVersion, assetRevision;
			char dot;
			std::istringstream(itVersion->get<std::string>()) >> assetVersion >> dot >> assetRevision;
			if (dot == '.') {
				if (assetVersion < version) {
					ret = true;
				}
				else if ((assetVersion == version) && (assetRevision <= revision)) {
					ret = true;
				}
			}
		}

		return ret;
	}


	std::unique_ptr<graphics::Camera> GLTFReader::parseCamera(const nlohmann::json& jsonCamera)
	{
		std::unique_ptr<graphics::Camera> camera;

		auto itType = jsonCamera.find("name");
		auto itPerspective = jsonCamera.find("perspective");
		auto itOrthographic = jsonCamera.find("orthographic");

		if (itType != jsonCamera.end()) {
			if ((itType->get<std::string>() == "perspective") && (itPerspective != jsonCamera.end())) {
				auto itAspectRatio	= itPerspective->find("aspectRatio");
				auto itYFov			= itPerspective->find("yfov");
				auto itZFar			= itPerspective->find("zfar");
				auto itZNear		= itPerspective->find("znear");
				if ((itAspectRatio != jsonCamera.end()) && (itYFov != jsonCamera.end())
					&& (itZFar != jsonCamera.end()) && (itZNear != jsonCamera.end())
				) {
					camera = std::make_unique<graphics::Camera>();
					camera->setPerspectiveProjectionMatrix(*itYFov, *itAspectRatio, *itZNear, *itZFar);
				}
			}
			else if ((itType->get<std::string>() == "orthographic") && (itOrthographic != jsonCamera.end())) {
				auto itXMag			= itOrthographic->find("xmag");
				auto itYMag			= itOrthographic->find("ymag");
				auto itZFar			= itOrthographic->find("zfar");
				auto itZNear		= itOrthographic->find("znear");
				if ((itXMag != jsonCamera.end()) && (itYMag != jsonCamera.end())
					&& (itZFar != jsonCamera.end()) && (itZNear != jsonCamera.end())
				) {
					camera = std::make_unique<graphics::Camera>();
					camera->setOrthographicProjectionMatrix(*itXMag, *itYMag, *itZNear, *itZFar);
				}
			}
		}

		return camera;
	}


	std::unique_ptr<graphics::Material> GLTFReader::parseMaterial(const nlohmann::json& jsonMaterial)
	{
		auto material = std::make_unique<graphics::Material>();

		auto itName = jsonMaterial.find("name");
		if (itName != jsonMaterial.end()) {
			material->name = *itName;
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
			if (itAlphaMode->get<std::string>() == "OPAQUE") {
			}
			else if (itAlphaMode->get<std::string>() == "MASK") {
			}
			else if (itAlphaMode->get<std::string>() == "BLEND") {
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

		return material;
	}


	std::unique_ptr<graphics::Mesh> parseMesh(const nlohmann::json& /*jsonMesh*/)
	{
		return nullptr;
	}


	std::unique_ptr<app::Entity> parseNode(const nlohmann::json& jsonNode)
	{
		std::unique_ptr<app::Entity> entity;

		auto itName = jsonNode.find("name");
		if (itName != jsonNode.end()) {
			entity = std::make_unique<app::Entity>(*itName);

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
		}

		return entity;
	}

}
