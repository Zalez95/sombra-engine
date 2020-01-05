#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <AudioFile.h>

#include <se/window/KeyCodes.h>

#include <se/app/Image.h>
#include <se/app/RawMesh.h>
#include <se/app/InputManager.h>
#include <se/app/GraphicsManager.h>
#include <se/app/PhysicsManager.h>
#include <se/app/CollisionManager.h>
#include <se/app/AnimationManager.h>
#include <se/app/AudioManager.h>
#include <se/app/loaders/MeshLoader.h>
#include <se/app/loaders/ImageReader.h>
#include <se/app/loaders/FontReader.h>
#include <se/app/loaders/TerrainLoader.h>
#include <se/app/loaders/SceneReader.h>

#include <se/graphics/GraphicsSystem.h>
#include <se/graphics/3D/Camera.h>
#include <se/graphics/3D/Lights.h>
#include <se/graphics/3D/Mesh.h>
#include <se/graphics/3D/Material.h>
#include <se/graphics/3D/Renderable3D.h>
#include <se/graphics/text/Font.h>
#include <se/graphics/text/RenderableText.h>

#include <se/collision/BoundingBox.h>
#include <se/collision/BoundingSphere.h>
#include <se/collision/ConvexPolyhedron.h>
#include <se/collision/CompositeCollider.h>
#include <se/collision/HalfEdgeMeshExt.h>
#include <se/collision/QuickHull.h>
#include <se/collision/HACD.h>

#include <se/physics/PhysicsEngine.h>
#include <se/physics/RigidBody.h>
#include <se/physics/forces/Gravity.h>
#include <se/physics/constraints/DistanceConstraint.h>

#include <se/audio/Buffer.h>
#include <se/audio/Source.h>

#include <se/utils/Log.h>
#include <se/utils/FileReader.h>

#include "game/Game.h"

namespace game {

	se::app::RawMesh createRawMesh(
		const se::collision::HalfEdgeMesh& heMesh,
		const se::collision::ContiguousVector<glm::vec3>& normals
	) {
		se::app::RawMesh rawMesh("heMeshTriangles");

		// The faces must be triangles
		se::collision::HalfEdgeMesh heMeshTriangles = se::collision::triangulateFaces(heMesh);

		rawMesh.positions.reserve(heMeshTriangles.vertices.size());
		rawMesh.normals.reserve(heMeshTriangles.vertices.size());
		rawMesh.faceIndices.reserve(3 * heMeshTriangles.faces.size());

		std::map<int, int> vertexMap;
		for (auto itVertex = heMeshTriangles.vertices.begin(); itVertex != heMeshTriangles.vertices.end(); ++itVertex) {
			glm::vec3 normal = se::collision::calculateVertexNormal(heMesh, normals, itVertex.getIndex());

			glm::vec3 c1 = glm::cross(normal, glm::vec3(0.0f, 0.0f, 1.0f));
			glm::vec3 c2 = glm::cross(normal, glm::vec3(0.0f, 1.0f, 0.0f));
			glm::vec3 tangent = (glm::length(c1) > glm::length(c2))? c1 : c2;

			rawMesh.positions.push_back(itVertex->location);
			rawMesh.normals.push_back(normal);
			rawMesh.tangents.push_back(tangent);
			vertexMap.emplace(itVertex.getIndex(), rawMesh.positions.size() - 1);
		}

		for (auto itFace = heMeshTriangles.faces.begin(); itFace != heMeshTriangles.faces.end(); ++itFace) {
			se::utils::FixedVector<int, 3> faceIndices;
			se::collision::getFaceIndices(heMeshTriangles, itFace.getIndex(), std::back_inserter(faceIndices));
			for (int iVertex : faceIndices) {
				rawMesh.faceIndices.push_back(vertexMap[iVertex]);
			}
		}

		return rawMesh;
	}


