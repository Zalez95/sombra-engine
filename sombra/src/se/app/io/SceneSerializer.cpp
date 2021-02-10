#include <fstream>
#include <iomanip>
#include "GLMJSON.h"
#include "se/physics/RigidBody.h"
#include "se/collision/BoundingBox.h"
#include "se/collision/BoundingSphere.h"
#include "se/collision/Capsule.h"
#include "se/collision/TriangleCollider.h"
#include "se/app/io/SceneSerializer.h"
#include "se/app/TagComponent.h"
#include "se/app/ScriptComponent.h"
#include "se/app/MeshComponent.h"
#include "se/app/TerrainComponent.h"
#include "se/app/TransformsComponent.h"
#include "se/app/CameraComponent.h"
#include "se/app/SkinComponent.h"
#include "se/app/LightComponent.h"
#include "se/app/LightProbe.h"

using namespace se::physics;
using namespace se::collision;

namespace se::app
{

	inline Result deserializeEntity(
		const nlohmann::json& json, const std::unordered_map<std::size_t, Entity>& indexEntityMap,
		Entity& entity
	) {
		auto itEntity = json.find("entity");
		if (itEntity == json.end()) {
			return Result(false, "Missing entity property");
		}

		auto itEntity2 = indexEntityMap.find(itEntity->get<std::size_t>());
		if (itEntity2 == indexEntityMap.end()) {
			return Result(false, "Entity not found " + std::to_string(itEntity->get<std::size_t>()));
		}

		entity = itEntity2->second;

		return Result();
	}


	inline void serializeTag(const TagComponent& tag, nlohmann::json& json)
	{
		json["name"] = tag.getName();
	}


	inline Result deserializeTag(const nlohmann::json& json, TagComponent& tag)
	{
		auto itName = json.find("name");

		if (itName != json.end()) {
			tag.setName(itName->get<std::string>());
			return Result();
		}
		else {
			return Result("Missing name property");
		}
	}


	inline void serializeTransforms(const TransformsComponent& transforms, nlohmann::json& json)
	{
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
	}


	inline Result deserializeTransforms(const nlohmann::json& json, TransformsComponent& transforms)
	{
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

		return Result();
	}


	inline void serializeCamera(const CameraComponent& camera, nlohmann::json& json)
	{
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
	}


	inline Result deserializeCamera(const nlohmann::json& json, CameraComponent& camera)
	{
		auto itIsOrtho = json.find("isOrthographic");
		if (itIsOrtho == json.end()) {
			return Result(false, "Missing isOrthographic property");
		}

		if (*itIsOrtho) {
			auto itLeft = json.find("left"), itRight = json.find("right"), itBottom = json.find("bottom"), itTop = json.find("top"),
				itZNear = json.find("zNear"), itZFar = json.find("zFar");

			if ((itLeft == json.end()) || (itRight == json.end()) || (itBottom == json.end()) || (itTop == json.end())
				|| (itZNear == json.end()) || (itZFar == json.end())
			) {
				return Result(false, "Missing orthographic properties");
			}

			camera.setOrthographicProjection(*itLeft, *itRight, *itBottom, *itTop, *itZNear, *itZFar);
		}
		else {
			auto itFovy = json.find("fovy"), itAspectRatio = json.find("aspectRatio"), itZNear = json.find("zNear"), itZFar = json.find("zFar");

			if ((itFovy == json.end()) || (itAspectRatio == json.end()) || (itZNear == json.end()) || (itZFar == json.end())) {
				return Result(false, "Missing projection properties");
			}

			camera.setPerspectiveProjection(*itFovy, *itAspectRatio, *itZNear, *itZFar);
		}

		return Result();
	}


	inline void serializeRigidBody(const RigidBody& rigidBody, nlohmann::json& json)
	{
		json["invertedMass"] = rigidBody.getConfig().invertedMass;
		json["invertedInertiaTensor"] = toJson(rigidBody.getConfig().invertedInertiaTensor);
		json["linearDrag"] = rigidBody.getConfig().linearDrag;
		json["angularDrag"] = rigidBody.getConfig().angularDrag;
		json["frictionCoefficient"] = rigidBody.getConfig().frictionCoefficient;
		json["sleepMotion"] = rigidBody.getConfig().sleepMotion;
	}


