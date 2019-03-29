#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

#include <se/app/Entity.h>
#include <se/app/InputManager.h>
#include <se/app/GraphicsManager.h>
#include <se/app/PhysicsManager.h>
#include <se/app/CollisionManager.h>
#include <se/app/AnimationManager.h>
#include <se/app/AudioManager.h>

#include <se/graphics/Texture.h>
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

#include <se/loaders/MeshLoader.h>
#include <se/loaders/SceneReader.h>
#include <se/loaders/ImageReader.h>
#include <se/loaders/FontReader.h>
#include <se/loaders/TerrainLoader.h>
#include <se/loaders/RawMesh.h>
#include <AudioFile.h>

#include <se/utils/Image.h>
#include <se/utils/Log.h>
#include <se/utils/FileReader.h>

#include "game/Game.h"

namespace game {

// static variables definition
	const std::string Game::kTitle		= "< SOMBRA >";
	const unsigned int Game::kWidth		= 1280;
	const unsigned int Game::kHeight	= 720;
	const float Game::kUpdateTime		= 0.016f;
	const unsigned int Game::kNumCubes	= 50;
	const float Game::kFOV				= glm::radians(60.0f);
	const float Game::kZNear			= 0.5f;
	const float Game::kZFar				= 250.0f;

// Public functions
	Game::Game() : se::app::Application(kTitle, kWidth, kHeight, kUpdateTime) {}


	Game::~Game() {}


	se::loaders::RawMesh createRawMesh(const se::collision::HalfEdgeMesh& heMesh)
	{
		se::loaders::RawMesh rawMesh("heMeshTriangles");

		// The faces must be triangles
		se::collision::HalfEdgeMesh heMeshTriangles = se::collision::triangulateFaces(heMesh);

		rawMesh.positions.reserve(heMeshTriangles.vertices.size());
		rawMesh.faceIndices.reserve(3 * heMeshTriangles.faces.size());

		std::map<int, int> vertexMap;
		for (auto itVertex = heMeshTriangles.vertices.begin(); itVertex != heMeshTriangles.vertices.end(); ++itVertex) {
			rawMesh.positions.push_back(itVertex->location);
			vertexMap.emplace(itVertex.getIndex(), rawMesh.positions.size() - 1);
		}

		for (auto itFace = heMeshTriangles.faces.begin(); itFace != heMeshTriangles.faces.end(); ++itFace) {
			for (int iVertex : se::collision::getFaceIndices(heMeshTriangles, itFace.getIndex())) {
				rawMesh.faceIndices.push_back(vertexMap[iVertex]);
			}
		}

		return rawMesh;
	}