	se::collision::HalfEdgeMesh createTestTube1()
	{
		se::collision::HalfEdgeMesh meshData;
		std::array<int, 48> vertexIndices = {
			se::collision::addVertex(meshData, { -0.000000014f, 0.499999761f, -1.0f }),
			se::collision::addVertex(meshData, { -0.000000014f, 0.499999761f, 1.0f }),
			se::collision::addVertex(meshData, { 0.249999970f, 0.433012485f, -1.0f }),
			se::collision::addVertex(meshData, { 0.249999970f, 0.433012485f, 1.0f }),
			se::collision::addVertex(meshData, { 0.433012694f, 0.249999791f, -1.0f }),
			se::collision::addVertex(meshData, { 0.433012694f, 0.249999791f, 1.0f }),
			se::collision::addVertex(meshData, { 0.5f, -0.000000210f, -1.0f }),
			se::collision::addVertex(meshData, { 0.5f, -0.000000210f, 1.0f }),
			se::collision::addVertex(meshData, { 0.433012694f, -0.250000208f, -1.0f }),
			se::collision::addVertex(meshData, { 0.433012694f, -0.250000208f, 1.0f }),
			se::collision::addVertex(meshData, { 0.250000029f, -0.433012902f, -1.0f }),
			se::collision::addVertex(meshData, { 0.250000029f, -0.433012902f, 1.0f }),
			se::collision::addVertex(meshData, { 0.00000006f, -0.500000178f, -1.0f }),
			se::collision::addVertex(meshData, { 0.00000006f, -0.500000178f, 1.0f }),
			se::collision::addVertex(meshData, { -0.249999910f, -0.433012962f, -1.0f }),
			se::collision::addVertex(meshData, { -0.249999910f, -0.433012962f, 1.0f }),
			se::collision::addVertex(meshData, { -0.433012634f, -0.250000357f, -1.0f }),
			se::collision::addVertex(meshData, { -0.433012634f, -0.250000357f, 1.0f }),
			se::collision::addVertex(meshData, { -0.5f, -0.000000421f, -1.0f }),
			se::collision::addVertex(meshData, { -0.5f, -0.000000421f, 1.0f }),
			se::collision::addVertex(meshData, { -0.433012872f, 0.249999567f, -1.0f }),
			se::collision::addVertex(meshData, { -0.433012872f, 0.249999567f, 1.0f }),
			se::collision::addVertex(meshData, { -0.250000327f, 0.433012336f, -1.0f }),
			se::collision::addVertex(meshData, { -0.250000327f, 0.433012336f, 1.0f }),
			se::collision::addVertex(meshData, { 0.0f, 1.0f, -1.0f }),
			se::collision::addVertex(meshData, { 0.0f, 1.0f, 1.0f }),
			se::collision::addVertex(meshData, { 0.5f, 0.866025388f, -1.0f }),
			se::collision::addVertex(meshData, { 0.5f, 0.866025388f, 1.0f }),
			se::collision::addVertex(meshData, { 0.866025447f, 0.499999970f, -1.0f }),
			se::collision::addVertex(meshData, { 0.866025447f, 0.499999970f, 1.0f }),
			se::collision::addVertex(meshData, { 1.0f, -0.000000043f, -1.0f }),
			se::collision::addVertex(meshData, { 1.0f, -0.000000043f, 1.0f }),
			se::collision::addVertex(meshData, { 0.866025388f, -0.500000059f, -1.0f }),
			se::collision::addVertex(meshData, { 0.866025388f, -0.500000059f, 1.0f }),
			se::collision::addVertex(meshData, { 0.500000059f, -0.866025388f, -1.0f }),
			se::collision::addVertex(meshData, { 0.500000059f, -0.866025388f, 1.0f }),
			se::collision::addVertex(meshData, { 0.00000015f, -1.0f, -1.0f }),
			se::collision::addVertex(meshData, { 0.00000015f, -1.0f, 1.0f }),
			se::collision::addVertex(meshData, { -0.499999791f, -0.866025507f, -1.0f }),
			se::collision::addVertex(meshData, { -0.499999791f, -0.866025507f, 1.0f }),
			se::collision::addVertex(meshData, { -0.866025209f, -0.500000298f, -1.0f }),
			se::collision::addVertex(meshData, { -0.866025209f, -0.500000298f, 1.0f }),
			se::collision::addVertex(meshData, { -1.0f, -0.000000464f, -1.0f }),
			se::collision::addVertex(meshData, { -1.0f, -0.000000464f, 1.0f }),
			se::collision::addVertex(meshData, { -0.866025686f, 0.499999493f, -1.0f }),
			se::collision::addVertex(meshData, { -0.866025686f, 0.499999493f, 1.0f }),
			se::collision::addVertex(meshData, { -0.500000596f, 0.866025090f, -1.0f }),
			se::collision::addVertex(meshData, { -0.500000596f, 0.866025090f, 1.0f })
		};
		std::array<std::array<int, 4>, 48> faceIndices = {{
			{{ vertexIndices[0], vertexIndices[2], vertexIndices[3], vertexIndices[1] }},
			{{ vertexIndices[2], vertexIndices[4], vertexIndices[5], vertexIndices[3] }},
			{{ vertexIndices[4], vertexIndices[6], vertexIndices[7], vertexIndices[5] }},
			{{ vertexIndices[6], vertexIndices[8], vertexIndices[9], vertexIndices[7] }},
			{{ vertexIndices[8], vertexIndices[10], vertexIndices[11], vertexIndices[9] }},
			{{ vertexIndices[10], vertexIndices[12], vertexIndices[13], vertexIndices[11] }},
			{{ vertexIndices[12], vertexIndices[14], vertexIndices[15], vertexIndices[13] }},
			{{ vertexIndices[14], vertexIndices[16], vertexIndices[17], vertexIndices[15] }},
			{{ vertexIndices[16], vertexIndices[18], vertexIndices[19], vertexIndices[17] }},
			{{ vertexIndices[18], vertexIndices[20], vertexIndices[21], vertexIndices[19] }},
			{{ vertexIndices[20], vertexIndices[22], vertexIndices[23], vertexIndices[21] }},
			{{ vertexIndices[22], vertexIndices[0], vertexIndices[1], vertexIndices[23] }},
			{{ vertexIndices[24], vertexIndices[25], vertexIndices[27], vertexIndices[26] }},
			{{ vertexIndices[26], vertexIndices[27], vertexIndices[29], vertexIndices[28] }},
			{{ vertexIndices[28], vertexIndices[29], vertexIndices[31], vertexIndices[30] }},
			{{ vertexIndices[30], vertexIndices[31], vertexIndices[33], vertexIndices[32] }},
			{{ vertexIndices[32], vertexIndices[33], vertexIndices[35], vertexIndices[34] }},
			{{ vertexIndices[34], vertexIndices[35], vertexIndices[37], vertexIndices[36] }},
			{{ vertexIndices[36], vertexIndices[37], vertexIndices[39], vertexIndices[38] }},
			{{ vertexIndices[38], vertexIndices[39], vertexIndices[41], vertexIndices[40] }},
			{{ vertexIndices[40], vertexIndices[41], vertexIndices[43], vertexIndices[42] }},
			{{ vertexIndices[42], vertexIndices[43], vertexIndices[45], vertexIndices[44] }},
			{{ vertexIndices[44], vertexIndices[45], vertexIndices[47], vertexIndices[46] }},
			{{ vertexIndices[46], vertexIndices[47], vertexIndices[25], vertexIndices[24] }},
			{{ vertexIndices[13], vertexIndices[15], vertexIndices[39], vertexIndices[37] }},
			{{ vertexIndices[37], vertexIndices[35], vertexIndices[11], vertexIndices[13] }},
			{{ vertexIndices[35], vertexIndices[33], vertexIndices[9], vertexIndices[11] }},
			{{ vertexIndices[33], vertexIndices[31], vertexIndices[7], vertexIndices[9] }},
			{{ vertexIndices[31], vertexIndices[29], vertexIndices[5], vertexIndices[7] }},
			{{ vertexIndices[29], vertexIndices[27], vertexIndices[3], vertexIndices[5] }},
			{{ vertexIndices[27], vertexIndices[25], vertexIndices[1], vertexIndices[3] }},
			{{ vertexIndices[25], vertexIndices[47], vertexIndices[23], vertexIndices[1] }},
			{{ vertexIndices[47], vertexIndices[45], vertexIndices[21], vertexIndices[23] }},
			{{ vertexIndices[45], vertexIndices[43], vertexIndices[19], vertexIndices[21] }},
			{{ vertexIndices[43], vertexIndices[41], vertexIndices[17], vertexIndices[19] }},
			{{ vertexIndices[41], vertexIndices[39], vertexIndices[15], vertexIndices[17] }},
			{{ vertexIndices[20], vertexIndices[18], vertexIndices[42], vertexIndices[44] }},
			{{ vertexIndices[16], vertexIndices[40], vertexIndices[42], vertexIndices[18] }},
			{{ vertexIndices[14], vertexIndices[38], vertexIndices[40], vertexIndices[16] }},
			{{ vertexIndices[12], vertexIndices[36], vertexIndices[38], vertexIndices[14] }},
			{{ vertexIndices[10], vertexIndices[34], vertexIndices[36], vertexIndices[12] }},
			{{ vertexIndices[8], vertexIndices[32], vertexIndices[34], vertexIndices[10] }},
			{{ vertexIndices[6], vertexIndices[30], vertexIndices[32], vertexIndices[8] }},
			{{ vertexIndices[4], vertexIndices[28], vertexIndices[30], vertexIndices[6] }},
			{{ vertexIndices[2], vertexIndices[26], vertexIndices[28], vertexIndices[4] }},
			{{ vertexIndices[0], vertexIndices[24], vertexIndices[26], vertexIndices[2] }},
			{{ vertexIndices[22], vertexIndices[46], vertexIndices[24], vertexIndices[0] }},
			{{ vertexIndices[20], vertexIndices[44], vertexIndices[46], vertexIndices[22] }}
		}};

		for (const auto& face : faceIndices) {
			addFace(meshData, face.begin(), face.end());
		}

		return meshData;
	}

// Public functions
	Game::Game() : se::app::Application({ kTitle, kWidth, kHeight, false, false, false, false }, kUpdateTime) {}


