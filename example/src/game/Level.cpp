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
#include <se/app/io/MeshLoader.h>
#include <se/app/io/ImageReader.h>
#include <se/app/io/FontReader.h>
#include <se/app/io/SceneSerializer.h>
#include <se/app/io/ShaderLoader.h>
#include <se/app/CameraSystem.h>
#include <se/app/AudioSystem.h>
#include <se/app/AppRenderer.h>
#include <se/app/TagComponent.h>
#include <se/app/MeshComponent.h>
#include "se/app/TerrainComponent.h"
#include <se/app/CameraComponent.h>
#include <se/app/LightComponent.h>
#include <se/app/LightProbeComponent.h>
#include <se/app/ParticleSystemComponent.h>
#include <se/app/TransformsComponent.h>
#include <se/app/RigidBodyComponent.h>
#include <se/app/AudioSourceComponent.h>
#include <se/app/Scene.h>

#include <se/graphics/Renderer.h>
#include <se/graphics/3D/RenderableMesh.h>
#include <se/graphics/core/UniformVariable.h>
#include <se/graphics/core/GraphicsOperations.h>

#include <se/physics/collision/BoundingBox.h>
#include <se/physics/collision/BoundingSphere.h>
#include <se/physics/collision/ConvexPolyhedron.h>
#include <se/physics/collision/CompositeCollider.h>
#include "se/physics/collision/TerrainCollider.h"
#include <se/physics/collision/HalfEdgeMeshExt.h>
#include <se/physics/collision/QuickHull.h>
#include <se/physics/collision/HACD.h>
#include <se/physics/RigidBodyWorld.h>
#include <se/physics/forces/Gravity.h>
#include <se/physics/constraints/DistanceConstraint.h>

#include <se/animation/AnimationEngine.h>
#include <se/animation/SkeletonAnimator.h>

#include <se/audio/Buffer.h>

#include <se/utils/Log.h>
#include <se/app/Repository.h>

#include "Game.h"
#include "Level.h"
#include "PlayerController.h"

namespace game {

