#include <numeric>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <AudioFile.h>

#include <se/window/KeyCodes.h>
#include <se/window/WindowManager.h>

#include <se/app/graphics/Image.h>
#include <se/app/graphics/RawMesh.h>
#include <se/app/graphics/Material.h>
#include <se/app/graphics/TextureUtils.h>
#include <se/app/events/ContainerEvent.h>
#include <se/app/loaders/MeshLoader.h>
#include <se/app/loaders/ImageReader.h>
#include <se/app/loaders/FontReader.h>
#include <se/app/loaders/TerrainLoader.h>
#include <se/app/loaders/SceneReader.h>
#include <se/app/loaders/TechniqueLoader.h>
#include <se/app/EntityDatabase.h>
#include <se/app/CameraSystem.h>
#include <se/app/AudioSystem.h>
#include <se/app/AppRenderer.h>
#include <se/app/TagComponent.h>
#include <se/app/MeshComponent.h>
#include <se/app/CameraComponent.h>
#include <se/app/LightComponent.h>
#include <se/app/LightProbe.h>
#include <se/app/TransformsComponent.h>
#include <se/app/Scene.h>

#include <se/graphics/Renderer.h>
#include <se/graphics/3D/RenderableMesh.h>
#include <se/graphics/core/GraphicsOperations.h>

#include <se/collision/BoundingBox.h>
#include <se/collision/BoundingSphere.h>
#include <se/collision/ConvexPolyhedron.h>
#include <se/collision/CompositeCollider.h>
#include <se/collision/HalfEdgeMeshExt.h>
#include <se/collision/QuickHull.h>
#include <se/collision/HACD.h>

#include <se/physics/RigidBody.h>
#include <se/physics/PhysicsEngine.h>
#include <se/physics/forces/Gravity.h>
#include <se/physics/constraints/DistanceConstraint.h>

#include <se/animation/AnimationEngine.h>
#include <se/animation/CompositeAnimator.h>

#include <se/audio/Buffer.h>
#include <se/audio/Source.h>

#include <se/utils/Log.h>
#include <se/utils/Repository.h>

#include "Game.h"
#include "Level.h"

namespace game {

	class MyTechniqueBuilder : public se::app::SceneReader::TechniqueBuilder
	{
	private:	// Attributes
		se::app::Application& mApplication;
		se::app::Scene& mScene;

	public:		// Functions
		MyTechniqueBuilder(se::app::Application& application, se::app::Scene& scene) :
			mApplication(application), mScene(scene) {};

		virtual se::app::SceneReader::TechniqueSPtr createTechnique(const se::app::Material& material, bool hasSkin) override
		{
			std::string shadowPassKey = hasSkin? "passShadowSkinning" : "passShadow";
			auto shadowPass = mScene.repository.find<std::string, se::graphics::Pass>(shadowPassKey);

			auto gBufferRenderer = static_cast<se::graphics::Renderer*>(mApplication.getExternalTools().graphicsEngine->getRenderGraph().getNode("gBufferRenderer"));
			std::string programKey = hasSkin? "programGBufMaterialSkinning" : "programGBufMaterial";
			auto program = mScene.repository.find<std::string, se::graphics::Program>(programKey);
			auto pass = std::make_shared<se::graphics::Pass>(*gBufferRenderer);
			pass->addBindable(program);
			se::app::TechniqueLoader::addMaterialBindables(pass, material, program);

			auto technique = std::make_unique<se::graphics::Technique>();
			technique->addPass(shadowPass)
				.addPass(pass);

			return technique;
		};
	};


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
	Level::Level(Game& game) :
		IGameScreen(game),
		mGame(game), mScene("Level", game), mPlayerEntity(se::app::kNullEntity),
		mPlayerController(nullptr), mLogoTexture(nullptr), mReticleTexture(nullptr), mPickText(nullptr)
	{
		mGame.getEventManager().subscribe(this, se::app::Topic::Key);

		/*********************************************************************
		 * GRAPHICS DATA
		 *********************************************************************/
		se::app::TerrainLoader terrainLoader(mGame.getEntityDatabase(), mScene);
		se::collision::QuickHull qh(0.0001f);
		se::collision::HACD hacd(0.002f, 0.0002f);

		se::app::Image<unsigned char> heightMap1, splatMap1, logo1, reticle1;
		se::app::Image<float> environment1;
		std::shared_ptr<se::graphics::Mesh> cubeMesh = nullptr, planeMesh = nullptr;
		std::shared_ptr<se::graphics::Texture> logoTexture, reticleTexture, chessTexture, splatmapTexture,
			skyTexture, environmentTexture, prefilterTexture;
		std::shared_ptr<se::graphics::Font> arial = nullptr;
		//se::app::Scenes loadedScenes;

		try {
			// Meshes
			se::app::RawMesh cubeRawMesh = se::app::MeshLoader::createBoxMesh("Cube", glm::vec3(1.0f));
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

			// Programs
			std::shared_ptr<se::graphics::Program> programShadow;
			programShadow = se::app::TechniqueLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, nullptr);
			if (!programShadow) {
				throw std::runtime_error("programShadow not found");
			}
			mScene.repository.add(std::string("programShadow"), programShadow);

			std::shared_ptr<se::graphics::Program> programShadowSkinning;
			programShadowSkinning = se::app::TechniqueLoader::createProgram("res/shaders/vertex3DSkinning.glsl", nullptr, nullptr);
			if (!programShadowSkinning) {
				throw std::runtime_error("programShadowSkinning not found");
			}
			mScene.repository.add(std::string("programShadow"), programShadowSkinning);

			std::shared_ptr<se::graphics::Program> programShadowTerrain;
			programShadowTerrain = se::app::TechniqueLoader::createProgram("res/shaders/vertexTerrain.glsl", "res/shaders/geometryTerrain.glsl", nullptr);
			if (!programShadowTerrain) {
				throw std::runtime_error("programShadowTerrain not found");
			}
			mScene.repository.add(std::string("programShadowTerrain"), std::move(programShadowTerrain));

			std::shared_ptr<se::graphics::Program> programSky;
			programSky = se::app::TechniqueLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, "res/shaders/fragmentSkyBox.glsl");
			if (!programSky) {
				throw std::runtime_error("programSky not found");
			}
			mScene.repository.add(std::string("programSky"), programSky);

