#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

#include <se/app/Entity.h>
#include <se/app/InputManager.h>
#include <se/app/GraphicsManager.h>
#include <se/app/PhysicsManager.h>
#include <se/app/CollisionManager.h>
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
#include <se/collision/CompoundCollider.h>
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
#include <se/loaders/MeshReader.h>
#include <se/loaders/MaterialReader.h>
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
	const std::string Game::sTitle		= "< SOMBRA >";
	const unsigned int Game::sWidth		= 1280;
	const unsigned int Game::sHeight	= 720;
	const float Game::sUpdateTime		= 0.016f;
	const unsigned int Game::sNumCubes	= 50;

// Public functions
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
		se::loaders::TerrainLoader terrainLoader(*mGraphicsManager, *mPhysicsManager);
		se::collision::QuickHull qh(0.0001f);
		se::collision::HACD hacd(0.002f, 0.0002f);

		std::shared_ptr<se::graphics::Mesh> mesh1 = nullptr, mesh2 = nullptr;
		std::vector<std::shared_ptr<se::loaders::RawMesh>> fileRawMeshes;
		std::vector<std::shared_ptr<se::graphics::Mesh>> fileMeshes;
		std::vector<std::shared_ptr<se::graphics::Material>> fileMaterials;
		std::shared_ptr<se::utils::Image> heightMap1 = nullptr;
		std::shared_ptr<se::graphics::Texture> texture1 = nullptr, texture2 = nullptr;
		std::unique_ptr<se::graphics::Camera> camera1 = nullptr;
		std::unique_ptr<se::graphics::PointLight> pointLight1 = nullptr, pointLight2 = nullptr;
		std::shared_ptr<se::graphics::Font> arial = nullptr;
		std::unique_ptr<se::audio::Source> source1 = nullptr;

		try {
			// Readers
			AudioFile<float> audioFile;
			se::loaders::MeshReader meshReader;
			se::loaders::MaterialReader materialReader;
			se::loaders::FontReader fontReader;

			// Meshes
			se::loaders::RawMesh rawMesh1("Cube");
			rawMesh1.positions = {
				{-0.5f,	-0.5f,	-0.5f},
				{-0.5f,	-0.5f,	 0.5f},
				{-0.5f,	 0.5f,	-0.5f},
				{-0.5f,	 0.5f,	 0.5f},
				{ 0.5f,	-0.5f,	-0.5f},
				{ 0.5f,	-0.5f,	 0.5f},
				{ 0.5f,	 0.5f,	-0.5f},
				{ 0.5f,	 0.5f,	 0.5f}
			};
			rawMesh1.faceIndices = {
				0, 1, 2,
				1, 3, 2,
				0, 2, 4,
				2, 6, 4,
				4, 6, 5,
				5, 6, 7,
				1, 5, 3,
				3, 5, 7,
				0, 4, 1,
				1, 4, 5,
				2, 3, 6,
				3, 7, 6
			};
			rawMesh1.normals = meshReader.calculateNormals(rawMesh1.positions, rawMesh1.faceIndices);
			rawMesh1.uvs = std::vector<glm::vec2>(8);
			mesh1 = std::make_shared<se::graphics::Mesh>(se::loaders::MeshLoader::createGraphicsMesh(rawMesh1));

			se::loaders::RawMesh rawMesh2("Plane");
			rawMesh2.positions = {
				{-0.5f,	-0.5f,	0.0f},
				{ 0.5f,	-0.5f,	0.0f},
				{-0.5f,	0.5f,	0.0f},
				{ 0.5f,	0.5f,	0.0f}
			};
			rawMesh2.normals = {
				{0.0f,	0.0f,	1.0f},
				{0.0f,	0.0f,	1.0f},
				{0.0f,	0.0f,	1.0f},
				{0.0f,	0.0f,	1.0f}
			};
			rawMesh2.uvs = {
				{0.0f,	0.0f},
				{1.0f,	0.0f},
				{0.0f,	1.0f},
				{1.0f,	1.0f}
			};
			rawMesh2.faceIndices = {
				0, 1, 2,
				1, 3, 2,
			};
			mesh2 = std::make_shared<se::graphics::Mesh>(se::loaders::MeshLoader::createGraphicsMesh(rawMesh2));

			se::utils::FileReader fileReader1("res/meshes/test.semsh");
			auto loadedRawMeshes = meshReader.read(fileReader1);
			for (auto& meshRawUPtr : loadedRawMeshes) {
				fileRawMeshes.push_back( std::move(meshRawUPtr) );
			}

			// Materials
			se::utils::FileReader fileReader2("res/materials/game_materials.semat");
			auto loadedMaterials = materialReader.read(fileReader2);
			for (std::unique_ptr<se::graphics::Material>& materialUPtr : loadedMaterials) {
				fileMaterials.push_back( std::move(materialUPtr) );
			}

			// Images
			se::utils::Image image1 = se::loaders::ImageReader::read("res/images/test.png", se::utils::ImageFormat::RGBA_IMAGE);
			heightMap1 = std::make_unique<se::utils::Image>( se::loaders::ImageReader::read("res/images/terrain.png", se::utils::ImageFormat::L_IMAGE) );

			// Textures
			texture1 = std::make_shared<se::graphics::Texture>();
			texture1->setImage(
				image1.getPixels(), se::graphics::TexturePixelType::U_BYTE, se::graphics::TextureFormat::RGBA,
				image1.getWidth(), image1.getHeight()
			);

			float pixels[] = {
				0.0f, 0.0f, 0.0f,	1.0f, 1.0f, 1.0f,
				1.0f, 1.0f, 1.0f,	0.0f, 0.0f, 0.0f
			};
			texture2 = std::make_shared<se::graphics::Texture>();
			texture2->setImage(pixels, se::graphics::TexturePixelType::FLOAT, se::graphics::TextureFormat::RGB, 2, 2);

			// Cameras
			camera1 = std::make_unique<se::graphics::Camera>(glm::vec3(0,0,0), glm::vec3(0,0,-1), glm::vec3(0,1,0));

			// Lights
			se::graphics::BaseLight baseLight1(glm::vec3(0.5f, 0.6f, 0.3f), glm::vec3(0.1f, 0.5f, 0.6f));
			se::graphics::Attenuation attenuation1{ 0.25f, 0.2f, 0.1f };
			pointLight1 = std::make_unique<se::graphics::PointLight>(baseLight1, attenuation1, glm::vec3());
			pointLight2 = std::make_unique<se::graphics::PointLight>(baseLight1, attenuation1, glm::vec3());

			se::utils::FileReader fileReader3("res/fonts/arial.fnt");
			arial = std::move(fontReader.read(fileReader3));

			// Audio
			if (audioFile.load("res/audio/bounce.wav")) {
				buffer1.setBufferFloatData(audioFile.samples[0], audioFile.getSampleRate());
				source1 = std::make_unique<se::audio::Source>();
				source1->bind(buffer1);
				source1->setLooping(true);
				source1->play();
			}
		}
		catch (std::exception& e) {
			mState = AppState::Error;
			SOMBRA_ERROR_LOG << "Error: " << e.what();
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
		mAudioManager->setListener(player.get());
		mInputManager->addEntity(player.get());

		mEntities.push_back(std::move(player));

		// Terrain
		mEntities.push_back( terrainLoader.createTerrain("terrain", 100.0f, *heightMap1, 10.0f) );

		// Plane
		auto plane = std::make_unique<se::app::Entity>("plane");
		plane->position = glm::vec3(-5.0f, 1.0f, -5.0f);

		auto renderable3D1 = std::make_unique<se::graphics::Renderable3D>(mesh2, fileMaterials[4], texture2);
		mGraphicsManager->addEntity(plane.get(), std::move(renderable3D1), glm::mat4(1.0f));

		mEntities.push_back(std::move(plane));

		// Fixed cubes
		glm::vec3 cubePositions[5] = { glm::vec3(2, 5, -10), glm::vec3(0, 7, -10), glm::vec3(0, 5, -8), glm::vec3(0, 5, -10), glm::vec3(10, 5, -10) };
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

			auto renderable3D2 = std::make_unique<se::graphics::Renderable3D>(mesh1, fileMaterials[i], nullptr);
			mGraphicsManager->addEntity(cube.get(), std::move(renderable3D2), glm::mat4(1.0f));

			mEntities.push_back(std::move(cube));
		}

		constraint = new se::physics::DistanceConstraint({ rb1, rb2 });
		mPhysicsEngine->getConstraintManager().addConstraint(constraint);

		// HACD Tube
		hacd.calculate( createTestTube1() );
		for (const se::collision::HalfEdgeMesh& heMesh : hacd.getMeshes()) {
			auto tubeSlice = std::make_unique<se::app::Entity>("tube");
			tubeSlice->orientation = glm::normalize(glm::quat(-1, glm::vec3(1, 0, 0)));
			tubeSlice->position = glm::vec3(0.0f, 2.0f, 75.0f);

			auto tmpMaterial = std::make_shared<se::graphics::Material>(
				"tmp_material",
				glm::vec3( glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f) ),
				glm::vec3( glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f) ),
				glm::vec3( glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f) ),
				0.2f
			);

			auto tmpRawMesh = createRawMesh(heMesh);
			auto tmpGraphicsMesh = std::make_shared<se::graphics::Mesh>(se::loaders::MeshLoader::createGraphicsMesh(tmpRawMesh));
			auto renderable3D2 = std::make_unique<se::graphics::Renderable3D>(tmpGraphicsMesh, tmpMaterial, nullptr, se::graphics::RenderFlags::WIREFRAME | se::graphics::RenderFlags::DISABLE_FACE_CULLING);
			mGraphicsManager->addEntity(tubeSlice.get(), std::move(renderable3D2), glm::mat4(1.0f));

			mEntities.push_back(std::move(tubeSlice));
		}

		// Random cubes
		for (std::size_t i = 0; i < sNumCubes; ++i) {
			auto cube = std::make_unique<se::app::Entity>("random-cube");
			cube->position = glm::ballRand(50.0f);

			auto rigidBody2 = std::make_unique<se::physics::RigidBody>(
				10.0f, 0.9f,
				2.0f / 5.0f * 10.0f * glm::pow(2.0f, 2.0f) * glm::mat3(1.0f), 0.9f
			);
			auto collider2 = std::make_unique<se::collision::BoundingBox>(glm::vec3(1,1,1));
			mCollisionManager->addEntity(cube.get(), std::move(collider2), rigidBody2.get());
			mPhysicsManager->addEntity(cube.get(), std::move(rigidBody2));

			auto renderable3D2 = std::make_unique<se::graphics::Renderable3D>(mesh1, fileMaterials[4], nullptr);
			mGraphicsManager->addEntity(cube.get(), std::move(renderable3D2), glm::mat4(1.0f));

			mEntities.push_back(std::move(cube));
		}

		// Buildings
		for (std::size_t i = 0; i < fileRawMeshes.size(); ++i) {
			const se::loaders::RawMesh& rawMesh = *fileRawMeshes[i];
			if (i == 0) {
				auto building = std::make_unique<se::app::Entity>("building");
				building->orientation = glm::normalize(glm::quat(-1, glm::vec3(1, 0, 0)));

				auto tmpPair = se::loaders::MeshLoader::createHalfEdgeMesh(rawMesh);
				qh.calculate(tmpPair.first);
				auto rigidBody2 = std::make_unique<se::physics::RigidBody>();
				auto collider2 = std::make_unique<se::collision::ConvexPolyhedron>(qh.getMesh());
				mCollisionManager->addEntity(building.get(), std::move(collider2), rigidBody2.get());
				mPhysicsManager->addEntity(building.get(), std::move(rigidBody2));

				auto tmpMaterial = std::make_shared<se::graphics::Material>(
					"tmp_material",
					glm::vec3( glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f) ),
					glm::vec3( glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f) ),
					glm::vec3( glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f) ),
					0.2f
				);
				auto tmpRawMesh = createRawMesh(qh.getMesh());
				auto tmpGraphicsMesh = std::make_shared<se::graphics::Mesh>(se::loaders::MeshLoader::createGraphicsMesh(tmpRawMesh));
				auto renderable3D2 = std::make_unique<se::graphics::Renderable3D>(tmpGraphicsMesh, tmpMaterial, nullptr, se::graphics::RenderFlags::WIREFRAME | se::graphics::RenderFlags::DISABLE_FACE_CULLING);

				mGraphicsManager->addEntity(building.get(), std::move(renderable3D2), glm::mat4(1.0f));

				mEntities.push_back(std::move(building));
			}
			else {
				auto building = std::make_unique<se::app::Entity>("building");
				building->orientation = glm::normalize(glm::quat(-1, glm::vec3(1, 0, 0)));

				auto tmpMaterial = std::make_shared<se::graphics::Material>(
					"tmp_material",
					glm::vec3( glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f) ),
					glm::vec3( glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f) ),
					glm::vec3( glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f) ),
					0.2f
				);
				auto tmpGraphicsMesh = std::make_shared<se::graphics::Mesh>(se::loaders::MeshLoader::createGraphicsMesh(rawMesh));
				auto renderable3D2 = std::make_unique<se::graphics::Renderable3D>(tmpGraphicsMesh, tmpMaterial, nullptr);
				mGraphicsManager->addEntity(building.get(), std::move(renderable3D2), glm::mat4(1.0f));

				mEntities.push_back(std::move(building));
			}
		}

		// Lights
		auto eL1 = std::make_unique<se::app::Entity>("point-light1");
		eL1->position = glm::vec3(2, 1, 5);
		mGraphicsManager->addEntity(eL1.get(), std::move(pointLight1));
		mEntities.push_back(std::move(eL1));

		auto eL2 = std::make_unique<se::app::Entity>("point-light2");
		eL2->position = glm::vec3(-3, 1, 5);
		mGraphicsManager->addEntity(eL2.get(), std::move(pointLight2));
		mEntities.push_back(std::move(eL2));
	}


	void Game::end()
	{
		for (EntityUPtr& entity : mEntities) {
			mInputManager->removeEntity(entity.get());
			mPhysicsManager->removeEntity(entity.get());
			mCollisionManager->removeEntity(entity.get());
			mGraphicsManager->removeEntity(entity.get());
		}
		mEntities.clear();
		//delete constraint;

		mGraphicsSystem->removeLayer(&mLayer2D);
	}

}