	Game::~Game() {}


	void Game::start()
	{
		SOMBRA_INFO_LOG << mGraphicsSystem->getGLInfo();
		mGraphicsSystem->addLayer(&mLayer2D);

		/*********************************************************************
		 * GRAPHICS DATA
		 *********************************************************************/
		se::app::TerrainLoader terrainLoader(*mGraphicsManager, *mPhysicsManager, *mCollisionManager);
		se::collision::QuickHull qh(0.0001f);
		se::collision::HACD hacd(0.002f, 0.0002f);

		se::app::Image heightMap1;
		std::shared_ptr<se::graphics::Mesh> cubeMesh = nullptr, planeMesh = nullptr, domeMesh = nullptr;
		std::shared_ptr<se::graphics::Texture> logoTexture = nullptr, chessTexture = nullptr;
		std::unique_ptr<se::graphics::Camera> camera1 = nullptr;
		std::unique_ptr<se::graphics::PointLight> pointLight1 = nullptr, pointLight2 = nullptr, pointLight3 = nullptr;
		std::shared_ptr<se::graphics::Font> arial = nullptr;
		std::unique_ptr<se::audio::Source> source1 = nullptr;
		se::app::Scenes loadedScenes;

		try {
			// Readers
			AudioFile<float> audioFile;
			se::app::FontReader fontReader;
			auto sceneReader = se::app::SceneReader::createSceneReader(se::app::SceneFileType::GLTF);

			// Meshes
			se::app::RawMesh cubeRawMesh("Cube");
			cubeRawMesh.positions = {
				{ 0.5f, 0.5f,-0.5f},
				{ 0.5f,-0.5f,-0.5f},
				{-0.5f,-0.5f,-0.5f},
				{-0.5f, 0.5f,-0.5f},
				{ 0.5f, 0.5f, 0.5f},
				{ 0.5f,-0.5f, 0.5f},
				{-0.5f,-0.5f, 0.5f},
				{-0.5f, 0.5f, 0.5f},
				{ 0.5f, 0.5f,-0.5f},
				{ 0.5f,-0.5f,-0.5f},
				{ 0.5f, 0.5f, 0.5f},
				{ 0.5f,-0.5f, 0.5f},
				{ 0.5f, 0.5f, 0.5f},
				{ 0.5f,-0.5f, 0.5f},
				{-0.5f,-0.5f, 0.5f},
				{-0.5f, 0.5f, 0.5f},
				{ 0.5f, 0.5f,-0.5f},
				{ 0.5f,-0.5f,-0.5f},
				{-0.5f, 0.5f,-0.5f},
				{-0.5f, 0.5f,-0.5f},
				{-0.5f,-0.5f,-0.5f},
				{-0.5f,-0.5f,-0.5f},
				{-0.5f,-0.5f, 0.5f},
				{-0.5f, 0.5f, 0.5f}
			};
			cubeRawMesh.texCoords = {
				{0.666467010f, 0.666466951f},
				{0.999800264f, 0.000199760f},
				{0.333533257f, 0.333133578f},
				{0.333533287f, 0.666466951f},
				{0.666467010f, 0.333533167f},
				{0.999800145f, 0.333133548f},
				{0.333533197f, 0.000199760f},
				{0.333533197f, 0.333533257f},
				{0.333133667f, 0.333533167f},
				{0.000199899f, 0.333533197f},
				{0.333133548f, 0.666466951f},
				{0.000199760f, 0.666466951f},
				{0.333133697f, 0.333133548f},
				{0.333133488f, 0.000199760f},
				{0.000199760f, 0.000199909f},
				{0.000199869f, 0.333133667f},
				{0.333133548f, 0.999800264f},
				{0.000199760f, 0.999800264f},
				{0.333133548f, 0.666866540f},
				{0.666467010f, 0.333133488f},
				{0.000199770f, 0.666866540f},
				{0.666866540f, 0.000199799f},
				{0.666866540f, 0.333133578f},
				{0.666466891f, 0.000199760f}
			};
			cubeRawMesh.faceIndices = {
				16, 20, 18,
				5, 21, 1,
				2, 23, 19,
				0, 7, 4,
				10, 9, 8,
				15, 13, 12,
				16, 17, 20,
				5, 22, 21,
				2, 6, 23,
				0, 3, 7,
				10, 11, 9,
				15, 14, 13
			};
			cubeRawMesh.normals = se::app::MeshLoader::calculateNormals(cubeRawMesh.positions, cubeRawMesh.faceIndices);
			cubeRawMesh.tangents = se::app::MeshLoader::calculateTangents(cubeRawMesh.positions, cubeRawMesh.texCoords, cubeRawMesh.faceIndices);
			cubeMesh = std::make_shared<se::graphics::Mesh>(se::app::MeshLoader::createGraphicsMesh(cubeRawMesh));

			se::app::RawMesh planeRawMesh("Plane");
			planeRawMesh.positions = { {-0.5f,-0.5f, 0.0f}, { 0.5f,-0.5f, 0.0f}, {-0.5f, 0.5f, 0.0f}, { 0.5f, 0.5f, 0.0f} };
			planeRawMesh.texCoords = { {0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f} };
			planeRawMesh.faceIndices = { 0, 1, 2, 1, 3, 2, };
			planeRawMesh.normals = se::app::MeshLoader::calculateNormals(planeRawMesh.positions, planeRawMesh.faceIndices);
			planeRawMesh.tangents = se::app::MeshLoader::calculateTangents(planeRawMesh.positions, planeRawMesh.texCoords, planeRawMesh.faceIndices);
			planeMesh = std::make_shared<se::graphics::Mesh>(se::app::MeshLoader::createGraphicsMesh(planeRawMesh));

			se::app::RawMesh domeRawMesh = se::app::MeshLoader::createDomeMesh("sky", 32, 16, kZFar / 2);
			domeMesh = std::make_shared<se::graphics::Mesh>( se::app::MeshLoader::createGraphicsMesh(domeRawMesh) );

			// GLTF scenes
			se::app::Result result = sceneReader->load("res/meshes/test.gltf", loadedScenes);
			if (!result) {
				throw std::runtime_error(result.description());
			}

			// Images
			se::app::Image logo1;
			result = se::app::ImageReader::read("res/images/logo.png", logo1);
			if (!result) {
				throw std::runtime_error(result.description());
			}

			result = se::app::ImageReader::read("res/images/terrain.png", heightMap1, 1);
			if (!result) {
				throw std::runtime_error(result.description());
			}

			// Textures
			logoTexture = std::make_shared<se::graphics::Texture>();
			logoTexture->setImage(
				logo1.pixels.get(), se::graphics::TypeId::UnsignedByte, se::graphics::ColorFormat::RGBA,
				logo1.width, logo1.height
			);

			float pixels[] = {
				0.0f, 0.0f, 0.0f,	1.0f, 1.0f, 1.0f,
				1.0f, 1.0f, 1.0f,	0.0f, 0.0f, 0.0f
			};
			chessTexture = std::make_shared<se::graphics::Texture>();
			chessTexture->setImage(pixels, se::graphics::TypeId::Float, se::graphics::ColorFormat::RGB, 2, 2);

			// Cameras
			camera1 = std::make_unique<se::graphics::Camera>();
			camera1->setPerspectiveProjectionMatrix(glm::radians(kFOV), kWidth / static_cast<float>(kHeight), kZNear, kZFar);

			// Lights
			pointLight1 = std::make_unique<se::graphics::PointLight>();
			pointLight1->name = "pointLight1";
			pointLight1->intensity = 5.0f;
			pointLight2 = std::make_unique<se::graphics::PointLight>();
			pointLight2->name = "pointLight2";
			pointLight2->color = glm::vec3(0.5f, 1.0f, 0.5f);
			pointLight2->intensity = 2.0f;
			pointLight3 = std::make_unique<se::graphics::PointLight>();
			pointLight3->name = "pointLight3";
			pointLight3->color = glm::vec3(1.0f, 0.5f, 0.5f);
			pointLight3->intensity = 10.0f;

			// Fonts
			se::utils::FileReader fileReader3("res/fonts/arial.fnt");
			arial = std::move(fontReader.read(fileReader3));

			// Audio
			if (!audioFile.load("res/audio/bounce.wav")) {
				throw std::runtime_error("Error reading the audio file");
			}

			mBuffers.emplace_back(
				audioFile.samples[0].data(), audioFile.samples[0].size() * sizeof(float),
				se::audio::FormatId::MonoFloat, audioFile.getSampleRate()
			);
			source1 = std::make_unique<se::audio::Source>();
			source1->bind(mBuffers.back());
			source1->setLooping(true);
			source1->play();
		}
		catch (std::exception& e) {
			SOMBRA_ERROR_LOG << "Error: " << e.what();
			mState = AppState::Error;
			return;
		}

		// Forces
		mForces.push_back(new se::physics::Gravity({ 0.0f, -9.8f, 0.0f }));
		se::physics::Force* gravity = mForces.back();

		// RenderableTexts
		se::graphics::RenderableText renderableText1("First try rendering text", arial, 10, glm::vec2());

		// Renderable2Ds
		mRenderable2Ds.emplace_back(glm::vec2(0.75f, 0.75f), glm::vec2(0.15f, 0.2f), logoTexture);
		mLayer2D.addRenderable2D(&mRenderable2Ds.back());

		/*********************************************************************
		 * GAME DATA
		 *********************************************************************/
		// Player
		auto player = std::make_unique<se::app::Entity>("player");
		mPlayer = player.get();
		mEntities.emplace_back(std::move(player));

		mPlayer->position = glm::vec3(0.0f, 1.0f, 10.0f);
		mPlayer->orientation = glm::quat(glm::vec3(0.0f, glm::pi<float>(), 0.0f));

		se::physics::RigidBodyConfig config1(0.001f);
		config1.invertedMass = 1.0f / 40.0f;	// No inertia tensor so the player can't rotate due to collisions
		config1.linearDrag = 0.01f;
		config1.angularDrag = 0.01f;
		config1.frictionCoefficient = 1.16f;
		auto rigidBody1 = std::make_unique<se::physics::RigidBody>(config1, se::physics::RigidBodyData());
		auto collider1 = std::make_unique<se::collision::BoundingSphere>(0.5f);
		mCollisionManager->addEntity(mPlayer, std::move(collider1));
		mPhysicsManager->addEntity(mPlayer, std::move(rigidBody1));

		mGraphicsManager->addCameraEntity(mPlayer, std::move(camera1));
		mGraphicsManager->addLightEntity(mPlayer, std::move(pointLight1));
		mAudioManager->setListener(mPlayer);

		mInputManager->setMouseCommand(std::make_unique<MouseFPSControl>(*mWindowSystem, mPlayerInput, kMouseSpeed));
		mInputManager->addKeyCommand(SE_KEY_W, se::window::ButtonState::Pressed, std::make_unique<StartMoving>(InputTransforms::Direction::Front, mPlayerInput));
		mInputManager->addKeyCommand(SE_KEY_W, se::window::ButtonState::Released, std::make_unique<StopMoving>(InputTransforms::Direction::Front, mPlayerInput));
		mInputManager->addKeyCommand(SE_KEY_A, se::window::ButtonState::Pressed, std::make_unique<StartMoving>(InputTransforms::Direction::Left, mPlayerInput));
		mInputManager->addKeyCommand(SE_KEY_A, se::window::ButtonState::Released, std::make_unique<StopMoving>(InputTransforms::Direction::Left, mPlayerInput));
		mInputManager->addKeyCommand(SE_KEY_S, se::window::ButtonState::Pressed, std::make_unique<StartMoving>(InputTransforms::Direction::Back, mPlayerInput));
		mInputManager->addKeyCommand(SE_KEY_S, se::window::ButtonState::Released, std::make_unique<StopMoving>(InputTransforms::Direction::Back, mPlayerInput));
		mInputManager->addKeyCommand(SE_KEY_D, se::window::ButtonState::Pressed, std::make_unique<StartMoving>(InputTransforms::Direction::Right, mPlayerInput));
		mInputManager->addKeyCommand(SE_KEY_D, se::window::ButtonState::Released, std::make_unique<StopMoving>(InputTransforms::Direction::Right, mPlayerInput));
		mInputManager->addKeyCommand(SE_KEY_SPACE, se::window::ButtonState::Pressed, std::make_unique<StartMoving>(InputTransforms::Direction::Up, mPlayerInput));
		mInputManager->addKeyCommand(SE_KEY_SPACE, se::window::ButtonState::Released, std::make_unique<StopMoving>(InputTransforms::Direction::Up, mPlayerInput));
		mInputManager->addKeyCommand(SE_KEY_LEFT_CONTROL, se::window::ButtonState::Pressed, std::make_unique<StartMoving>(InputTransforms::Direction::Down, mPlayerInput));
		mInputManager->addKeyCommand(SE_KEY_LEFT_CONTROL, se::window::ButtonState::Released, std::make_unique<StopMoving>(InputTransforms::Direction::Down, mPlayerInput));
		mInputManager->addKeyCommand(SE_KEY_ESCAPE, se::window::ButtonState::Pressed, std::make_unique<CloseCommand>(*this));

		// Sky
		auto skyEntity = std::make_unique<se::app::Entity>("sky");
		auto renderable3D = std::make_unique<se::graphics::Renderable3D>(domeMesh, nullptr);
		mGraphicsManager->addSkyEntity(skyEntity.get(), std::move(renderable3D));
		mEntities.push_back(std::move(skyEntity));

		// Terrain
		std::vector<float> lodDistances{ 2000.0f, 1000.0f, 500.0f, 250.0f, 125.0f, 75.0f, 40.0f, 20.0f, 10.0f, 0.0f };
		mEntities.push_back( terrainLoader.createTerrain("terrain", 500.0f, 10.0f, heightMap1, lodDistances) );

		// Plane
		auto plane = std::make_unique<se::app::Entity>("plane");
		plane->position = glm::vec3(-15.0f, 1.0f, -5.0f);

		std::shared_ptr<se::graphics::Material> planeMaterial(new se::graphics::Material{
			"plane_material",
			se::graphics::PBRMetallicRoughness{ glm::vec4(1.0f), nullptr, 0.2f, 0.5f, nullptr },
			nullptr, 1.0f, nullptr, 1.0f, chessTexture, glm::vec3(1.0f), se::graphics::AlphaMode::Opaque, 0.5f, true
		});

		auto renderable3D1 = std::make_unique<se::graphics::Renderable3D>(planeMesh, planeMaterial);
		mGraphicsManager->addRenderableEntity(plane.get(), std::move(renderable3D1));

		mEntities.push_back(std::move(plane));

		// Fixed cubes
		glm::vec3 cubePositions[5] = {
			{ 2.0f, 5.0f, -10.0f },
			{ 0.0f, 7.0f, -10.0f },
			{ 0.0f, 5.0f, -8.0f },
			{ 0.0f, 5.0f, -10.0f },
			{ 10.0f, 5.0f, -10.0f }
		};
		glm::vec4 colors[5] = { { 1.0f, 0.2f, 0.2f, 1.0f }, { 0.2f, 1.0f, 0.2f, 1.0f }, { 0.2f, 0.2f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.2f, 0.2f, 0.1f, 1.0f } };
		se::physics::RigidBody *rb1 = nullptr, *rb2 = nullptr;
		for (std::size_t i = 0; i < 5; ++i) {
			auto cube = std::make_unique<se::app::Entity>("non-random-cube");
			cube->position = cubePositions[i];

			se::physics::RigidBodyConfig config2(20.0f, 2.0f / 5.0f * 10.0f * glm::pow(2.0f, 2.0f) * glm::mat3(1.0f), 0.001f);
			config2.linearDrag = 0.95f;
			config2.angularDrag = 0.95f;
			config2.frictionCoefficient = 0.5f;
			auto rigidBody2 = std::make_unique<se::physics::RigidBody>(config2, se::physics::RigidBodyData());
			if (i == 1) {
				rb1 = rigidBody2.get();
			}
			if (i == 2) {
				mAudioManager->addSource(cube.get(), std::move(source1));
			}
			if (i == 3) {
				rigidBody2->getData().angularVelocity = glm::vec3(0.0f, 10.0f, 0.0f);
				rb2 = rigidBody2.get();
			}
			if (i == 4) {
				cube->velocity += glm::vec3(-1, 0, 0);
			}
			auto collider2 = std::make_unique<se::collision::BoundingBox>(glm::vec3(1.0f, 1.0f, 1.0f));
			mCollisionManager->addEntity(cube.get(), std::move(collider2));
			mPhysicsManager->addEntity(cube.get(), std::move(rigidBody2));

			std::shared_ptr<se::graphics::Material> tmpMaterial(new se::graphics::Material{
				"tmp_material",
				se::graphics::PBRMetallicRoughness{ colors[i], nullptr, 0.2f, 0.5f, nullptr },
				nullptr, 1.0f, nullptr, 1.0f, nullptr, glm::vec3(0.0f), se::graphics::AlphaMode::Opaque, 0.5f, false
			});

			auto renderable3D2 = std::make_unique<se::graphics::Renderable3D>(cubeMesh, tmpMaterial);
			mGraphicsManager->addRenderableEntity(cube.get(), std::move(renderable3D2));

			mEntities.push_back(std::move(cube));
		}

		mConstraints.push_back(new se::physics::DistanceConstraint({ rb1, rb2 }));
		mPhysicsEngine->getConstraintManager().addConstraint(mConstraints.back());

		{
			std::shared_ptr<se::graphics::Material> redMaterial(new se::graphics::Material{
				"tmp_material",
				se::graphics::PBRMetallicRoughness{ { 1.0f, 0.0f, 0.0f, 1.0f }, nullptr, 0.2f, 0.5f, nullptr },
				nullptr, 1.0f, nullptr, 1.0f, nullptr, glm::vec3(0.0f), se::graphics::AlphaMode::Opaque, 0.5f, false
			});

			auto nonMovableCube = std::make_unique<se::app::Entity>("non-movable-cube");
			nonMovableCube->position = glm::vec3(-50.0f, 0.0f, -40.0f);
			nonMovableCube->scale = glm::vec3(10.0f, 1.0f, 10.0f);

			se::physics::RigidBodyConfig config2(0.001f);
			config2.frictionCoefficient = 0.75f;
			auto rigidBody2 = std::make_unique<se::physics::RigidBody>(config2, se::physics::RigidBodyData());
			auto collider2 = std::make_unique<se::collision::BoundingBox>(glm::vec3(1.0f));
			mCollisionManager->addEntity(nonMovableCube.get(), std::move(collider2));
			mPhysicsManager->addEntity(nonMovableCube.get(), std::move(rigidBody2));

			auto renderable3D2 = std::make_unique<se::graphics::Renderable3D>(cubeMesh, redMaterial);
			mGraphicsManager->addRenderableEntity(nonMovableCube.get(), std::move(renderable3D2));

			mEntities.push_back(std::move(nonMovableCube));

			auto gravityCube = std::make_unique<se::app::Entity>("gravity-cube");
			gravityCube->position = glm::vec3(-50.0f, 2.0f, -40.0f);

			se::physics::RigidBodyConfig config3(20.0f, 2.0f / 5.0f * 10.0f * glm::pow(2.0f, 2.0f) * glm::mat3(1.0f), 0.001f);
			config3.linearDrag = 0.95f;
			config3.angularDrag = 0.95f;
			config3.frictionCoefficient = 0.65f;
			auto rigidBody3 = std::make_unique<se::physics::RigidBody>(config3, se::physics::RigidBodyData());
			auto collider3 = std::make_unique<se::collision::BoundingBox>(glm::vec3(1.0f));

			mPhysicsEngine->getForceManager().addRBForce(rigidBody3.get(), gravity);

			mCollisionManager->addEntity(gravityCube.get(), std::move(collider3));
			mPhysicsManager->addEntity(gravityCube.get(), std::move(rigidBody3));

			auto renderable3D3 = std::make_unique<se::graphics::Renderable3D>(cubeMesh, redMaterial);
			mGraphicsManager->addRenderableEntity(gravityCube.get(), std::move(renderable3D3));

			mEntities.push_back(std::move(gravityCube));
		}

		// HACD Tube
		se::collision::HalfEdgeMesh tube = createTestTube1();
		glm::vec3 tubeCentroid = se::collision::calculateCentroid(tube);
		hacd.calculate(tube);
		for (const auto& [heMesh, normals] : hacd.getMeshes()) {
			glm::vec3 sliceCentroid = se::collision::calculateCentroid(heMesh);
			glm::vec3 displacement = sliceCentroid - tubeCentroid;
			if (glm::length(displacement) > 0.0f) {
				displacement = glm::normalize(displacement);
			}
			displacement *= 0.1f;

			auto tubeSlice = std::make_unique<se::app::Entity>("tube");
			tubeSlice->orientation = glm::normalize(glm::quat(-1, glm::vec3(1.0f, 0.0f, 0.0f)));
			tubeSlice->position = glm::vec3(0.0f, 2.0f, 75.0f) + displacement;

			std::shared_ptr<se::graphics::Material> tmpMaterial(new se::graphics::Material{
				"tmp_material",
				se::graphics::PBRMetallicRoughness{
					glm::vec4(glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f), 1.0f),
					nullptr, 0.2f, 0.5f, nullptr
				},
				nullptr, 1.0f, nullptr, 1.0f, nullptr, glm::vec3(0.0f), se::graphics::AlphaMode::Opaque, 0.5f, false
			});

			auto tmpRawMesh = createRawMesh(heMesh, normals);
			auto tmpGraphicsMesh = std::make_shared<se::graphics::Mesh>(se::app::MeshLoader::createGraphicsMesh(tmpRawMesh));
			auto renderable3D2 = std::make_unique<se::graphics::Renderable3D>(tmpGraphicsMesh, tmpMaterial);
			mGraphicsManager->addRenderableEntity(tubeSlice.get(), std::move(renderable3D2));

			mEntities.push_back(std::move(tubeSlice));
		}