			std::shared_ptr<se::graphics::Program> programGBufMaterial;
			programGBufMaterial = se::app::TechniqueLoader::createProgram("res/shaders/vertexNormalMap.glsl", nullptr, "res/shaders/fragmentGBufMaterial.glsl");
			if (!programGBufMaterial) {
				throw std::runtime_error("programGBufMaterial not found");
			}
			mScene.repository.add(std::string("programGBufMaterial"), programGBufMaterial);

			std::shared_ptr<se::graphics::Program> programGBufMaterialSkinning;
			programGBufMaterialSkinning = se::app::TechniqueLoader::createProgram("res/shaders/vertexNormalMapSkinning.glsl", nullptr, "res/shaders/fragmentGBufMaterial.glsl");
			if (!programGBufMaterialSkinning) {
				throw std::runtime_error("programGBufMaterialSkinning not found");
			}
			mScene.repository.add(std::string("programGBufMaterialSkinning"), std::move(programGBufMaterialSkinning));

			std::shared_ptr<se::graphics::Program> programGBufSplatmap;
			programGBufSplatmap = se::app::TechniqueLoader::createProgram("res/shaders/vertexTerrain.glsl", "res/shaders/geometryTerrain.glsl", "res/shaders/fragmentGBufSplatmap.glsl");
			if (!programGBufSplatmap) {
				throw std::runtime_error("programGBufSplatmap not found");
			}
			mScene.repository.add(std::string("programGBufSplatmap"), std::move(programGBufSplatmap));

			auto shadowRenderer = static_cast<se::graphics::Renderer*>(mGame.getExternalTools().graphicsEngine->getRenderGraph().getNode("shadowRenderer"));
			auto passShadow = std::make_shared<se::graphics::Pass>(*shadowRenderer);
			passShadow->addBindable(programShadow);
			mScene.repository.add(std::string("passShadow"), std::move(passShadow));

			auto passShadowSkinning = std::make_shared<se::graphics::Pass>(*shadowRenderer);
			passShadowSkinning->addBindable(programShadowSkinning);
			mScene.repository.add(std::string("passShadowSkinning"), std::move(passShadowSkinning));

			// Readers
			AudioFile<float> audioFile;
			MyTechniqueBuilder techniqueBuilder(mGame, mScene);
			auto sceneReader = se::app::SceneReader::createSceneReader(se::app::SceneReader::FileType::GLTF, mGame, techniqueBuilder);

			// Fonts
			arial = mGame.getRepository().find<std::string, se::graphics::Font>("arial");
			if (!arial) {
				throw std::runtime_error("Arial font not found");
			}

			// GLTF scenes
			se::app::Result result = sceneReader->load("res/meshes/test.gltf", mScene);
			if (!result) {
				throw std::runtime_error(result.description());
			}

			// Images
			result = se::app::ImageReader::read("res/images/logo.png", logo1);
			if (!result) {
				throw std::runtime_error(result.description());
			}