	inline Result deserializeRigidBody(const nlohmann::json& json, RigidBody& rigidBody)
	{
		auto itInvertedMass = json.find("invertedMass"), itInvertedInertiaTensor = json.find("invertedInertiaTensor"),
			itLinearDrag = json.find("linearDrag"), itAngularDrag = json.find("angularDrag"),
			itFrictionCoefficient = json.find("frictionCoefficient"), itSleepMotion = json.find("sleepMotion");

		if ((itInvertedMass == json.end()) || (itInvertedInertiaTensor == json.end())
			|| (itLinearDrag == json.end()) || (itAngularDrag == json.end())
			|| (itFrictionCoefficient == json.end()) || (itSleepMotion == json.end())
		) {
			return Result(false, "Missing properties");
		}

		rigidBody.getConfig().invertedMass = *itInvertedMass;
		if (!toMat3(*itInvertedInertiaTensor, rigidBody.getConfig().invertedInertiaTensor)) {
			return Result(false, "Failed to parse the invertedInertiaTensor");
		}

		rigidBody.getConfig().linearDrag = *itLinearDrag;
		rigidBody.getConfig().angularDrag = *itAngularDrag;
		rigidBody.getConfig().frictionCoefficient = *itFrictionCoefficient;
		rigidBody.getConfig().sleepMotion = *itSleepMotion;

		return Result();
	}


	inline void serializeCollider(const Collider& collider, nlohmann::json& json)
	{
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
	}


	inline Result deserializeCollider(const nlohmann::json& json, std::unique_ptr<Collider>& collider)
	{
		auto itType = json.find("type");
		if (itType == json.end()) {
			return Result(false, "Missing type property");
		}

		if (*itType == "BoundingBox") {
			auto itLengths = json.find("lengths");
			if (itLengths == json.end()) {
				return Result(false, "Missing BoundingBox lengths property");
			}

			glm::vec3 lengths;
			if (!toVec3(*itLengths, lengths)) {
				return Result(false, "Failed to parse BoundingBox lengths property");
			}

			collider = std::make_unique<BoundingBox>(lengths);
		}
		else if (*itType == "BoundingSphere") {
			auto itRadius = json.find("radius");
			if (itRadius == json.end()) {
				return Result(false, "Missing BoundingSphere radius property");
			}

			collider = std::make_unique<BoundingSphere>(itRadius->get<float>());
		}
		else if (*itType == "Capsule") {
			auto itRadius = json.find("radius");
			if (itRadius == json.end()) {
				return Result(false, "Missing Capsule radius property");
			}
			auto itHeight = json.find("height");
			if (itHeight == json.end()) {
				return Result(false, "Missing Capsule height property");
			}

			collider = std::make_unique<Capsule>(itRadius->get<float>(), itHeight->get<float>());
		}
		else if (*itType == "TriangleCollider") {
			auto itVertices = json.find("localVertices");
			if (itVertices == json.end()) {
				return Result(false, "Missing TriangleCollider localVertices property");
			}

			if (itVertices->size() < 3) {
				return Result(false, "Not enough localVertices");
			}

			std::array<glm::vec3, 3> vertices;
			for (std::size_t i = 0; i < 3; ++i) {
				if (!toVec3((*itVertices)[i], vertices[i])) {
					return Result(false, "Failed to parse the localVertices property");
				}
			}

			collider = std::make_unique<TriangleCollider>(vertices);
		}
		else {
			return Result(false, "Wrong type: " + itType->get<std::string>());
		}

		return Result();
	}