		// Random cubes
		for (std::size_t i = 0; i < kNumCubes; ++i) {
			auto cube = std::make_unique<se::app::Entity>("random-cube");
			cube->position = glm::ballRand(50.0f);

			se::physics::RigidBodyConfig config2(10.0f, 2.0f / 5.0f * 10.0f * glm::pow(2.0f, 2.0f) * glm::mat3(1.0f), 0.001f);
			config2.linearDrag = 0.9f;
			config2.angularDrag = 0.9f;
			config2.frictionCoefficient = 0.5f;
			auto rigidBody2 = std::make_unique<se::physics::RigidBody>(config2, se::physics::RigidBodyData());
			auto collider2 = std::make_unique<se::collision::BoundingBox>(glm::vec3(1.0f, 1.0f, 1.0f));
			mCollisionManager->addEntity(cube.get(), std::move(collider2));
			mPhysicsManager->addEntity(cube.get(), std::move(rigidBody2));

			auto renderable3D2 = std::make_unique<se::graphics::Renderable3D>(cubeMesh, nullptr);
			mGraphicsManager->addRenderableEntity(cube.get(), std::move(renderable3D2));

			mEntities.push_back(std::move(cube));
		}

		// Lights
		std::shared_ptr<se::graphics::Material> lightMaterial(new se::graphics::Material{
			"light_material",
			se::graphics::PBRMetallicRoughness{ glm::vec4(1.0f), nullptr, 0.2f, 0.5f, nullptr },
			nullptr, 1.0f, nullptr, 1.0f, nullptr, glm::vec3(5.0f), se::graphics::AlphaMode::Opaque, 0.5f, true
		});