			result = se::app::ImageReader::read("res/images/terrain.png", heightMap1, 1);
			if (!result) {
				throw std::runtime_error(result.description());
			}

			result = se::app::ImageReader::read("res/images/splatmap.png", splatMap1);
			if (!result) {
				throw std::runtime_error(result.description());
			}

			result = se::app::ImageReader::read("res/images/reticle.png", reticle1);
			if (!result) {
				throw std::runtime_error(result.description());
			}

			result = se::app::ImageReader::readHDR("res/images/satara_night_2k.hdr", environment1);
			if (!result) {
				throw std::runtime_error(result.description());
			}

			// Textures
			logoTexture = std::make_shared<se::graphics::Texture>(se::graphics::TextureTarget::Texture2D);
			logoTexture->setImage(
				logo1.pixels.get(), se::graphics::TypeId::UnsignedByte, se::graphics::ColorFormat::RGBA, se::graphics::ColorFormat::RGBA,
				logo1.width, logo1.height
			);

			reticleTexture = std::make_shared<se::graphics::Texture>(se::graphics::TextureTarget::Texture2D);
			reticleTexture->setImage(
				reticle1.pixels.get(), se::graphics::TypeId::UnsignedByte, se::graphics::ColorFormat::RGBA, se::graphics::ColorFormat::RGBA,
				reticle1.width, reticle1.height
			);

			float pixels[] = {
				0.0f, 0.0f, 0.0f,	1.0f, 1.0f, 1.0f,
				1.0f, 1.0f, 1.0f,	0.0f, 0.0f, 0.0f
			};
			chessTexture = std::make_shared<se::graphics::Texture>(se::graphics::TextureTarget::Texture2D);
			chessTexture->setImage(pixels, se::graphics::TypeId::Float, se::graphics::ColorFormat::RGB, se::graphics::ColorFormat::RGB, 2, 2);

			splatmapTexture = std::make_shared<se::graphics::Texture>(se::graphics::TextureTarget::Texture2D);
			splatmapTexture->setWrapping(se::graphics::TextureWrap::ClampToEdge, se::graphics::TextureWrap::ClampToEdge)
				.setImage(
					splatMap1.pixels.get(), se::graphics::TypeId::UnsignedByte, se::graphics::ColorFormat::RGBA,
					se::graphics::ColorFormat::RGBA, splatMap1.width, splatMap1.height
				);

			auto environmentEquiTexture = std::make_shared<se::graphics::Texture>(se::graphics::TextureTarget::Texture2D);
			environmentEquiTexture->setWrapping(se::graphics::TextureWrap::ClampToEdge, se::graphics::TextureWrap::ClampToEdge)
				.setFiltering(se::graphics::TextureFilter::Linear, se::graphics::TextureFilter::Linear)
				.setImage(
					environment1.pixels.get(), se::graphics::TypeId::Float, se::graphics::ColorFormat::RGB,
					se::graphics::ColorFormat::RGB, environment1.width, environment1.height
				);
			skyTexture = se::app::TextureUtils::equirectangularToCubeMap(environmentEquiTexture, 512);
			environmentTexture = se::app::TextureUtils::convoluteCubeMap(skyTexture, 32);
			prefilterTexture = se::app::TextureUtils::prefilterCubeMap(skyTexture, 128);

			// Audio
			if (!audioFile.load("res/audio/bounce.wav")) {
				throw std::runtime_error("Error reading the audio file");
			}

			mScene.repository.add(std::string("sound"), std::make_shared<se::audio::Buffer>(
				audioFile.samples[0].data(), audioFile.samples[0].size() * sizeof(float),
				se::audio::FormatId::MonoFloat, audioFile.getSampleRate()
			));

