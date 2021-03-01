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

using namespace se::utils;
using namespace se::physics;
using namespace se::collision;
using namespace se::animation;

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


	inline void serializeAnimationNode(
		const AnimationNode& node, const std::unordered_map<const AnimationNode*, std::size_t>& nodeIndices,
		nlohmann::json& json
	) {
		auto& nodeData = node.getData();

		json["name"] = nodeData.name.data();

		nlohmann::json nodeLocalJson;
		nodeLocalJson["position"] = toJson(nodeData.localTransforms.position);
		nodeLocalJson["orientation"] = toJson(nodeData.localTransforms.orientation);
		nodeLocalJson["scale"] = toJson(nodeData.localTransforms.scale);
		json["localTransforms"] = nodeLocalJson;

		nlohmann::json nodeWorldJson;
		nodeWorldJson["position"] = toJson(nodeData.worldTransforms.position);
		nodeWorldJson["orientation"] = toJson(nodeData.worldTransforms.orientation);
		nodeWorldJson["scale"] = toJson(nodeData.worldTransforms.scale);
		json["worldTransforms"] = nodeWorldJson;

		json["worldMatrix"] = toJson(nodeData.worldMatrix);

		auto childrenJson = nlohmann::json::array();
		for (auto it = node.cbegin<Traversal::Children>(); it != node.cend<Traversal::Children>(); ++it) {
			childrenJson.push_back(nodeIndices.find(&(*it))->second);
		}
		json["children"] = std::move(childrenJson);
	}


	inline Result deserializeAnimationNode(
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
				toVec3(*itPosition, nodeData.localTransforms.position);
			}
			auto itOrientation = itLocal->find("orientation");
			if (itOrientation != itLocal->end()) {
				toQuat(*itOrientation, nodeData.localTransforms.orientation);
			}
			auto itScale = itLocal->find("scale");
			if (itScale != itLocal->end()) {
				toVec3(*itScale, nodeData.localTransforms.scale);
			}
		}
		else {
			return Result(false, "Missing local transforms");
		}

		auto itWorld = json.find("worldTransforms");
		if (itWorld != json.end()) {
			auto itPosition = itWorld->find("position");
			if (itPosition != itWorld->end()) {
				toVec3(*itPosition, nodeData.worldTransforms.position);
			}
			auto itOrientation = itWorld->find("orientation");
			if (itOrientation != itWorld->end()) {
				toQuat(*itOrientation, nodeData.worldTransforms.orientation);
			}
			auto itScale = itWorld->find("scale");
			if (itScale != itWorld->end()) {
				toVec3(*itScale, nodeData.worldTransforms.scale);
			}
		}
		else {
			return Result(false, "Missing world transforms");
		}

		auto itWorldMatrix = json.find("worldMatrix");
		if (itWorldMatrix != json.end()) {
			toMat4(*itWorldMatrix, nodeData.worldMatrix);
		}
		else {
			return Result(false, "Missing world matrix");
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


	Result SceneSerializer::serialize(const std::string& path, const Scene& scene)
	{
		nlohmann::json outputJson;

		// Scene nodes
		std::size_t numNodes = 0;
		std::unordered_map<const AnimationNode*, std::size_t> nodeIndices;
		for (auto itNode = scene.rootNode.cbegin(); itNode != scene.rootNode.cend(); ++itNode) {
			nodeIndices.emplace(&(*itNode), numNodes++);
		}

		auto nodesVJson = nlohmann::json::array();
		for (auto itNode = scene.rootNode.cbegin(); itNode != scene.rootNode.cend(); ++itNode) {
			nlohmann::json nodeJson;
			serializeAnimationNode(*itNode, nodeIndices, nodeJson);
			nodesVJson.push_back(nodeJson);
		}

		if (!nodesVJson.empty()) {
			outputJson["nodes"] = std::move(nodesVJson);
		}

		// Entities
		outputJson["numEntities"] = scene.entities.size();

		std::unordered_map<Entity, std::size_t> entityIndexMap;
		entityIndexMap.reserve(mEntityDatabase.getMaxEntities());
		for (std::size_t i = 0; i < scene.entities.size(); ++i) {
			entityIndexMap.emplace(scene.entities[i], i);
		}

		// Components
		auto tagsVJson = nlohmann::json::array();
		auto transformsVJson = nlohmann::json::array();
		auto camerasVJson = nlohmann::json::array();
		auto collidersVJson = nlohmann::json::array();
		auto rigidBodiesVJson = nlohmann::json::array();

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
			outputJson["tags"] = std::move(tagsVJson);
		}
		if (!transformsVJson.empty()) {
			outputJson["transforms"] = std::move(transformsVJson);
		}
		if (!camerasVJson.empty()) {
			outputJson["cameras"] = std::move(camerasVJson);
		}
		if (!rigidBodiesVJson.empty()) {
			outputJson["rigidBodies"] = std::move(rigidBodiesVJson);
		}
		if (!collidersVJson.empty()) {
			outputJson["colliders"] = std::move(collidersVJson);
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

		// Scene nodes
		std::vector<const AnimationNode*> nodePointers;
		auto itNodes = inputJson.find("nodes");
		if (itNodes != inputJson.end()) {
			nodePointers.reserve(itNodes->size());

			std::vector<std::unique_ptr<AnimationNode>> nodes;
			nodes.reserve(itNodes->size());

			std::unordered_map<AnimationNode*, std::vector<std::size_t>> childrenIndicesMap;
			childrenIndicesMap.reserve(itNodes->size());

			for (std::size_t i = 0; i < itNodes->size(); ++i) {
				auto& node = nodes.emplace_back(std::make_unique<AnimationNode>());
				auto& childrenIndices = childrenIndicesMap[node.get()];

				auto result = deserializeAnimationNode((*itNodes)[i], *node, childrenIndices);
				if (!result) {
					return Result(false, "Failed to parse the node " + std::to_string(i) + ": " + result.description());
				}

				nodePointers.push_back(node.get());
			}

			for (auto& [node, childrenIndices] : childrenIndicesMap) {
				for (auto childIndex : childrenIndices) {
					node->insert(node->cend(), std::move(nodes[childIndex]));
				}
			}

			for (auto& node : nodes) {
				if (node) {
					output.rootNode.insert(output.rootNode.cend(), std::move(node));
				}
			}
		}

		// Entities
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

		// Components
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