		auto eL2 = std::make_unique<se::app::Entity>("point-light2");
		eL2->position = glm::vec3(-3.0f, 1.0f, 5.0f);
		eL2->scale = glm::vec3(0.1f);
		mGraphicsManager->addLightEntity(eL2.get(), std::move(pointLight2));
		mGraphicsManager->addRenderableEntity(eL2.get(), std::make_unique<se::graphics::Renderable3D>(cubeMesh, lightMaterial));
		mEntities.push_back(std::move(eL2));

		auto eL3 = std::make_unique<se::app::Entity>("point-light3");
		eL3->position = glm::vec3(2.0f, 10.0f, 5.0f);
		eL3->scale = glm::vec3(0.1f);
		mGraphicsManager->addLightEntity(eL3.get(), std::move(pointLight3));
		mGraphicsManager->addRenderableEntity(eL3.get(), std::make_unique<se::graphics::Renderable3D>(cubeMesh, lightMaterial));
		mEntities.push_back(std::move(eL3));

		// GLTF Scene
		auto sceneEntity = std::make_unique<se::app::Entity>("Scene");
		mAnimationManager->addEntity(sceneEntity.get(), std::move(loadedScenes.scenes[0]->rootNode));
		mEntities.push_back( std::move(sceneEntity) );

		std::vector<std::shared_ptr<se::app::Skin>> sharedSkins;
		std::move(loadedScenes.skins.begin(), loadedScenes.skins.end(), std::back_inserter(sharedSkins));
		for (auto& e : loadedScenes.scenes[0]->entities) {
			if (e.animationNode) {
				auto entity = std::make_unique<se::app::Entity>(e.animationNode->getData().name);
				mAnimationManager->addEntity(entity.get(), e.animationNode);

				if (e.hasRenderable3Ds) {
					if (e.hasSkin) {
						for (std::size_t iRenderable3D : loadedScenes.renderable3DIndices[e.renderable3DsIndex]) {
							mGraphicsManager->addRenderableEntity(entity.get(), std::move(loadedScenes.renderable3Ds[iRenderable3D]), sharedSkins[e.skinIndex]);
						}
					}
					else {
						for (std::size_t iRenderable3D : loadedScenes.renderable3DIndices[e.renderable3DsIndex]) {
							mGraphicsManager->addRenderableEntity(entity.get(), std::move(loadedScenes.renderable3Ds[iRenderable3D]));
						}
					}
				}

				mEntities.push_back( std::move(entity) );
			}
		}