			// Forces
			mScene.repository.add<std::string, se::physics::Force>("gravity", std::make_shared<se::physics::Gravity>(glm::vec3(0.0f, -9.8f, 0.0f)));
		}
		catch (std::exception& e) {
			SOMBRA_ERROR_LOG << "Error: " << e.what();
			return;
		}

		auto gBufferRenderer = static_cast<se::graphics::Renderer*>(mGame.getExternalTools().graphicsEngine->getRenderGraph().getNode("gBufferRenderer"));
		auto forwardRenderer = static_cast<se::graphics::Renderer*>(mGame.getExternalTools().graphicsEngine->getRenderGraph().getNode("forwardRenderer"));
		auto shadowRenderer = static_cast<se::graphics::Renderer*>(mGame.getExternalTools().graphicsEngine->getRenderGraph().getNode("shadowRenderer"));
		auto programSky = mScene.repository.find<std::string, se::graphics::Program>("programSky");
		auto programShadow = mScene.repository.find<std::string, se::graphics::Program>("programShadow");
		auto programShadowSkinning = mScene.repository.find<std::string, se::graphics::Program>("programShadowSkinning");
		auto programShadowTerrain = mScene.repository.find<std::string, se::graphics::Program>("programShadowTerrain");
		auto programGBufMaterial = mScene.repository.find<std::string, se::graphics::Program>("programGBufMaterial");
		auto programGBufSplatmap = mScene.repository.find<std::string, se::graphics::Program>("programGBufSplatmap");

		auto passShadow = mScene.repository.find<std::string, se::graphics::Pass>("passShadow");
		auto passShadowSkinning = mScene.repository.find<std::string, se::graphics::Pass>("passShadowSkinning");

		auto gravity = mScene.repository.find<std::string, se::physics::Force>("gravity");

		// Renderable2Ds
		auto technique2D = mGame.getRepository().find<std::string, se::graphics::Technique>("technique2D");
		mLogoTexture = new se::graphics::RenderableSprite({ 1060.0f, 20.0f }, { 200.0f, 200.0f }, glm::vec4(1.0f), logoTexture);
		mLogoTexture->addTechnique(technique2D);
		mLogoTexture->setZIndex(255);
		mGame.getExternalTools().graphicsEngine->addRenderable(mLogoTexture);
		mReticleTexture = new se::graphics::RenderableSprite({ kWidth / 2.0f - 10.0f, kHeight / 2.0f - 10.0f }, { 20.0f, 20.0f }, glm::vec4(1.0f, 1.0f, 1.0f, 0.6f), reticleTexture);
		mReticleTexture->addTechnique(technique2D);
		mReticleTexture->setZIndex(255);
		mGame.getExternalTools().graphicsEngine->addRenderable(mReticleTexture);
		mPickText = new se::graphics::RenderableText({ 0.0f, 700.0f }, { 16.0f, 16.0f }, arial, { 0.0f, 1.0f, 0.0f, 1.0f });
		mPickText->addTechnique(technique2D);
		mPickText->setZIndex(255);
		mGame.getExternalTools().graphicsEngine->addRenderable(mPickText);

		/*********************************************************************
		 * GAME DATA
		 *********************************************************************/
		// Player
		{
			mPlayerEntity = mGame.getEntityDatabase().addEntity();
			mScene.entities.push_back(mPlayerEntity);

			mGame.getEntityDatabase().emplaceComponent<se::app::TagComponent>(mPlayerEntity, "player");

			se::app::TransformsComponent transforms;
			transforms.position = glm::vec3(0.0f, 1.0f, 10.0f);
			transforms.orientation = glm::quat(glm::vec3(0.0f, glm::pi<float>(), 0.0f));
			mGame.getEntityDatabase().addComponent(mPlayerEntity, std::move(transforms));

			se::physics::RigidBodyConfig config(0.001f);
			config.invertedMass = 1.0f / 40.0f;	// No inertia tensor so the player can't rotate due to collisions
			config.linearDrag = 0.01f;
			config.angularDrag = 0.01f;
			config.frictionCoefficient = 1.16f;
			se::physics::RigidBody rigidBody(config, se::physics::RigidBodyData());
			mGame.getEntityDatabase().addComponent(mPlayerEntity, std::move(rigidBody));

			auto collider = std::make_unique<se::collision::BoundingSphere>(0.5f);
			mGame.getEntityDatabase().addComponent<se::collision::Collider>(mPlayerEntity, std::move(collider));

			se::app::CameraComponent camera;
			camera.setPerspectiveProjectionMatrix(glm::radians(kFOV), kWidth / static_cast<float>(kHeight), kZNear, kZFar);
			mGame.getEntityDatabase().addComponent(mPlayerEntity, std::move(camera));

			auto spotLight = std::make_shared<se::app::LightSource>(se::app::LightSource::Type::Spot);
			spotLight->intensity = 5.0f;
			spotLight->range = 20.0f;
			spotLight->innerConeAngle = glm::pi<float>() / 12.0f;
			spotLight->outerConeAngle = glm::pi<float>() / 6.0f;
			mScene.repository.add(std::string("spotLight"), spotLight);
			mGame.getEntityDatabase().addComponent(mPlayerEntity, se::app::LightComponent{ spotLight });

			mGame.getEventManager().publish(new se::app::ContainerEvent<se::app::Topic::Camera, se::app::Entity>(mPlayerEntity));
		}

		// Sky
		{
			auto skyEntity = mGame.getEntityDatabase().addEntity();
			mScene.entities.push_back(skyEntity);

			mGame.getEntityDatabase().emplaceComponent<se::app::TagComponent>(skyEntity, "sky");

			se::app::TransformsComponent transforms;
			transforms.scale = glm::vec3(kZFar / 2.0f);
			mGame.getEntityDatabase().addComponent(skyEntity, std::move(transforms));

			auto passSky = std::make_shared<se::graphics::Pass>(*forwardRenderer);
			skyTexture->setTextureUnit(0);
			passSky->addBindable(programSky)
				.addBindable(skyTexture)
				.addBindable(std::make_shared<se::graphics::UniformVariableValue<int>>("uCubeMap", *programSky, 0))
				.addBindable(std::make_shared<se::graphics::CullingOperation>(false));

			auto techniqueSky = std::make_unique<se::graphics::Technique>();
			techniqueSky->addPass(passSky);

			se::app::MeshComponent mesh;
			mesh.rMeshes.emplace_back(cubeMesh).addTechnique(std::move(techniqueSky));
			mGame.getEntityDatabase().addComponent(skyEntity, std::move(mesh));

			se::app::LightProbe lightProbe = { environmentTexture, prefilterTexture };
			mGame.getEntityDatabase().addComponent(skyEntity, std::move(lightProbe));
		}

		// Terrain
		{
			float size = 500.0f, maxHeight = 10.0f;
			std::vector<float> lodDistances{ 2000.0f, 1000.0f, 500.0f, 250.0f, 125.0f, 75.0f, 40.0f, 20.0f, 10.0f, 0.0f };

			se::app::SplatmapMaterial terrainMaterial;
			terrainMaterial.splatmapTexture = std::move(splatmapTexture);
			terrainMaterial.materials.push_back({ se::app::PBRMetallicRoughness{ { 0.5f, 0.25f, 0.1f, 1.0f }, {}, 0.2f, 0.5f, {} }, {}, 1.0f });
			terrainMaterial.materials.push_back({ se::app::PBRMetallicRoughness{ { 0.1f, 0.75f, 0.25f, 1.0f }, {}, 0.2f, 0.5f, {} }, {}, 1.0f });
			terrainMaterial.materials.push_back({ se::app::PBRMetallicRoughness{ { 0.1f, 0.25f, 0.75f, 1.0f }, {}, 0.2f, 0.5f, {} }, {}, 1.0f });

			auto terrainShadowPass = std::make_shared<se::graphics::Pass>(*shadowRenderer);
			terrainShadowPass->addBindable(programShadowTerrain);
			se::app::TechniqueLoader::addHeightMapBindables(terrainShadowPass, heightMap1, size, maxHeight, programShadowTerrain);

			auto terrainPass = std::make_shared<se::graphics::Pass>(*gBufferRenderer);
			terrainPass->addBindable(programGBufSplatmap);
			se::app::TechniqueLoader::addHeightMapBindables(terrainPass, heightMap1, size, maxHeight, programGBufSplatmap);
			se::app::TechniqueLoader::addSplatmapMaterialBindables(terrainPass, terrainMaterial, programGBufSplatmap);

			auto terrainTechnique = std::make_shared<se::graphics::Technique>();
			terrainTechnique->addPass(terrainShadowPass)
				.addPass(terrainPass);
			mScene.repository.add(std::string("techniqueSplatmap"), terrainTechnique);

			terrainLoader.createTerrain("terrain", size, maxHeight, heightMap1, lodDistances, "techniqueSplatmap");
		}

		// Plane
		{
			auto plane = mGame.getEntityDatabase().addEntity();
			mScene.entities.push_back(plane);

			mGame.getEntityDatabase().emplaceComponent<se::app::TagComponent>(plane, "plane");

			se::app::TransformsComponent transforms;
			transforms.position = glm::vec3(-15.0f, 1.0f, -5.0f);
			mGame.getEntityDatabase().addComponent(plane, std::move(transforms));

			auto passPlane = std::make_shared<se::graphics::Pass>(*gBufferRenderer);
			passPlane->addBindable(programGBufMaterial);
			se::app::TechniqueLoader::addMaterialBindables(
				passPlane,
				se::app::Material{
					se::app::PBRMetallicRoughness{ glm::vec4(1.0f), {}, 0.2f, 0.5f, {} },
					{}, 1.0f, {}, 1.0f, chessTexture, glm::vec3(1.0f), se::graphics::AlphaMode::Opaque, 0.5f, true
				},
				programGBufMaterial
			);

			auto techniquePlane = std::make_unique<se::graphics::Technique>();
			techniquePlane->addPass(passShadow)
				.addPass(passPlane);

			se::app::MeshComponent mesh;
			mesh.rMeshes.emplace_back(planeMesh).addTechnique(std::move(techniquePlane));
			mGame.getEntityDatabase().addComponent(plane, std::move(mesh));
		}

		// Fixed cubes
		glm::vec3 cubePositions[5] = {
			{ 2.0f, 5.0f, -10.0f },
			{ 0.0f, 7.0f, -10.0f },
			{ 0.0f, 5.0f, -8.0f },
			{ 0.0f, 5.0f, -10.0f },
			{ 10.0f, 5.0f, -10.0f }
		};
		glm::vec4 colors[5] = { { 1.0f, 0.2f, 0.2f, 1.0f }, { 0.2f, 1.0f, 0.2f, 1.0f }, { 0.2f, 0.2f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.2f, 0.2f, 0.1f, 1.0f } };
		se::app::Entity e1, e2;
		for (std::size_t i = 0; i < 5; ++i) {
			auto cube = mGame.getEntityDatabase().addEntity();
			mScene.entities.push_back(cube);

			mGame.getEntityDatabase().emplaceComponent<se::app::TagComponent>(cube, "non-random-cube-" + std::to_string(i));

			se::app::TransformsComponent transforms;
			transforms.position = cubePositions[i];

			se::physics::RigidBodyConfig config(20.0f, 2.0f / 5.0f * 10.0f * glm::pow(2.0f, 2.0f) * glm::mat3(1.0f), 0.001f);
			config.linearDrag = 0.95f;
			config.angularDrag = 0.95f;
			config.frictionCoefficient = 0.5f;
			se::physics::RigidBody rigidBody(config, se::physics::RigidBodyData());
			if (i == 1) {
				e1 = cube;
			}
			if (i == 2) {
				se::audio::Source source1;
				source1.bind(*mScene.repository.find<std::string, se::audio::Buffer>("sound"));
				source1.setLooping(true);
				source1.play();
				mGame.getEntityDatabase().addComponent(cube, std::move(source1));
			}
			if (i == 3) {
				rigidBody.getData().angularVelocity = glm::vec3(0.0f, 10.0f, 0.0f);
				e2 = cube;
			}
			if (i == 4) {
				transforms.velocity += glm::vec3(-1, 0, 0);
			}

			mGame.getEntityDatabase().addComponent(cube, std::move(transforms));
			mGame.getEntityDatabase().addComponent(cube, std::move(rigidBody));

			auto collider = std::make_unique<se::collision::BoundingBox>(glm::vec3(1.0f, 1.0f, 1.0f));
			mGame.getEntityDatabase().addComponent<se::collision::Collider>(cube, std::move(collider));

			auto passCube = std::make_shared<se::graphics::Pass>(*gBufferRenderer);
			passCube->addBindable(programGBufMaterial);
			se::app::TechniqueLoader::addMaterialBindables(
				passCube,
				se::app::Material{
					se::app::PBRMetallicRoughness{ colors[i], {}, 0.9f, 0.1f, {} },
					{}, 1.0f, {}, 1.0f, {}, glm::vec3(0.0f), se::graphics::AlphaMode::Opaque, 0.5f, false
				},
				programGBufMaterial
			);

			auto techniqueCube = std::make_unique<se::graphics::Technique>();
			techniqueCube->addPass(passShadow)
				.addPass(passCube);

			se::app::MeshComponent mesh;
			mesh.rMeshes.emplace_back(cubeMesh).addTechnique(std::move(techniqueCube));
			mGame.getEntityDatabase().addComponent(cube, std::move(mesh));
		}

		auto [rb1] = mGame.getEntityDatabase().getComponents<se::physics::RigidBody>(e1);
		auto [rb2] = mGame.getEntityDatabase().getComponents<se::physics::RigidBody>(e2);
		std::shared_ptr<se::physics::Constraint> constraint(new se::physics::DistanceConstraint({ rb1, rb2 }));
		mScene.repository.add(std::string("distance"), constraint);
		mGame.getExternalTools().physicsEngine->getConstraintManager().addConstraint(constraint.get());

		auto passRed = std::make_shared<se::graphics::Pass>(*gBufferRenderer);
		passRed->addBindable(programGBufMaterial);
		se::app::TechniqueLoader::addMaterialBindables(
			passRed,
			se::app::Material{
				se::app::PBRMetallicRoughness{ { 1.0f, 0.0f, 0.0f, 1.0f }, {}, 0.2f, 0.5f, {} },
				{}, 1.0f, {}, 1.0f, {}, glm::vec3(0.0f), se::graphics::AlphaMode::Opaque, 0.5f, false
			},
			programGBufMaterial
		);

		auto techniqueRed = std::make_shared<se::graphics::Technique>();
		techniqueRed->addPass(passShadow)
			.addPass(passRed);

		{
			auto nonMovableCube = mGame.getEntityDatabase().addEntity();
			mScene.entities.push_back(nonMovableCube);

			mGame.getEntityDatabase().emplaceComponent<se::app::TagComponent>(nonMovableCube, "non-movable-cube");

			se::app::TransformsComponent transforms;
			transforms.position = glm::vec3(-50.0f, 0.0f, -40.0f);
			transforms.scale = glm::vec3(10.0f, 1.0f, 10.0f);
			mGame.getEntityDatabase().addComponent(nonMovableCube, std::move(transforms));

			se::physics::RigidBodyConfig config(0.001f);
			config.frictionCoefficient = 0.75f;
			se::physics::RigidBody rigidBody(config, se::physics::RigidBodyData());
			mGame.getEntityDatabase().addComponent(nonMovableCube, std::move(rigidBody));

			auto collider = std::make_unique<se::collision::BoundingBox>(glm::vec3(1.0f));
			mGame.getEntityDatabase().addComponent<se::collision::Collider>(nonMovableCube, std::move(collider));

			se::app::MeshComponent mesh;
			mesh.rMeshes.emplace_back(cubeMesh).addTechnique(techniqueRed);
			mGame.getEntityDatabase().addComponent(nonMovableCube, std::move(mesh));
		}

		{
			auto gravityCube = mGame.getEntityDatabase().addEntity();
			mScene.entities.push_back(gravityCube);

			mGame.getEntityDatabase().emplaceComponent<se::app::TagComponent>(gravityCube, "gravity-cube");

			se::app::TransformsComponent transforms;
			transforms.position = glm::vec3(-50.0f, 2.0f, -40.0f);
			mGame.getEntityDatabase().addComponent(gravityCube, std::move(transforms));

			se::physics::RigidBodyConfig config(20.0f, 2.0f / 5.0f * 10.0f * glm::pow(2.0f, 2.0f) * glm::mat3(1.0f), 0.001f);
			config.linearDrag = 0.95f;
			config.angularDrag = 0.95f;
			config.frictionCoefficient = 0.65f;
			se::physics::RigidBody rigidBody(config, se::physics::RigidBodyData());
			mGame.getEntityDatabase().addComponent(gravityCube, std::move(rigidBody));
			auto [rb] = mGame.getEntityDatabase().getComponents<se::physics::RigidBody>(gravityCube);
			mGame.getExternalTools().physicsEngine->getForceManager().addRBForce(rb, gravity.get());

			auto collider3 = std::make_unique<se::collision::BoundingBox>(glm::vec3(1.0f));
			mGame.getEntityDatabase().addComponent<se::collision::Collider>(gravityCube, std::move(collider3));

			se::app::MeshComponent mesh;
			mesh.rMeshes.emplace_back(cubeMesh).addTechnique(techniqueRed);
			mGame.getEntityDatabase().addComponent(gravityCube, std::move(mesh));
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

			auto tubeSlice = mGame.getEntityDatabase().addEntity();
			mScene.entities.push_back(tubeSlice);

			mGame.getEntityDatabase().emplaceComponent<se::app::TagComponent>(tubeSlice, "tubeSlice");

			se::app::TransformsComponent transforms;
			transforms.orientation = glm::normalize(glm::quat(-1, glm::vec3(1.0f, 0.0f, 0.0f)));
			transforms.position = glm::vec3(0.0f, 2.0f, 75.0f) + displacement;
			mGame.getEntityDatabase().addComponent(tubeSlice, std::move(transforms));

			auto passSlice = std::make_shared<se::graphics::Pass>(*gBufferRenderer);
			passSlice->addBindable(programGBufMaterial);
			se::app::TechniqueLoader::addMaterialBindables(
				passSlice,
				se::app::Material{
					se::app::PBRMetallicRoughness{
						glm::vec4(glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f), 1.0f),
						{}, 0.2f, 0.5f, {}
					},
					{}, 1.0f, {}, 1.0f, {}, glm::vec3(0.0f), se::graphics::AlphaMode::Opaque, 0.5f, false
				},
				programGBufMaterial
			);

			auto techniqueSlice = std::make_shared<se::graphics::Technique>();
			techniqueSlice->addPass(passShadow)
				.addPass(passSlice);

			auto tmpRawMesh = se::app::MeshLoader::createRawMesh(heMesh, normals).first;
			auto tmpGraphicsMesh = std::make_shared<se::graphics::Mesh>(se::app::MeshLoader::createGraphicsMesh(tmpRawMesh));
			se::app::MeshComponent mesh;
			mesh.rMeshes.emplace_back(tmpGraphicsMesh).addTechnique(techniqueSlice);
			mGame.getEntityDatabase().addComponent(tubeSlice, std::move(mesh));
		}

		// Random cubes
		{
			auto passRandom = std::make_shared<se::graphics::Pass>(*gBufferRenderer);
			passRandom->addBindable(programGBufMaterial);
			se::app::TechniqueLoader::addMaterialBindables(
				passRandom,
				se::app::Material{
					se::app::PBRMetallicRoughness{ { 0.0f, 0.0f, 1.0f, 1.0f }, {}, 0.2f, 0.5f, {} },
					{}, 1.0f, {}, 1.0f, {}, glm::vec3(0.0f), se::graphics::AlphaMode::Opaque, 0.5f, false
				},
				programGBufMaterial
			);

			auto techniqueRandom = std::make_shared<se::graphics::Technique>();
			techniqueRandom->addPass(passShadow)
				.addPass(passRandom);

			for (std::size_t i = 0; i < kNumCubes; ++i) {
				auto cube = mGame.getEntityDatabase().addEntity();
				mScene.entities.push_back(cube);

				mGame.getEntityDatabase().emplaceComponent<se::app::TagComponent>(cube, "random-cube-" + std::to_string(i));

				se::app::TransformsComponent transforms;
				transforms.position = glm::ballRand(50.0f);
				mGame.getEntityDatabase().addComponent(cube, std::move(transforms));

				se::physics::RigidBodyConfig config(10.0f, 2.0f / 5.0f * 10.0f * glm::pow(2.0f, 2.0f) * glm::mat3(1.0f), 0.001f);
				config.linearDrag = 0.9f;
				config.angularDrag = 0.9f;
				config.frictionCoefficient = 0.5f;
				se::physics::RigidBody rigidBody(config, se::physics::RigidBodyData());
				mGame.getEntityDatabase().addComponent(cube, std::move(rigidBody));
				auto [rb] = mGame.getEntityDatabase().getComponents<se::physics::RigidBody>(cube);
				mGame.getExternalTools().physicsEngine->getForceManager().addRBForce(rb, gravity.get());

				auto collider = std::make_unique<se::collision::BoundingBox>(glm::vec3(1.0f, 1.0f, 1.0f));
				mGame.getEntityDatabase().addComponent<se::collision::Collider>(cube, std::move(collider));

				se::app::MeshComponent mesh;
				mesh.rMeshes.emplace_back(cubeMesh).addTechnique(techniqueRandom);
				mGame.getEntityDatabase().addComponent(cube, std::move(mesh));
			}
		}

		setHandleInput(true);
	}


	Level::~Level()
	{
		setHandleInput(false);

		mGame.getExternalTools().graphicsEngine->removeRenderable(mLogoTexture);
		delete mLogoTexture;
		mGame.getExternalTools().graphicsEngine->removeRenderable(mReticleTexture);
		delete mReticleTexture;
		mGame.getExternalTools().graphicsEngine->removeRenderable(mPickText);
		delete mPickText;

		mGame.getEventManager().unsubscribe(this, se::app::Topic::Key);
	}


	void Level::update(float deltaTime)
	{
		if (mPlayerController) {
			mPlayerController->update(deltaTime);
		}
	}


	void Level::notify(const se::app::IEvent& event)
	{
		// Check if we shouldn't handle the input
		if (mPlayerController) {
			tryCall(&Level::onKeyEvent, event);
		}
	}


	void Level::setHandleInput(bool handle)
	{
		if (handle && !mPlayerController) {
			mPlayerController = new PlayerController(*this, *mPickText);
			mPlayerController->resetMousePosition();
			mGame.getExternalTools().windowManager->setCursorVisibility(false);
		}

		if (!handle && mPlayerController) {
			mPlayerController->resetMousePosition();
			delete mPlayerController;
			mPlayerController = nullptr;

			mGame.getExternalTools().windowManager->setCursorVisibility(true);
		}
	}

// Private functions
	void Level::onKeyEvent(const se::app::KeyEvent& event)
	{
		if ((event.getKeyCode() == SE_KEY_ESCAPE) && (event.getState() != se::app::KeyEvent::State::Released)) {
			mGame.getStateMachine().submitEvent(static_cast<se::utils::StateMachine::Event>(GameEvent::AddGameMenu));
		}
	}

}