	Result SceneSerializer::serialize(const std::string& path, const Scene& scene)
	{
		nlohmann::json outputJson;
		outputJson["numEntities"] = scene.entities.size();

		auto tagsVJson = nlohmann::json::array();
		auto transformsVJson = nlohmann::json::array();
		auto camerasVJson = nlohmann::json::array();
		auto collidersVJson = nlohmann::json::array();
		auto rigidBodiesVJson = nlohmann::json::array();

		std::unordered_map<Entity, std::size_t> entityIndexMap;
		entityIndexMap.reserve(mEntityDatabase.getMaxEntities());
		for (std::size_t i = 0; i < scene.entities.size(); ++i) {
			entityIndexMap.emplace(scene.entities[i], i);
		}

		for (Entity entity : scene.entities) {
			std::size_t index = entityIndexMap[entity];
			auto [tag, transforms, camera, rigidBody, collider] = mEntityDatabase.getComponents<
				TagComponent, TransformsComponent, CameraComponent, RigidBody, Collider
			>(entity);

			if (tag) {
				nlohmann::json tagJson;
				tagJson["entity"] = index;
				serializeTag(*tag, tagJson);
				tagsVJson.push_back(tagJson);
			}
			if (transforms) {
				nlohmann::json transformsJson;
				transformsJson["entity"] = index;
				serializeTransforms(*transforms, transformsJson);
				transformsVJson.push_back(transformsJson);
			}
			if (camera) {
				nlohmann::json cameraJson;
				cameraJson["entity"] = index;
				serializeCamera(*camera, cameraJson);
				camerasVJson.push_back(cameraJson);
			}
			if (rigidBody) {
				nlohmann::json rigidBodyJson;
				rigidBodyJson["entity"] = index;
				serializeRigidBody(*rigidBody, rigidBodyJson);
				rigidBodiesVJson.push_back(rigidBodyJson);
			}
			if (collider) {
				nlohmann::json colliderJson;
				colliderJson["entity"] = index;
				serializeCollider(*collider, colliderJson);
				collidersVJson.push_back(colliderJson);
			}
		}

		if (!tagsVJson.empty()) {
			outputJson["tags"] = tagsVJson;
		}
		if (!transformsVJson.empty()) {
			outputJson["transforms"] = transformsVJson;
		}
		if (!camerasVJson.empty()) {
			outputJson["cameras"] = camerasVJson;
		}
		if (!rigidBodiesVJson.empty()) {
			outputJson["rigidBodies"] = rigidBodiesVJson;
		}
		if (!collidersVJson.empty()) {
			outputJson["colliders"] = collidersVJson;
		}

		std::ofstream outputstream(path);
		if (!outputstream.good()) {
			return Result(false, "Can't open the file");
		}

		outputstream << std::setw(4) << outputJson;
		if (!outputstream.good()) {
			return Result(false, "Failed to write to the file");
		}

		return Result();
	}