	se::collision::HalfEdgeMesh createTestTube1()
	{
		se::collision::HalfEdgeMesh meshData;
		se::collision::addVertex(meshData, { -0.000000014f, 0.499999761f, -1.0f });
		se::collision::addVertex(meshData, { -0.000000014f, 0.499999761f, 1.0f });
		se::collision::addVertex(meshData, { 0.249999970f, 0.433012485f, -1.0f });
		se::collision::addVertex(meshData, { 0.249999970f, 0.433012485f, 1.0f });
		se::collision::addVertex(meshData, { 0.433012694f, 0.249999791f, -1.0f });
		se::collision::addVertex(meshData, { 0.433012694f, 0.249999791f, 1.0f });
		se::collision::addVertex(meshData, { 0.5f, -0.000000210f, -1.0f });
		se::collision::addVertex(meshData, { 0.5f, -0.000000210f, 1.0f });
		se::collision::addVertex(meshData, { 0.433012694f, -0.250000208f, -1.0f });
		se::collision::addVertex(meshData, { 0.433012694f, -0.250000208f, 1.0f });
		se::collision::addVertex(meshData, { 0.250000029f, -0.433012902f, -1.0f });
		se::collision::addVertex(meshData, { 0.250000029f, -0.433012902f, 1.0f });
		se::collision::addVertex(meshData, { 0.00000006f, -0.500000178f, -1.0f });
		se::collision::addVertex(meshData, { 0.00000006f, -0.500000178f, 1.0f });
		se::collision::addVertex(meshData, { -0.249999910f, -0.433012962f, -1.0f });
		se::collision::addVertex(meshData, { -0.249999910f, -0.433012962f, 1.0f });
		se::collision::addVertex(meshData, { -0.433012634f, -0.250000357f, -1.0f });
		se::collision::addVertex(meshData, { -0.433012634f, -0.250000357f, 1.0f });
		se::collision::addVertex(meshData, { -0.5f, -0.000000421f, -1.0f });
		se::collision::addVertex(meshData, { -0.5f, -0.000000421f, 1.0f });
		se::collision::addVertex(meshData, { -0.433012872f, 0.249999567f, -1.0f });
		se::collision::addVertex(meshData, { -0.433012872f, 0.249999567f, 1.0f });
		se::collision::addVertex(meshData, { -0.250000327f, 0.433012336f, -1.0f });
		se::collision::addVertex(meshData, { -0.250000327f, 0.433012336f, 1.0f });
		se::collision::addVertex(meshData, { 0.0f, 1.0f, -1.0f });
		se::collision::addVertex(meshData, { 0.0f, 1.0f, 1.0f });
		se::collision::addVertex(meshData, { 0.5f, 0.866025388f, -1.0f });
		se::collision::addVertex(meshData, { 0.5f, 0.866025388f, 1.0f });
		se::collision::addVertex(meshData, { 0.866025447f, 0.499999970f, -1.0f });
		se::collision::addVertex(meshData, { 0.866025447f, 0.499999970f, 1.0f });
		se::collision::addVertex(meshData, { 1.0f, -0.000000043f, -1.0f });
		se::collision::addVertex(meshData, { 1.0f, -0.000000043f, 1.0f });
		se::collision::addVertex(meshData, { 0.866025388f, -0.500000059f, -1.0f });
		se::collision::addVertex(meshData, { 0.866025388f, -0.500000059f, 1.0f });
		se::collision::addVertex(meshData, { 0.500000059f, -0.866025388f, -1.0f });
		se::collision::addVertex(meshData, { 0.500000059f, -0.866025388f, 1.0f });
		se::collision::addVertex(meshData, { 0.00000015f, -1.0f, -1.0f });
		se::collision::addVertex(meshData, { 0.00000015f, -1.0f, 1.0f });
		se::collision::addVertex(meshData, { -0.499999791f, -0.866025507f, -1.0f });
		se::collision::addVertex(meshData, { -0.499999791f, -0.866025507f, 1.0f });
		se::collision::addVertex(meshData, { -0.866025209f, -0.500000298f, -1.0f });
		se::collision::addVertex(meshData, { -0.866025209f, -0.500000298f, 1.0f });
		se::collision::addVertex(meshData, { -1.0f, -0.000000464f, -1.0f });
		se::collision::addVertex(meshData, { -1.0f, -0.000000464f, 1.0f });
		se::collision::addVertex(meshData, { -0.866025686f, 0.499999493f, -1.0f });
		se::collision::addVertex(meshData, { -0.866025686f, 0.499999493f, 1.0f });
		se::collision::addVertex(meshData, { -0.500000596f, 0.866025090f, -1.0f });
		se::collision::addVertex(meshData, { -0.500000596f, 0.866025090f, 1.0f });
		se::collision::addFace(meshData, { 0, 2, 3, 1 });
		se::collision::addFace(meshData, { 2, 4, 5, 3 });
		se::collision::addFace(meshData, { 4, 6, 7, 5 });
		se::collision::addFace(meshData, { 6, 8, 9, 7 });
		se::collision::addFace(meshData, { 8, 10, 11, 9 });
		se::collision::addFace(meshData, { 10, 12, 13, 11 });
		se::collision::addFace(meshData, { 12, 14, 15, 13 });
		se::collision::addFace(meshData, { 14, 16, 17, 15 });
		se::collision::addFace(meshData, { 16, 18, 19, 17 });
		se::collision::addFace(meshData, { 18, 20, 21, 19 });
		se::collision::addFace(meshData, { 20, 22, 23, 21 });
		se::collision::addFace(meshData, { 22, 0, 1, 23 });
		se::collision::addFace(meshData, { 24, 25, 27, 26 });
		se::collision::addFace(meshData, { 26, 27, 29, 28 });
		se::collision::addFace(meshData, { 28, 29, 31, 30 });
		se::collision::addFace(meshData, { 30, 31, 33, 32 });
		se::collision::addFace(meshData, { 32, 33, 35, 34 });
		se::collision::addFace(meshData, { 34, 35, 37, 36 });
		se::collision::addFace(meshData, { 36, 37, 39, 38 });
		se::collision::addFace(meshData, { 38, 39, 41, 40 });
		se::collision::addFace(meshData, { 40, 41, 43, 42 });
		se::collision::addFace(meshData, { 42, 43, 45, 44 });
		se::collision::addFace(meshData, { 44, 45, 47, 46 });
		se::collision::addFace(meshData, { 46, 47, 25, 24 });
		se::collision::addFace(meshData, { 13, 15, 39, 37 });
		se::collision::addFace(meshData, { 37, 35, 11, 13 });
		se::collision::addFace(meshData, { 35, 33, 9, 11 });
		se::collision::addFace(meshData, { 33, 31, 7, 9 });
		se::collision::addFace(meshData, { 31, 29, 5, 7 });
		se::collision::addFace(meshData, { 29, 27, 3, 5 });
		se::collision::addFace(meshData, { 27, 25, 1, 3 });
		se::collision::addFace(meshData, { 25, 47, 23, 1 });
		se::collision::addFace(meshData, { 47, 45, 21, 23 });
		se::collision::addFace(meshData, { 45, 43, 19, 21 });
		se::collision::addFace(meshData, { 43, 41, 17, 19 });
		se::collision::addFace(meshData, { 41, 39, 15, 17 });
		se::collision::addFace(meshData, { 20, 18, 42, 44 });
		se::collision::addFace(meshData, { 16, 40, 42, 18 });
		se::collision::addFace(meshData, { 14, 38, 40, 16 });
		se::collision::addFace(meshData, { 12, 36, 38, 14 });
		se::collision::addFace(meshData, { 10, 34, 36, 12 });
		se::collision::addFace(meshData, { 8, 32, 34, 10 });
		se::collision::addFace(meshData, { 6, 30, 32, 8 });
		se::collision::addFace(meshData, { 4, 28, 30, 6 });
		se::collision::addFace(meshData, { 2, 26, 28, 4 });
		se::collision::addFace(meshData, { 0, 24, 26, 2 });
		se::collision::addFace(meshData, { 22, 46, 24, 0 });
		se::collision::addFace(meshData, { 20, 44, 46, 22 });
		return meshData;
	}