		Application::start();
	}


	void Game::stop()
	{
		if (mState == AppState::Stopped) { return; }

		Application::stop();

		for (se::physics::Force* force : mForces) {
			mPhysicsEngine->getForceManager().removeForce(force);
			delete force;
		}

		for (se::physics::Constraint* constraint : mConstraints) {
			mPhysicsEngine->getConstraintManager().removeConstraint(constraint);
			delete constraint;
		}

		for (se::animation::IAnimator* animator : mAnimators) {
			mAnimationSystem->removeAnimator(animator);
			delete animator;
		}

		for (EntityUPtr& entity : mEntities) {
			mGraphicsManager->removeEntity(entity.get());
			mPhysicsManager->removeEntity(entity.get());
			mCollisionManager->removeEntity(entity.get());
			mAnimationManager->removeEntity(entity.get());
			mAudioManager->removeEntity(entity.get());
		}
		mEntities.clear();

		mGraphicsSystem->removeLayer(&mLayer2D);
	}

// Private functions
	void Game::onUpdate(float deltaTime)
	{
		SOMBRA_DEBUG_LOG << "Init (" << deltaTime << ")";
		std::cout << deltaTime << "ms\r";

		glm::vec3 forward	= glm::vec3(0.0f, 0.0f, 1.0f) * mPlayer->orientation;
		glm::vec3 up		= glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 right		= glm::cross(forward, up);

		// Set the pitch and yaw
		if ((mPlayerInput.yaw != 0.0f) || (mPlayerInput.pitch != 0.0f)) {
			float yaw = mPlayerInput.yaw * deltaTime;
			float pitch = mPlayerInput.pitch * deltaTime;
			mPlayerInput.yaw = mPlayerInput.pitch = 0.0f;

			// Clamp the pitch
			float currentPitch = std::asin(forward.y);
			float nextPitch = currentPitch + pitch;
			nextPitch = std::clamp(nextPitch, -glm::half_pi<float>() + kPitchLimit, glm::half_pi<float>() - kPitchLimit);
			pitch = nextPitch - currentPitch;
			SOMBRA_DEBUG_LOG << "Updating the entity " << mPlayer << " orientation (" << pitch << ", " << yaw << ")";

			// Apply the rotation
			glm::quat qYaw = glm::angleAxis(yaw, glm::vec3(0.0f, 1.0f, 0.0f));
			glm::quat qPitch = glm::angleAxis(pitch, glm::vec3(1.0f, 0.0f, 0.0f));
			mPlayer->orientation = glm::normalize(qPitch * mPlayer->orientation * qYaw);
			mPlayer->updated.set( static_cast<int>(se::app::Entity::Update::Input) );
		}

		// Add WASD movement
		glm::vec3 direction(0.0f);
		if (mPlayerInput.movement[static_cast<int>(InputTransforms::Direction::Front)]) { direction += forward; }
		if (mPlayerInput.movement[static_cast<int>(InputTransforms::Direction::Back)]) { direction -= forward; }
		if (mPlayerInput.movement[static_cast<int>(InputTransforms::Direction::Right)]) { direction += right; }
		if (mPlayerInput.movement[static_cast<int>(InputTransforms::Direction::Left)]) { direction -= right; }
		float length = glm::length(direction);
		if (length > 0.0f) {
			mPlayer->velocity += kRunSpeed * direction / length;
			mPlayer->updated.set( static_cast<int>(se::app::Entity::Update::Input) );
		}

		// Add the world Y velocity
		if (mPlayerInput.movement[static_cast<int>(InputTransforms::Direction::Up)]) {
			mPlayer->velocity += kJumpSpeed * up;
			mPlayer->updated.set( static_cast<int>(se::app::Entity::Update::Input) );
		}
		if (mPlayerInput.movement[static_cast<int>(InputTransforms::Direction::Down)]) {
			mPlayer->velocity -= kJumpSpeed * up;
			mPlayer->updated.set( static_cast<int>(se::app::Entity::Update::Input) );
		}

		Application::onUpdate(deltaTime);
		SOMBRA_DEBUG_LOG << "End";
	}

}