	Result SceneSerializer::deserialize(const std::string& path, Scene& output)
	{
		std::ifstream inputstream(path);
		if (!inputstream.good()) {
			return Result(false, "Can't open the GLTF file");
		}

		nlohmann::json inputJson;
		try {
			inputstream >> inputJson;
		}
		catch(const nlohmann::json::parse_error& e) {
			return Result(false, "Failed to parse the JSON file: " + std::string(e.what()));
		}

		auto itNumEntities = inputJson.find("numEntities");
		if (itNumEntities == inputJson.end()) {
			return Result(false, "Missing numEntities property");
		}

		std::size_t numEntities = *itNumEntities;
		output.entities.reserve(numEntities);
		std::unordered_map<std::size_t, Entity> indexEntityMap;
		indexEntityMap.reserve(numEntities);
		for (std::size_t i = 0; i < numEntities; ++i) {
			Entity entity = mEntityDatabase.addEntity();
			if (entity != kNullEntity) {
				output.entities.push_back(entity);
				indexEntityMap.emplace(i, entity);
			}
			else {
				return Result(false, "Can't add more entities");
			}
		}

		auto itTags = inputJson.find("tags");
		if (itTags != inputJson.end()) {
			for (std::size_t i = 0; i < itTags->size(); ++i) {
				auto& tagJson = (*itTags)[i];

				Entity entity = kNullEntity;
				auto result = deserializeEntity(tagJson, indexEntityMap, entity);
				if (!result) {
					return Result(false, "Failed to deserialize tag " + std::to_string(i) + ": " + result.description());
				}

				auto tag = mEntityDatabase.emplaceComponent<TagComponent>(entity);
				result = deserializeTag(tagJson, *tag);
				if (!result) {
					return Result(false, "Failed to deserialize tag " + std::to_string(i) + ": " + result.description());
				}
			}
		}

		auto itTransforms = inputJson.find("transforms");
		if (itTransforms != inputJson.end()) {
			for (std::size_t i = 0; i < itTransforms->size(); ++i) {
				auto& transformsJson = (*itTransforms)[i];

				Entity entity = kNullEntity;
				auto result = deserializeEntity(transformsJson, indexEntityMap, entity);
				if (!result) {
					return Result(false, "Failed to deserialize transforms " + std::to_string(i) + ": " + result.description());
				}

				auto transforms = mEntityDatabase.emplaceComponent<TransformsComponent>(entity);
				result = deserializeTransforms(transformsJson, *transforms);
				if (!result) {
					return Result(false, "Failed to deserialize transforms " + std::to_string(i) + ": " + result.description());
				}
			}
		}

		/*mEntityDatabase->addComponentTable<SkinComponent>(kMaxEntities);*/

		auto itCameras = inputJson.find("cameras");
		if (itCameras != inputJson.end()) {
			for (std::size_t i = 0; i < itCameras->size(); ++i) {
				auto& cameraJson = (*itCameras)[i];

				Entity entity = kNullEntity;
				auto result = deserializeEntity(cameraJson, indexEntityMap, entity);
				if (!result) {
					return Result(false, "Failed to deserialize camera " + std::to_string(i) + ": " + result.description());
				}

				auto camera = mEntityDatabase.emplaceComponent<CameraComponent>(entity);
				result = deserializeCamera(cameraJson, *camera);
				if (!result) {
					return Result(false, "Failed to deserialize camera " + std::to_string(i) + ": " + result.description());
				}
			}
		}

		/*mEntityDatabase->addComponentTable<LightComponent>(kMaxEntities);
		mEntityDatabase->addComponentTable<LightProbe>(kMaxLightProbes);
		mEntityDatabase->addComponentTable<MeshComponent>(kMaxEntities);
		mEntityDatabase->addComponentTable<TerrainComponent>(kMaxTerrains);*/

		auto itRigidBodies = inputJson.find("rigidBodies");
		if (itRigidBodies != inputJson.end()) {
			for (std::size_t i = 0; i < itRigidBodies->size(); ++i) {
				auto& rigidBodyJson = (*itRigidBodies)[i];

				Entity entity = kNullEntity;
				auto result = deserializeEntity(rigidBodyJson, indexEntityMap, entity);
				if (!result) {
					return Result(false, "Failed to deserialize RigidBody " + std::to_string(i) + ": " + result.description());
				}

				auto rigidBody = mEntityDatabase.emplaceComponent<RigidBody>(entity);
				result = deserializeRigidBody(rigidBodyJson, *rigidBody);
				if (!result) {
					return Result(false, "Failed to deserialize RigidBody " + std::to_string(i) + ": " + result.description());
				}
			}
		}

		auto itColliders = inputJson.find("colliders");
		if (itColliders != inputJson.end()) {
			for (std::size_t i = 0; i < itColliders->size(); ++i) {
				auto& colliderJson = (*itColliders)[i];

				Entity entity = kNullEntity;
				auto result = deserializeEntity(colliderJson, indexEntityMap, entity);
				if (!result) {
					return Result(false, "Failed to deserialize collider " + std::to_string(i) + ": " + result.description());
				}

				std::unique_ptr<Collider> collider;
				result = deserializeCollider(colliderJson, collider);
				if (!result) {
					return Result(false, "Failed to deserialize collider " + std::to_string(i) + ": " + result.description());
				}

				mEntityDatabase.addComponent<Collider>(entity, std::move(collider));
			}
		}

		/*mEntityDatabase->addComponentTable<ScriptComponent, true>(kMaxEntities);
		mEntityDatabase->addComponentTable<animation::AnimationNode*>(kMaxEntities);
		mEntityDatabase->addComponentTable<audio::Source>(kMaxEntities);*/

		return Result();
	}

}