	void Game::init()
	{
		mGraphicsSystem->addLayer(&mLayer2D);

		/*********************************************************************
		 * GRAPHICS DATA
		 *********************************************************************/
		se::loaders::TerrainLoader terrainLoader(*mGraphicsManager, *mPhysicsManager, *mCollisionManager);
		se::collision::QuickHull qh(0.0001f);
		se::collision::HACD hacd(0.002f, 0.0002f);

		std::shared_ptr<se::graphics::Mesh> mesh1 = nullptr, mesh2 = nullptr;
		std::shared_ptr<se::utils::Image> heightMap1 = nullptr;
		std::shared_ptr<se::graphics::Texture> texture1 = nullptr, texture2 = nullptr;
		std::unique_ptr<se::graphics::Camera> camera1 = nullptr;
		std::unique_ptr<se::graphics::PointLight> pointLight1 = nullptr, pointLight2 = nullptr, pointLight3 = nullptr;
		std::shared_ptr<se::graphics::Font> arial = nullptr;
		std::unique_ptr<se::audio::Source> source1 = nullptr;
		se::loaders::DataHolder dataHolder1;

		try {
			// Readers
			AudioFile<float> audioFile;
			se::loaders::FontReader fontReader;
			auto sceneReader = se::loaders::SceneReader::createSceneReader(se::loaders::SceneFileType::GLTF);

			// Meshes
			se::loaders::RawMesh rawMesh1("Cube");
			rawMesh1.positions = {
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
			rawMesh1.texCoords = {
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
			rawMesh1.faceIndices = {
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
			rawMesh1.normals = se::loaders::MeshLoader::calculateNormals(rawMesh1.positions, rawMesh1.faceIndices);
			rawMesh1.tangents = se::loaders::MeshLoader::calculateTangents(rawMesh1.positions, rawMesh1.texCoords, rawMesh1.faceIndices);
			mesh1 = std::make_shared<se::graphics::Mesh>(se::loaders::MeshLoader::createGraphicsMesh(rawMesh1));

			se::loaders::RawMesh rawMesh2("Plane");
			rawMesh2.positions = { {-0.5f,-0.5f, 0.0f}, { 0.5f,-0.5f, 0.0f}, {-0.5f, 0.5f, 0.0f}, { 0.5f, 0.5f, 0.0f} };
			rawMesh2.texCoords = { {0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f} };
			rawMesh2.faceIndices = { 0, 1, 2, 1, 3, 2, };
			rawMesh2.normals = se::loaders::MeshLoader::calculateNormals(rawMesh2.positions, rawMesh2.faceIndices);
			rawMesh2.tangents = se::loaders::MeshLoader::calculateTangents(rawMesh2.positions, rawMesh2.texCoords, rawMesh2.faceIndices);
			mesh2 = std::make_shared<se::graphics::Mesh>(se::loaders::MeshLoader::createGraphicsMesh(rawMesh2));

			// GLTF scenes
			dataHolder1 = sceneReader->load("res/meshes/test.gltf");

			// Images
			se::utils::Image image1 = se::loaders::ImageReader::read("res/images/test.png");
			heightMap1 = std::make_unique<se::utils::Image>( se::loaders::ImageReader::read("res/images/terrain.png", 1) );

			// Textures
			texture1 = std::make_shared<se::graphics::Texture>();
			texture1->setFiltering(se::graphics::TextureFilter::Nearest, se::graphics::TextureFilter::Nearest);
			texture1->setWrapping(se::graphics::TextureWrap::Repeat, se::graphics::TextureWrap::Repeat);
			texture1->setImage(
				image1.pixels.get(), se::graphics::TypeId::UnsignedByte, se::graphics::ColorFormat::RGBA,
				image1.width, image1.height
			);

			float pixels[] = {
				0.0f, 0.0f, 0.0f,	1.0f, 1.0f, 1.0f,
				1.0f, 1.0f, 1.0f,	0.0f, 0.0f, 0.0f
			};
			texture2 = std::make_shared<se::graphics::Texture>();
			texture2->setFiltering(se::graphics::TextureFilter::Nearest, se::graphics::TextureFilter::Nearest);
			texture2->setWrapping(se::graphics::TextureWrap::Repeat, se::graphics::TextureWrap::Repeat);
			texture2->setImage(pixels, se::graphics::TypeId::Float, se::graphics::ColorFormat::RGB, 2, 2);

			// Cameras
			camera1 = std::make_unique<se::graphics::Camera>();
			camera1->setPerspectiveProjectionMatrix(kFOV, kWidth / static_cast<float>(kHeight), kZNear, kZFar);

			// Lights
			se::graphics::BaseLight baseLight1{ glm::vec3(1.0f, 0.75f, 0.8f) }, baseLight2{ glm::vec3(1.0f, 0.5f, 0.5f) };
			se::graphics::Attenuation attenuation1{ 0.25f, 0.2f, 0.1f };
			pointLight1 = std::make_unique<se::graphics::PointLight>(se::graphics::PointLight{ baseLight1, attenuation1, glm::vec3(0.0f) });
			pointLight2 = std::make_unique<se::graphics::PointLight>(se::graphics::PointLight{ baseLight2, attenuation1, glm::vec3(0.0f) });
			pointLight3 = std::make_unique<se::graphics::PointLight>(se::graphics::PointLight{ baseLight1, attenuation1, glm::vec3(0.0f) });

			// Fonts
			se::utils::FileReader fileReader3("res/fonts/arial.fnt");
			arial = std::move(fontReader.read(fileReader3));

			// Audio
			if (audioFile.load("res/audio/bounce.wav")) {
				mBuffers.emplace_back(
					audioFile.samples[0].data(), audioFile.samples[0].size() * sizeof(float),
					se::audio::FormatId::MonoFloat, audioFile.getSampleRate()
				);
				source1 = std::make_unique<se::audio::Source>();
				source1->bind(mBuffers.back());
				source1->setLooping(true);
				source1->play();
			}
		}
		catch (std::exception& e) {
			SOMBRA_ERROR_LOG << "Error: " << e.what();
			mState = AppState::Error;
			return;
		}

		// RenderableTexts
		se::graphics::RenderableText renderableText1("First try rendering text", arial, 10, glm::vec2());

		// Renderable2Ds
		mRenderable2Ds.emplace_back(glm::vec2(0.75f, 0.75f), glm::vec2(0.15f, 0.2f), texture1);
		mLayer2D.addRenderable2D(&mRenderable2Ds.back());

		/*********************************************************************
		 * GAME DATA
		 *********************************************************************/
		// Player
		auto player	= std::make_unique<se::app::Entity>("player");
		player->position = glm::vec3(0, 1, 10);

		auto rigidBody1 = std::make_unique<se::physics::RigidBody>(
			40.0f, 0.01f,
			2.0f / 5.0f * 10.0f * glm::pow(2.0f, 2.0f) * glm::mat3(1.0f), 0.01f
		);
		auto collider1 = std::make_unique<se::collision::BoundingSphere>(0.5f);
		mCollisionManager->addEntity(player.get(), std::move(collider1), rigidBody1.get());
		mPhysicsManager->addEntity(player.get(), std::move(rigidBody1));

		mGraphicsManager->addEntity(player.get(), std::move(camera1));
		mGraphicsManager->addEntity(player.get(), std::move(pointLight1));
		mAudioManager->setListener(player.get());
		mInputManager->addEntity(player.get());

		mEntities.push_back(std::move(player));

		// Terrain
		mEntities.push_back( terrainLoader.createTerrain("terrain", 100.0f, *heightMap1, 10.0f) );

		// Plane
		auto plane = std::make_unique<se::app::Entity>("plane");
		plane->position = glm::vec3(-15.0f, 1.0f, -5.0f);

		std::shared_ptr<se::graphics::Material> planeMaterial(new se::graphics::Material{
			"plane_material",
			se::graphics::PBRMetallicRoughness{ glm::vec4(1.0f), texture2, 0.2f, 0.5f, nullptr },
			nullptr, nullptr, nullptr, glm::vec3(0.0f), se::graphics::AlphaMode::Opaque, 0.5f, false
		});

		auto renderable3D1 = std::make_unique<se::graphics::Renderable3D>(mesh2, planeMaterial);
		mGraphicsManager->addEntity(plane.get(), std::move(renderable3D1));

		mEntities.push_back(std::move(plane));

		// Fixed cubes
		glm::vec3 cubePositions[5] = { glm::vec3(2, 5, -10), glm::vec3(0, 7, -10), glm::vec3(0, 5, -8), glm::vec3(0, 5, -10), glm::vec3(10, 5, -10) };
		glm::vec4 colors[5] = { { 1.0f, 0.2f, 0.2f, 1.0f }, { 0.2f, 1.0f, 0.2f, 1.0f }, { 0.2f, 0.2f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.2f, 0.2f, 0.1f, 1.0f } };
		se::physics::RigidBody *rb1 = nullptr, *rb2 = nullptr;
		for (std::size_t i = 0; i < 5; ++i) {
			auto cube = std::make_unique<se::app::Entity>("non-random-cube");
			cube->position = cubePositions[i];

			auto rigidBody2 = std::make_unique<se::physics::RigidBody>(
				20.0f, 0.95f,
				2.0f / 5.0f * 10.0f * glm::pow(2.0f, 2.0f) * glm::mat3(1.0f), 0.95f
			);
			if (i == 1) {
				rb1 = rigidBody2.get();
			}
			if (i == 2) {
				mAudioManager->addSource(cube.get(), std::move(source1));
			}
			if (i == 3) {
				rigidBody2->angularVelocity = glm::vec3(0, 10, 0);
				rb2 = rigidBody2.get();
			}
			if (i == 4) {
				cube->velocity += glm::vec3(-1, 0, 0);
			}
			auto collider2 = std::make_unique<se::collision::BoundingBox>(glm::vec3(1,1,1));
			mCollisionManager->addEntity(cube.get(), std::move(collider2), rigidBody2.get());
			mPhysicsManager->addEntity(cube.get(), std::move(rigidBody2));

			std::shared_ptr<se::graphics::Material> tmpMaterial(new se::graphics::Material{
				"tmp_material",
				se::graphics::PBRMetallicRoughness{ colors[i], nullptr, 0.2f, 0.5f, nullptr },
				nullptr, nullptr, nullptr, glm::vec3(0.0f), se::graphics::AlphaMode::Opaque, 0.5f, false
			});

			auto renderable3D2 = std::make_unique<se::graphics::Renderable3D>(mesh1, tmpMaterial);
			mGraphicsManager->addEntity(cube.get(), std::move(renderable3D2));

			mEntities.push_back(std::move(cube));
		}

		mConstraints.push_back(new se::physics::DistanceConstraint({ rb1, rb2 }));
		mPhysicsEngine->getConstraintManager().addConstraint(mConstraints.back());

		// HACD Tube
		hacd.calculate( createTestTube1() );
		for (const se::collision::HalfEdgeMesh& heMesh : hacd.getMeshes()) {
			auto tubeSlice = std::make_unique<se::app::Entity>("tube");
			tubeSlice->orientation = glm::normalize(glm::quat(-1, glm::vec3(1, 0, 0)));
			tubeSlice->position = glm::vec3(0.0f, 2.0f, 75.0f);

			std::shared_ptr<se::graphics::Material> tmpMaterial(new se::graphics::Material{
				"tmp_material",
				se::graphics::PBRMetallicRoughness{
					glm::vec4(glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f)),
					nullptr, 0.75f, 0.5f, nullptr
				},
				nullptr, nullptr, nullptr, glm::vec3(0.0f), se::graphics::AlphaMode::Opaque, 0.5f, false
			});

			auto tmpRawMesh = createRawMesh(heMesh);
			auto tmpGraphicsMesh = std::make_shared<se::graphics::Mesh>(se::loaders::MeshLoader::createGraphicsMesh(tmpRawMesh));
			auto renderable3D2 = std::make_unique<se::graphics::Renderable3D>(tmpGraphicsMesh, tmpMaterial);
			mGraphicsManager->addEntity(tubeSlice.get(), std::move(renderable3D2));

			mEntities.push_back(std::move(tubeSlice));
		}

		// Random cubes
		for (std::size_t i = 0; i < kNumCubes; ++i) {
			auto cube = std::make_unique<se::app::Entity>("random-cube");
			cube->position = glm::ballRand(50.0f);

			auto rigidBody2 = std::make_unique<se::physics::RigidBody>(
				10.0f, 0.9f,
				2.0f / 5.0f * 10.0f * glm::pow(2.0f, 2.0f) * glm::mat3(1.0f), 0.9f
			);
			auto collider2 = std::make_unique<se::collision::BoundingBox>(glm::vec3(1,1,1));
			mCollisionManager->addEntity(cube.get(), std::move(collider2), rigidBody2.get());
			mPhysicsManager->addEntity(cube.get(), std::move(rigidBody2));

			auto renderable3D2 = std::make_unique<se::graphics::Renderable3D>(mesh1, nullptr);
			mGraphicsManager->addEntity(cube.get(), std::move(renderable3D2));

			mEntities.push_back(std::move(cube));
		}

		// Lights
		auto eL2 = std::make_unique<se::app::Entity>("point-light2");
		eL2->position = glm::vec3(-3, 1, 5);
		mGraphicsManager->addEntity(eL2.get(), std::move(pointLight2));
		mEntities.push_back(std::move(eL2));

		auto eL3 = std::make_unique<se::app::Entity>("point-light3");
		eL3->position = glm::vec3(2, 10, 5);
		mGraphicsManager->addEntity(eL3.get(), std::move(pointLight3));
		mEntities.push_back(std::move(eL3));

		// GLTF meshes
		for (auto& pair : dataHolder1.entityR3DMap) {
			for (std::size_t iRenderable3D : pair.second) {
				mGraphicsManager->addEntity(dataHolder1.entities[pair.first].get(), std::move(dataHolder1.renderable3Ds[iRenderable3D]));
			}
			mEntities.push_back( std::move(dataHolder1.entities[pair.first]) );
		}
	}


	void Game::end()
	{
		for (se::physics::Constraint* constraint : mConstraints) {
			mPhysicsEngine->getConstraintManager().removeConstraint(constraint);
		}

		for (EntityUPtr& entity : mEntities) {
			mInputManager->removeEntity(entity.get());
			mGraphicsManager->removeEntity(entity.get());
			mPhysicsManager->removeEntity(entity.get());
			mCollisionManager->removeEntity(entity.get());
			mAnimationManager->removeEntity(entity.get());
			mAudioManager->removeEntity(entity.get());
		}
		mEntities.clear();

		mGraphicsSystem->removeLayer(&mLayer2D);
	}

}