	se::physics::HalfEdgeMesh createTestTube1()
	{
		se::physics::HalfEdgeMesh meshData;
		std::array<int, 48> vertexIndices = {
			se::physics::addVertex(meshData, { -0.000000014f, 0.499999761f, -1.0f }),
			se::physics::addVertex(meshData, { -0.000000014f, 0.499999761f, 1.0f }),
			se::physics::addVertex(meshData, { 0.249999970f, 0.433012485f, -1.0f }),
			se::physics::addVertex(meshData, { 0.249999970f, 0.433012485f, 1.0f }),
			se::physics::addVertex(meshData, { 0.433012694f, 0.249999791f, -1.0f }),
			se::physics::addVertex(meshData, { 0.433012694f, 0.249999791f, 1.0f }),
			se::physics::addVertex(meshData, { 0.5f, -0.000000210f, -1.0f }),
			se::physics::addVertex(meshData, { 0.5f, -0.000000210f, 1.0f }),
			se::physics::addVertex(meshData, { 0.433012694f, -0.250000208f, -1.0f }),
			se::physics::addVertex(meshData, { 0.433012694f, -0.250000208f, 1.0f }),
			se::physics::addVertex(meshData, { 0.250000029f, -0.433012902f, -1.0f }),
			se::physics::addVertex(meshData, { 0.250000029f, -0.433012902f, 1.0f }),
			se::physics::addVertex(meshData, { 0.00000006f, -0.500000178f, -1.0f }),
			se::physics::addVertex(meshData, { 0.00000006f, -0.500000178f, 1.0f }),
			se::physics::addVertex(meshData, { -0.249999910f, -0.433012962f, -1.0f }),
			se::physics::addVertex(meshData, { -0.249999910f, -0.433012962f, 1.0f }),
			se::physics::addVertex(meshData, { -0.433012634f, -0.250000357f, -1.0f }),
			se::physics::addVertex(meshData, { -0.433012634f, -0.250000357f, 1.0f }),
			se::physics::addVertex(meshData, { -0.5f, -0.000000421f, -1.0f }),
			se::physics::addVertex(meshData, { -0.5f, -0.000000421f, 1.0f }),
			se::physics::addVertex(meshData, { -0.433012872f, 0.249999567f, -1.0f }),
			se::physics::addVertex(meshData, { -0.433012872f, 0.249999567f, 1.0f }),
			se::physics::addVertex(meshData, { -0.250000327f, 0.433012336f, -1.0f }),
			se::physics::addVertex(meshData, { -0.250000327f, 0.433012336f, 1.0f }),
			se::physics::addVertex(meshData, { 0.0f, 1.0f, -1.0f }),
			se::physics::addVertex(meshData, { 0.0f, 1.0f, 1.0f }),
			se::physics::addVertex(meshData, { 0.5f, 0.866025388f, -1.0f }),
			se::physics::addVertex(meshData, { 0.5f, 0.866025388f, 1.0f }),
			se::physics::addVertex(meshData, { 0.866025447f, 0.499999970f, -1.0f }),
			se::physics::addVertex(meshData, { 0.866025447f, 0.499999970f, 1.0f }),
			se::physics::addVertex(meshData, { 1.0f, -0.000000043f, -1.0f }),
			se::physics::addVertex(meshData, { 1.0f, -0.000000043f, 1.0f }),
			se::physics::addVertex(meshData, { 0.866025388f, -0.500000059f, -1.0f }),
			se::physics::addVertex(meshData, { 0.866025388f, -0.500000059f, 1.0f }),
			se::physics::addVertex(meshData, { 0.500000059f, -0.866025388f, -1.0f }),
			se::physics::addVertex(meshData, { 0.500000059f, -0.866025388f, 1.0f }),
			se::physics::addVertex(meshData, { 0.00000015f, -1.0f, -1.0f }),
			se::physics::addVertex(meshData, { 0.00000015f, -1.0f, 1.0f }),
			se::physics::addVertex(meshData, { -0.499999791f, -0.866025507f, -1.0f }),
			se::physics::addVertex(meshData, { -0.499999791f, -0.866025507f, 1.0f }),
			se::physics::addVertex(meshData, { -0.866025209f, -0.500000298f, -1.0f }),
			se::physics::addVertex(meshData, { -0.866025209f, -0.500000298f, 1.0f }),
			se::physics::addVertex(meshData, { -1.0f, -0.000000464f, -1.0f }),
			se::physics::addVertex(meshData, { -1.0f, -0.000000464f, 1.0f }),
			se::physics::addVertex(meshData, { -0.866025686f, 0.499999493f, -1.0f }),
			se::physics::addVertex(meshData, { -0.866025686f, 0.499999493f, 1.0f }),
			se::physics::addVertex(meshData, { -0.500000596f, 0.866025090f, -1.0f }),
			se::physics::addVertex(meshData, { -0.500000596f, 0.866025090f, 1.0f })
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
		mLogoTexture(nullptr), mReticleTexture(nullptr), mPickText(nullptr)
	{
		mGame.getEventManager().subscribe(this, se::app::Topic::Key);

		/*********************************************************************
		 * GRAPHICS DATA
		 *********************************************************************/
		se::physics::QuickHull qh(0.0001f);
		se::physics::HACD hacd(0.002f, 0.0002f);

		auto gBufferRendererMesh = static_cast<se::graphics::Renderer*>(mGame.getExternalTools().graphicsEngine->getRenderGraph().getNode("gBufferRendererMesh"));
		auto forwardRendererMesh = static_cast<se::graphics::Renderer*>(mGame.getExternalTools().graphicsEngine->getRenderGraph().getNode("forwardRendererMesh"));

		se::app::Image<unsigned char> heightMap1, splatMap1, logo1, reticle1;
		se::app::Image<float> environment1;
		se::app::Repository::ResourceRef<se::graphics::Mesh> cubeMesh = nullptr, planeMesh = nullptr;
		se::app::Repository::ResourceRef<se::graphics::Texture> logoTexture, reticleTexture, chessTexture, skyTexture;
		se::app::Repository::ResourceRef<se::graphics::Font> arial;
		se::app::Repository::ResourceRef<se::graphics::Technique> technique2D;
		se::app::Repository::ResourceRef<se::graphics::Program> programSky, programShadow, programShadowSkinning, programGBufMaterial;
		se::app::Repository::ResourceRef<se::app::RenderableShaderStep> stepShadow;
		se::app::Repository::ResourceRef<se::app::RenderableShader> shaderSky, shaderPlane, shaderRandom;
		se::app::Repository::ResourceRef<se::audio::Buffer> sound;
		se::app::Repository::ResourceRef<se::app::LightSource> spotLight;
		se::app::Repository::ResourceRef<se::physics::Force> gravity;
		se::app::Repository::ResourceRef<se::app::Script> playerController;

		try {
			// Scene
			auto result = se::app::SceneSerializer::deserialize("res/map.se", mScene);
			if (!result) {
				throw std::runtime_error(result.description());
			}

			// Fonts
			arial = mGame.getRepository().findByName<se::graphics::Font>("arial");
			if (!arial) {
				throw std::runtime_error("Arial font not found");
			}

			// Images
			result = se::app::ImageReader::read("res/images/logo.png", logo1);
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
			logoTexture = mGame.getRepository().insert(std::make_shared<se::graphics::Texture>(se::graphics::TextureTarget::Texture2D), "logo");
			logoTexture.setFakeUser();
			logoTexture->setImage(
				logo1.pixels.get(), se::graphics::TypeId::UnsignedByte, se::graphics::ColorFormat::RGBA, se::graphics::ColorFormat::RGBA,
				logo1.width, logo1.height
			);

			reticleTexture = mGame.getRepository().insert(std::make_shared<se::graphics::Texture>(se::graphics::TextureTarget::Texture2D), "reticle");
			reticleTexture.setFakeUser();
			reticleTexture->setImage(
				reticle1.pixels.get(), se::graphics::TypeId::UnsignedByte, se::graphics::ColorFormat::RGBA, se::graphics::ColorFormat::RGBA,
				reticle1.width, reticle1.height
			);

			chessTexture = mScene.repository.findByName<se::graphics::Texture>("chessTexture");

			auto environmentEquiTexture = std::make_shared<se::graphics::Texture>(se::graphics::TextureTarget::Texture2D);
			environmentEquiTexture->setWrapping(se::graphics::TextureWrap::ClampToEdge, se::graphics::TextureWrap::ClampToEdge)
				.setFiltering(se::graphics::TextureFilter::Linear, se::graphics::TextureFilter::Linear)
				.setImage(
					environment1.pixels.get(), se::graphics::TypeId::Float, se::graphics::ColorFormat::RGB,
					se::graphics::ColorFormat::RGB, environment1.width, environment1.height
				);
			skyTexture = mScene.repository.insert(se::app::TextureUtils::equirectangularToCubeMap(environmentEquiTexture, 512), "skyTexture");

			// Meshes
			se::app::RawMesh cubeRawMesh = se::app::MeshLoader::createBoxMesh("Cube", glm::vec3(1.0f));
			cubeRawMesh.normals = se::app::MeshLoader::calculateNormals(cubeRawMesh.positions, cubeRawMesh.indices);
			cubeRawMesh.tangents = se::app::MeshLoader::calculateTangents(cubeRawMesh.positions, cubeRawMesh.texCoords, cubeRawMesh.indices);
			cubeMesh = mScene.repository.insert(std::make_shared<se::graphics::Mesh>(se::app::MeshLoader::createGraphicsMesh(cubeRawMesh)), "cube");

			se::app::RawMesh planeRawMesh("Plane");
			planeRawMesh.positions = { {-0.5f,-0.5f, 0.0f}, { 0.5f,-0.5f, 0.0f}, {-0.5f, 0.5f, 0.0f}, { 0.5f, 0.5f, 0.0f} };
			planeRawMesh.texCoords = { {0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f} };
			planeRawMesh.indices = { 0, 1, 2, 1, 3, 2, };
			planeRawMesh.normals = se::app::MeshLoader::calculateNormals(planeRawMesh.positions, planeRawMesh.indices);
			planeRawMesh.tangents = se::app::MeshLoader::calculateTangents(planeRawMesh.positions, planeRawMesh.texCoords, planeRawMesh.indices);
			planeMesh = mScene.repository.insert(std::make_shared<se::graphics::Mesh>(se::app::MeshLoader::createGraphicsMesh(planeRawMesh)), "plane");

			// Programs
			programShadow = mScene.repository.findByName<se::graphics::Program>("programShadow");
			programShadowSkinning = mScene.repository.findByName<se::graphics::Program>("programShadowSkinning");
			programGBufMaterial = mScene.repository.findByName<se::graphics::Program>("programGBufMaterial");

			std::shared_ptr<se::graphics::Program> programSkySPtr;
			result = se::app::ShaderLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, "res/shaders/fragmentSkyBox.glsl", programSkySPtr);
			if (!result) {
				throw std::runtime_error("programSky error: " + std::string(result.description()));
			}
			programSky = mScene.repository.insert(std::move(programSkySPtr), "programSky");

			// Techniques
			technique2D = mGame.getRepository().findByName<se::graphics::Technique>("technique2D");
			if (!technique2D) {
				throw std::runtime_error("Error reading the audio file");
			}

			// Shaders
			stepShadow = mScene.repository.findByName<se::app::RenderableShaderStep>("stepShadow");

			auto stepSky = mScene.repository.insert(std::make_shared<se::app::RenderableShaderStep>(*forwardRendererMesh), "stepSky");
			skyTexture->setTextureUnit(0);
			stepSky->addResource(programSky)
				.addResource(skyTexture)
				.addBindable(std::make_shared<se::graphics::SetOperation>(se::graphics::Operation::DepthTest, true))
				.addBindable(std::make_shared<se::graphics::SetOperation>(se::graphics::Operation::Culling, false))
				.addBindable(std::make_shared<se::graphics::UniformVariableValue<int>>("uCubeMap", programSky.get(), 0));

			shaderSky = mScene.repository.insert(std::make_shared<se::app::RenderableShader>(mGame.getEventManager()), "shaderSky");
			shaderSky->addStep(stepSky);

			auto stepPlane = mScene.repository.insert(std::make_shared<se::app::RenderableShaderStep>(*gBufferRendererMesh), "stepPlane");
			se::app::ShaderLoader::addMaterialBindables(
				stepPlane,
				se::app::Material{
					se::app::PBRMetallicRoughness{ glm::vec4(1.0f), {}, 0.2f, 0.5f, {} },
					{}, 1.0f, {}, 1.0f, chessTexture, glm::vec3(1.0f), se::graphics::AlphaMode::Opaque, 0.5f, true
				},
				programGBufMaterial
			);

			shaderPlane = mScene.repository.insert(std::make_shared<se::app::RenderableShader>(mGame.getEventManager()), "shaderPlane");
			shaderPlane->addStep(stepShadow)
				.addStep(stepPlane);

			auto stepRandom = mScene.repository.insert(std::make_shared<se::app::RenderableShaderStep>(*gBufferRendererMesh), "stepRandom");
			se::app::ShaderLoader::addMaterialBindables(
				stepRandom,
				se::app::Material{
					se::app::PBRMetallicRoughness{ { 0.0f, 0.0f, 1.0f, 1.0f }, {}, 0.2f, 0.5f, {} },
					{}, 1.0f, {}, 1.0f, {}, glm::vec3(0.0f), se::graphics::AlphaMode::Opaque, 0.5f, false
				},
				programGBufMaterial
			);

			shaderRandom = mScene.repository.insert(std::make_shared<se::app::RenderableShader>(mGame.getEventManager()), "shaderRandom");
			shaderRandom->addStep(stepShadow)
				.addStep(stepRandom);

			// Audio
			AudioFile<float> audioFile;
			if (!audioFile.load("res/audio/bounce.wav")) {
				throw std::runtime_error("Error reading the audio file");
			}

			sound = mScene.repository.insert(
				std::make_shared<se::audio::Buffer>(
					audioFile.samples[0].data(), audioFile.samples[0].size() * sizeof(float),
					se::audio::FormatId::MonoFloat, audioFile.getSampleRate()
				),
				"sound"
			);
			sound.setFakeUser();

			// Lights
			spotLight = mScene.repository.insert(std::make_shared<se::app::LightSource>(mGame.getEventManager(), se::app::LightSource::Type::Spot), "spotLight");

			// Forces
			gravity = mScene.repository.findByName<se::physics::Force>("gravity");
			gravity.setFakeUser();

			// Renderable2Ds
			mLogoTexture = new se::graphics::RenderableSprite({ 1060.0f, 20.0f }, { 200.0f, 200.0f }, glm::vec4(1.0f), logoTexture.get());
			mLogoTexture->addTechnique(technique2D.get());
			mLogoTexture->setZIndex(255);

			mReticleTexture = new se::graphics::RenderableSprite({ kWidths[0] / 2.0f - 10.0f, kHeights[0] / 2.0f - 10.0f }, { 20.0f, 20.0f }, glm::vec4(1.0f, 1.0f, 1.0f, 0.6f), reticleTexture.get());
			mReticleTexture->addTechnique(technique2D.get());
			mReticleTexture->setZIndex(255);

			mPickText = new se::graphics::RenderableText({ 0.0f, 700.0f }, { 16.0f, 16.0f }, arial.get(), { 0.0f, 1.0f, 0.0f, 1.0f });
			mPickText->addTechnique(technique2D.get());
			mPickText->setZIndex(255);

			// Scripts
			auto playerControllerSPtr = std::make_shared<PlayerController>(*this, *mPickText);
			playerController = mScene.repository.insert<se::app::Script>(std::move(playerControllerSPtr), "playerController");
		}
		catch (std::exception& e) {
			SOMBRA_ERROR_LOG << "Error: " << e.what();
			return;
		}

		mGame.getExternalTools().graphicsEngine->addRenderable(mLogoTexture);
		mGame.getExternalTools().graphicsEngine->addRenderable(mReticleTexture);
		mGame.getExternalTools().graphicsEngine->addRenderable(mPickText);

		/*********************************************************************
		 * GAME DATA
		 *********************************************************************/
		// Player
		mGame.getEntityDatabase().executeQuery([&](se::app::EntityDatabase::Query& query) {
			mPlayerEntity = query.addEntity();
			mScene.entities.push_back(mPlayerEntity);

			query.emplaceComponent<se::app::TagComponent>(mPlayerEntity, true, "player");

			se::app::TransformsComponent transforms;
			transforms.position = glm::vec3(0.0f, 1.0f, 10.0f);
			query.addComponent(mPlayerEntity, std::move(transforms));

			se::physics::RigidBodyProperties properties(1.0f, glm::mat3(1.0f));
			properties.invertedInertiaTensor = glm::mat3(0.0f);	// Prevent the player from rotating due to collisions
			properties.linearDrag = 0.99f;
			properties.angularDrag = 0.99f;
			properties.frictionCoefficient = 1.16f;

			auto collider = std::make_unique<se::physics::BoundingSphere>(0.5f);

			se::app::RigidBodyComponent rbComponent(properties);
			rbComponent.get().setCollider(std::move(collider));

			query.addComponent(mPlayerEntity, std::move(rbComponent));

			auto scriptComponent = query.emplaceComponent<se::app::ScriptComponent>(mPlayerEntity);
			scriptComponent->setScript(playerController);

			se::app::CameraComponent camera;
			camera.setPerspectiveProjection(glm::radians(kFOV), kWidths[0] / static_cast<float>(kHeights[0]), kZNear, kZFar);
			query.addComponent(mPlayerEntity, std::move(camera));

			spotLight->setIntensity(5.0f);
			spotLight->setRange(20.0f);
			spotLight->setSpotLightRange(glm::pi<float>() / 12.0f, glm::pi<float>() / 6.0f);
			se::app::LightComponent lightComponent;
			lightComponent.setSource(spotLight);
			query.addComponent(mPlayerEntity, std::move(lightComponent));

			mGame.getEventManager().publish(std::make_unique<se::app::ContainerEvent<se::app::Topic::Camera, se::app::Entity>>(mPlayerEntity));
		});

		mGame.getEntityDatabase().executeQuery([&](se::app::EntityDatabase::Query& query) {
			query.iterateComponents<se::app::LightComponent>([](se::app::LightComponent& light) {
				auto source = light.getSource();
				if (source->getType() == se::app::LightSource::Type::Directional) {
					source->setShadows();
				}
			}, true);
		});

		// Sky
		mGame.getEntityDatabase().executeQuery([&](se::app::EntityDatabase::Query& query) {
			auto skyEntity = query.addEntity();
			mScene.entities.push_back(skyEntity);

			query.emplaceComponent<se::app::TagComponent>(skyEntity, true, "sky");

			se::app::TransformsComponent transforms;
			transforms.scale = glm::vec3(kZFar / 2.0f);
			query.addComponent(skyEntity, std::move(transforms));

			auto mesh = query.emplaceComponent<se::app::MeshComponent>(skyEntity);
			auto rIndex = mesh->add(false, cubeMesh);
			mesh->addRenderableShader(rIndex, std::move(shaderSky));
		});

		// Plane
		mGame.getEntityDatabase().executeQuery([&](se::app::EntityDatabase::Query& query) {
			auto plane = query.addEntity();
			mScene.entities.push_back(plane);

			query.emplaceComponent<se::app::TagComponent>(plane, true, "plane");

			se::app::TransformsComponent transforms;
			transforms.position = glm::vec3(-15.0f, 1.0f, -5.0f);
			query.addComponent(plane, std::move(transforms));

			auto mesh = query.emplaceComponent<se::app::MeshComponent>(plane);
			auto rIndex = mesh->add(false, planeMesh);
			mesh->addRenderableShader(rIndex, std::move(shaderPlane));
		});

		// Fixed cubes
		glm::vec3 cubePositions[5] = {
			{ 2.0f, 5.0f, -10.0f },
			{ 0.0f, 7.0f, -10.0f },
			{ 0.0f, 5.0f, -8.0f },
			{ 0.0f, 5.0f, -10.0f },
			{ 10.0f, 5.0f, -10.0f }
		};
		glm::vec4 colors[5] = { { 1.0f, 0.2f, 0.2f, 1.0f }, { 0.2f, 1.0f, 0.2f, 1.0f }, { 0.2f, 0.2f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.2f, 0.2f, 0.1f, 1.0f } };
		se::app::Entity e1 = se::app::kNullEntity, e2 = se::app::kNullEntity;
		for (std::size_t i = 0; i < 5; ++i) {
			mGame.getEntityDatabase().executeQuery([&](se::app::EntityDatabase::Query& query) {
				auto cube = query.addEntity();
				mScene.entities.push_back(cube);

				query.emplaceComponent<se::app::TagComponent>(cube, true, "non-random-cube-" + std::to_string(i));

				se::app::TransformsComponent transforms;
				transforms.position = cubePositions[i];
				query.addComponent(cube, std::move(transforms));

				se::physics::RigidBodyProperties properties(20.0f, 2.0f / 5.0f * 10.0f * glm::pow(2.0f, 2.0f) * glm::mat3(1.0f));
				properties.linearDrag = 0.05f;
				properties.angularDrag = 0.05f;
				properties.frictionCoefficient = 0.5f;

				se::physics::RigidBodyState state;

				if (i == 1) {
					e1 = cube;
				}
				if (i == 2) {
					se::app::AudioSourceComponent source1;
					//source1.setBuffer(sound);
					source1.get().setLooping(true);
					source1.get().play();
					query.addComponent(cube, std::move(source1));
				}
				if (i == 3) {
					state.angularVelocity = glm::vec3(0.0f, 10.0f, 0.0f);
					e2 = cube;
				}
				if (i == 4) {
					transforms.velocity += glm::vec3(-1, 0, 0);
				}

				auto collider = std::make_unique<se::physics::BoundingBox>(glm::vec3(1.0f, 1.0f, 1.0f));

				se::app::RigidBodyComponent rbComponent(properties, state);
				rbComponent.get().setCollider(std::move(collider));
				query.addComponent(cube, std::move(rbComponent));

				auto stepCube = mScene.repository.insert(std::make_shared<se::app::RenderableShaderStep>(*gBufferRendererMesh), ("stepCube" + std::to_string(i)).c_str());
				se::app::ShaderLoader::addMaterialBindables(
					stepCube,
					se::app::Material{
						se::app::PBRMetallicRoughness{ colors[i], {}, 0.9f, 0.1f, {} },
						{}, 1.0f, {}, 1.0f, {}, glm::vec3(0.0f), se::graphics::AlphaMode::Opaque, 0.5f, false
					},
					programGBufMaterial
				);

				auto shaderCube = mScene.repository.insert(std::make_shared<se::app::RenderableShader>(mGame.getEventManager()), ("shaderCube" + std::to_string(i)).c_str());
				shaderCube->addStep(stepShadow)
					.addStep(stepCube);

				auto mesh = query.emplaceComponent<se::app::MeshComponent>(cube);
				auto rIndex = mesh->add(false, cubeMesh);
				mesh->addRenderableShader(rIndex, std::move(shaderCube));
			});
		}

		mGame.getEntityDatabase().executeQuery([&](se::app::EntityDatabase::Query& query) {
			auto [rb1] = query.getComponents<se::app::RigidBodyComponent>(e1);
			auto [rb2] = query.getComponents<se::app::RigidBodyComponent>(e2);
			mGame.getExternalTools().rigidBodyWorld->getConstraintManager().addConstraint(new se::physics::DistanceConstraint({ &rb1->get(), &rb2->get() }));
		});

		// HACD Tube
		std::size_t iSlice = 0;
		se::physics::HalfEdgeMesh tube = createTestTube1();
		glm::vec3 tubeCentroid = se::physics::calculateCentroid(tube);
		hacd.calculate(tube);
		for (const auto& [heMesh, normals] : hacd.getMeshes()) {
			glm::vec3 sliceCentroid = se::physics::calculateCentroid(heMesh);
			glm::vec3 displacement = sliceCentroid - tubeCentroid;
			if (glm::length(displacement) > 0.0f) {
				displacement = glm::normalize(displacement);
			}
			displacement *= 0.1f;

			mGame.getEntityDatabase().executeQuery([&](se::app::EntityDatabase::Query& query) {
				auto tubeSlice = query.addEntity();
				mScene.entities.push_back(tubeSlice);

				std::string name = "tubeSlice" + std::to_string(iSlice);
				query.emplaceComponent<se::app::TagComponent>(tubeSlice, true, name.c_str());

				se::app::TransformsComponent transforms;
				transforms.orientation = glm::normalize(glm::quat(-1, glm::vec3(1.0f, 0.0f, 0.0f)));
				transforms.position = glm::vec3(0.0f, 2.0f, 75.0f) + displacement;
				query.addComponent(tubeSlice, std::move(transforms));

				auto stepSlice = mScene.repository.insert(std::make_shared<se::app::RenderableShaderStep>(*gBufferRendererMesh), ("step" + name).c_str());
				se::app::ShaderLoader::addMaterialBindables(
					stepSlice,
					se::app::Material{
						se::app::PBRMetallicRoughness{
							glm::vec4(glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f), 1.0f),
							{}, 0.2f, 0.5f, {}
						},
						{}, 1.0f, {}, 1.0f, {}, glm::vec3(0.0f), se::graphics::AlphaMode::Opaque, 0.5f, false
					},
					programGBufMaterial
				);

				auto shaderSlice = mScene.repository.insert(std::make_shared<se::app::RenderableShader>(mGame.getEventManager()), ("shader" + name).c_str());
				shaderSlice->addStep(stepShadow)
					.addStep(stepSlice);

				auto tmpRawMesh = se::app::MeshLoader::createRawMesh(heMesh, normals).first;
				auto sliceMesh = mScene.repository.insert(std::make_shared<se::graphics::Mesh>(se::app::MeshLoader::createGraphicsMesh(tmpRawMesh)), ("mesh" + name).c_str());
				auto mesh = query.emplaceComponent<se::app::MeshComponent>(tubeSlice);
				auto rIndex = mesh->add(false, sliceMesh);
				mesh->addRenderableShader(rIndex, std::move(shaderSlice));
			});

			iSlice++;
		}

		// Random cubes
		for (std::size_t i = 0; i < kNumCubes; ++i) {
			mGame.getEntityDatabase().executeQuery([&](se::app::EntityDatabase::Query& query) {
				auto cube = query.addEntity();
				mScene.entities.push_back(cube);

				query.emplaceComponent<se::app::TagComponent>(cube, true, "random-cube-" + std::to_string(i));

				se::app::TransformsComponent transforms;
				transforms.position = glm::ballRand(50.0f) + glm::vec3(0.0f, 50.0f, 0.0f);
				query.addComponent(cube, std::move(transforms));

				se::physics::RigidBodyProperties properties(10.0f, 2.0f / 5.0f * 10.0f * glm::pow(2.0f, 2.0f) * glm::mat3(1.0f));
				properties.linearDrag = 0.1f;
				properties.angularDrag = 0.1f;
				properties.frictionCoefficient = 0.5f;

				auto collider = std::make_unique<se::physics::BoundingBox>(glm::vec3(1.0f, 1.0f, 1.0f));

				se::app::RigidBodyComponent rbComponent(properties);
				rbComponent.get().setCollider(std::move(collider));
				rbComponent.addForce(gravity);
				query.addComponent(cube, std::move(rbComponent));

				auto mesh = query.emplaceComponent<se::app::MeshComponent>(cube);
				auto rIndex = mesh->add(false, cubeMesh);
				mesh->addRenderableShader(rIndex, shaderRandom);
			});
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

		mScene.repository.findByName<se::physics::Force>("gravity").setFakeUser(false);
		mScene.repository.findByName<se::audio::Buffer>("sound").setFakeUser(false);
		mGame.getRepository().findByName<se::graphics::Texture>("reticle").setFakeUser(false);
		mGame.getRepository().findByName<se::graphics::Texture>("logo").setFakeUser(false);

		mGame.getEventManager().unsubscribe(this, se::app::Topic::Key);
	}


	bool Level::notify(const se::app::IEvent& event)
	{
		return tryCall(&Level::onKeyEvent, event);
	}


	void Level::setHandleInput(bool handle)
	{
		mGame.getEntityDatabase().executeQuery([&](se::app::EntityDatabase::Query& query) {
			bool hasControl = query.hasComponentsEnabled<se::app::ScriptComponent>(mPlayerEntity);

			if (handle) {
				mGame.getExternalTools().windowManager->setCursorMode(se::window::CursorMode::Camera);
				if (!hasControl) {
					query.enableComponent<se::app::ScriptComponent>(mPlayerEntity);
				}
			}
			else if (!handle) {
				mGame.getExternalTools().windowManager->setCursorMode(se::window::CursorMode::Normal);
				if (hasControl) {
					query.disableComponent<se::app::ScriptComponent>(mPlayerEntity);
				}
			}
		});
	}

// Private functions
	void Level::onKeyEvent(const se::app::KeyEvent& event)
	{
		if ((event.getKeyCode() == SE_KEY_ESCAPE) && (event.getState() != se::app::KeyEvent::State::Released)) {
			mGame.getStateMachine().submitEvent(static_cast<se::utils::StateMachine::Event>(GameEvent::AddGameMenu));
		}
	}

}
