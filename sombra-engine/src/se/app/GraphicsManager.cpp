#include <array>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include "se/app/GraphicsManager.h"
#include "se/app/events/ResizeEvent.h"
#include "se/app/graphics/VoxelizationNode.h"
#include "se/app/graphics/Tex3DViewerNode.h"
#include "se/app/graphics/Tex3DClearNode.h"
#include "se/utils/Log.h"
#include "se/graphics/Pass.h"
#include "se/graphics/Technique.h"
#include "se/graphics/FBClearNode.h"
#include "se/graphics/2D/Renderer2D.h"
#include "se/graphics/3D/Renderer3D.h"
#include "se/graphics/core/Program.h"
#include "se/graphics/core/Texture.h"
#include "se/graphics/core/FrameBuffer.h"
#include "se/graphics/core/GraphicsOperations.h"

namespace se::app {

	struct GraphicsManager::Impl
	{
		struct ShaderBaseLight
		{
			enum class Type : unsigned int { DirectionalLight = 0, PointLight, SpotLight };

			unsigned int type;		float padding1[3];
			glm::vec4 color;
			float intensity;
			float inverseRange;
			float lightAngleScale;
			float lightAngleOffset;
		};

		struct RenderableMeshData
		{
			RenderableMeshUPtr renderable;
			std::vector<std::shared_ptr<graphics::UniformVariableValue<glm::mat4>>> modelMatrix;
			SkinSPtr skin;
			std::vector<std::shared_ptr<graphics::UniformVariableValueVector<glm::mat4, kMaxJoints>>> jointMatrices;

			RenderableMeshData(RenderableMeshUPtr renderable) :
				renderable(std::move(renderable)), skin(nullptr) {};
		};

		struct RenderableTerrainData
		{
			RenderableTerrainUPtr renderable;
			std::vector<std::shared_ptr<graphics::UniformVariableValue<glm::mat4>>> modelMatrix;

			RenderableTerrainData(RenderableTerrainUPtr renderable) :
				renderable(std::move(renderable)) {};
		};

		struct PassData
		{
			std::shared_ptr<graphics::Pass> pass;
			std::shared_ptr<graphics::Program> program;
			std::shared_ptr<graphics::UniformVariableValue<glm::mat4>> viewMatrix;
			std::shared_ptr<graphics::UniformVariableValue<glm::mat4>> projectionMatrix;
			std::shared_ptr<graphics::UniformVariableValue<unsigned int>> numLights;
			std::shared_ptr<graphics::UniformVariableValueVector<glm::vec3, kMaxLights>> lightsPositions;
			std::shared_ptr<graphics::UniformVariableValueVector<glm::vec3, kMaxLights>> lightsDirections;
			std::shared_ptr<graphics::UniformBlock> lightsBlock;
		};

		std::map<Entity*, CameraUPtr> cameraEntities;
		std::map<Entity*, LightUPtr> lightEntities;
		std::multimap<Entity*, RenderableMeshData> renderableMeshEntities;
		std::map<Entity*, RenderableTerrainData> renderableTerrainEntities;

		Camera* activeCamera;
		std::shared_ptr<graphics::UniformBuffer> lightsBuffer;
		std::vector<PassData> passesData;
	};


	GraphicsManager::GraphicsManager(graphics::GraphicsEngine& graphicsEngine, EventManager& eventManager) :
		mGraphicsEngine(graphicsEngine), mEventManager(eventManager)
	{
		mEventManager.subscribe(this, Topic::Resize);

		SOMBRA_INFO_LOG << graphics::GraphicsOperations::getGraphicsInfo();

		mImpl = std::make_unique<Impl>();

		static constexpr int numVoxels = 128;

		{
			auto resources = dynamic_cast<graphics::BindableRenderNode*>(mGraphicsEngine.getRenderGraph().getNode("resources"));

			auto buffer = std::make_unique<unsigned int[]>(numVoxels * numVoxels * numVoxels);
			auto texture3D = std::make_unique<graphics::Texture>(graphics::TextureTarget::Texture3D);
			texture3D->setTextureUnit(Tex3DViewerNode::kTextureUnit)
				.setImageUnit(VoxelizationNode::kVoxelImageUnit)
				.setWrapping(graphics::TextureWrap::ClampToEdge, graphics::TextureWrap::ClampToEdge, graphics::TextureWrap::ClampToEdge)
				.setImage(
					buffer.get(), graphics::TypeId::UnsignedInt, graphics::ColorFormat::RedInteger,
					graphics::ColorFormat::Red32ui, numVoxels, numVoxels, numVoxels
				)
				.generateMipMap();
			auto texture3DIndex = resources->addBindable( std::move(texture3D) );
			resources->addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::Texture>>("texture3D", resources, texture3DIndex) );
		}

		{
			mGraphicsEngine.getRenderGraph().addNode( std::make_unique<graphics::FBClearNode>("defaultFBClear", true, true) );
		}

		{
			mGraphicsEngine.getRenderGraph().addNode( std::make_unique<Tex3DClearNode>("tex3DClear", *this, numVoxels) );
		}

		{
			auto voxelization = std::make_unique<VoxelizationNode>("voxelization", *this, numVoxels);
			voxelization->setSceneBounds(glm::vec3(-25.0f), glm::vec3(25.0f));
			mGraphicsEngine.getRenderGraph().addNode( std::move(voxelization) );
		}

		{
			auto tex3DViewer = std::make_unique<Tex3DViewerNode>("tex3DViewer", *this, numVoxels);
			tex3DViewer->setSceneBounds(glm::vec3(-25.0f), glm::vec3(25.0f));

			auto& passData = mImpl->passesData.emplace_back();
			passData.program = mProgramRepository.find("programTex3DViewer");
			passData.viewMatrix = std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uViewMatrix", *passData.program);
			passData.projectionMatrix = std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uProjectionMatrix", *passData.program);

			tex3DViewer->addBindable(passData.viewMatrix);
			tex3DViewer->addBindable(passData.projectionMatrix);

			mGraphicsEngine.getRenderGraph().addNode( std::move(tex3DViewer) );
		}

		{
			auto renderer3D = std::make_unique<graphics::Renderer3D>("renderer3D");
			auto targetIndex = renderer3D->addBindable();
			renderer3D->addInput( std::make_unique<graphics::BindableRNodeInput<graphics::FrameBuffer>>("target", renderer3D.get(), targetIndex) );
			renderer3D->addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::FrameBuffer>>("target", renderer3D.get(), targetIndex) );
			mGraphicsEngine.getRenderGraph().addNode( std::move(renderer3D) );
		}

		{
			auto renderer2D = std::make_unique<graphics::Renderer2D>("renderer2D");
			auto targetIndex = renderer2D->addBindable();
			renderer2D->addInput( std::make_unique<graphics::BindableRNodeInput<graphics::FrameBuffer>>("target", renderer2D.get(), targetIndex) );
			renderer2D->addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::FrameBuffer>>("target", renderer2D.get(), targetIndex) );
			mGraphicsEngine.getRenderGraph().addNode( std::move(renderer2D) );
		}

		{
			auto resources = mGraphicsEngine.getRenderGraph().getNode("resources");
			auto defaultFBClear = mGraphicsEngine.getRenderGraph().getNode("defaultFBClear");
			auto renderer3D = mGraphicsEngine.getRenderGraph().getNode("renderer3D");
			auto renderer2D = mGraphicsEngine.getRenderGraph().getNode("renderer2D");
			auto tex3DClear = mGraphicsEngine.getRenderGraph().getNode("tex3DClear");
			auto voxelization = mGraphicsEngine.getRenderGraph().getNode("voxelization");
			auto tex3DViewer = mGraphicsEngine.getRenderGraph().getNode("tex3DViewer");

			defaultFBClear->findInput("input")->connect( resources->findOutput("defaultFB") );
			renderer3D->findInput("target")->connect( defaultFBClear->findOutput("output") );
			tex3DViewer->findInput("target")->connect( renderer3D->findOutput("target") );
			renderer2D->findInput("target")->connect( tex3DViewer->findOutput("target") );

			tex3DClear->findInput("input")->connect( resources->findOutput("texture3D") );
			voxelization->findInput("texture3D")->connect( tex3DClear->findOutput("output") );
			tex3DViewer->findInput("texture3D")->connect( voxelization->findOutput("texture3D") );
		}

		// Reserve memory for the UniformBuffers
		mImpl->lightsBuffer = std::make_shared<graphics::UniformBuffer>();
		utils::FixedVector<Impl::ShaderBaseLight, kMaxLights> lightsBufferData(kMaxLights);
		mImpl->lightsBuffer->resizeAndCopy(lightsBufferData.data(), lightsBufferData.size());
	}


	GraphicsManager::~GraphicsManager()
	{
		mEventManager.unsubscribe(this, Topic::Resize);
	}


	void GraphicsManager::notify(const IEvent& event)
	{
		tryCall(&GraphicsManager::onResizeEvent, event);
	}


	void GraphicsManager::addCameraEntity(Entity* entity, CameraUPtr camera)
	{
		if (!entity || !camera) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added as Camera";
			return;
		}

		// The Camera initial data is overridden by the entity one
		camera->setPosition(entity->position);
		camera->setTarget(entity->position + glm::vec3(0.0f, 0.0f, 1.0f) * entity->orientation);
		camera->setUp({ 0.0f, 1.0f, 0.0f });

		// Add the Camera
		Camera* cPtr = camera.get();
		mImpl->cameraEntities.emplace(entity, std::move(camera));
		mImpl->activeCamera = cPtr;
		SOMBRA_INFO_LOG << "Entity " << entity << " with Camera " << cPtr << " added successfully";
	}


	void GraphicsManager::addLightEntity(Entity* entity, LightUPtr light)
	{
		if (!entity || !light) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added as ILight";
			return;
		}

		// The PointLight initial data is overridden by the entity one
		ILight* lPtr = light.get();
		if (auto dLight = dynamic_cast<DirectionalLight*>(lPtr)) {
			dLight->direction = glm::vec3(0.0f, 0.0f, 1.0f) * entity->orientation;
		}
		else if (auto pLight = dynamic_cast<PointLight*>(lPtr)) {
			pLight->position = entity->position;
		}
		else if (auto sLight = dynamic_cast<SpotLight*>(lPtr)) {
			sLight->position = entity->position;
			sLight->direction = glm::vec3(0.0f, 0.0f, 1.0f) * entity->orientation;
		}

		// Add the ILight
		mImpl->lightEntities.emplace(entity, std::move(light));
		SOMBRA_INFO_LOG << "Entity " << entity << " with ILight " << lPtr << " added successfully";
	}


	GraphicsManager::PassSPtr GraphicsManager::createPass2D(ProgramSPtr program)
	{
		auto renderer2D = dynamic_cast<graphics::Renderer2D*>( mGraphicsEngine.getRenderGraph().getNode("renderer2D") );
		if (!renderer2D) {
			return nullptr;
		}

		auto pass = std::make_shared<graphics::Pass>(*renderer2D);
		pass->addBindable(program)
			.addBindable(std::make_shared<graphics::BlendingOperation>(true))
			.addBindable(std::make_shared<graphics::DepthTestOperation>(false));

		for (int i = 0; i < static_cast<int>(graphics::Renderer2D::kMaxTextures); ++i) {
			utils::ArrayStreambuf<char, 64> aStreambuf;
			std::ostream(&aStreambuf) << "uTextures[" << i << "]";
			pass->addBindable(std::make_shared<graphics::UniformVariableValue<int>>(aStreambuf.data(), *program, i));
		}

		pass->addBindable(std::make_shared<graphics::UniformVariableCallback<glm::mat4>>(
			"uProjectionMatrix", *program,
			[]() {
				int x, y;
				std::size_t width, height;
				graphics::GraphicsOperations::getViewport(x, y, width, height);
				return glm::ortho(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, -1.0f, 1.0f);
			}
		));

		return pass;
	}


	GraphicsManager::PassSPtr GraphicsManager::createPass3D(ProgramSPtr program, bool hasLights)
	{
		auto renderer3D = dynamic_cast<graphics::Renderer3D*>( mGraphicsEngine.getRenderGraph().getNode("voxelization") );
		if (!renderer3D) {
			return nullptr;
		}

		auto pass = std::make_shared<graphics::Pass>(*renderer3D);
		auto& passData = mImpl->passesData.emplace_back();
		passData.pass = pass;
		passData.program = program;

		passData.viewMatrix = std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uViewMatrix", *program);
		passData.projectionMatrix = std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uProjectionMatrix", *program);
		if (mImpl->activeCamera) {
			passData.viewMatrix->setValue(mImpl->activeCamera->getViewMatrix());
			passData.projectionMatrix->setValue(mImpl->activeCamera->getProjectionMatrix());
		}

		pass->addBindable(program)
			.addBindable(passData.viewMatrix)
			.addBindable(passData.projectionMatrix);

		if (hasLights) {
			passData.numLights = std::make_shared<graphics::UniformVariableValue<unsigned int>>("uNumLights", *program);
			passData.lightsPositions = std::make_shared<graphics::UniformVariableValueVector<glm::vec3, kMaxLights>>("uLightsPositions", *program);
			passData.lightsDirections = std::make_shared<graphics::UniformVariableValueVector<glm::vec3, kMaxLights>>("uLightsDirections", *program);
			passData.lightsBlock = std::make_shared<graphics::UniformBlock>("LightsBlock", *program);

			pass->addBindable(mImpl->lightsBuffer)
				.addBindable(passData.numLights)
				.addBindable(passData.lightsPositions)
				.addBindable(passData.lightsDirections)
				.addBindable(passData.lightsBlock);
		}

		return pass;
	}


	void GraphicsManager::addMeshEntity(Entity* entity, RenderableMeshUPtr renderable, SkinSPtr skin)
	{
		if (!entity || !renderable) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added as Mesh";
			return;
		}

		graphics::RenderableMesh* rPtr = renderable.get();
		auto& meshData = mImpl->renderableMeshEntities.emplace(entity, std::move(renderable))->second;

		// Add the RenderableMesh model matrix uniform
		glm::mat4 translation	= glm::translate(glm::mat4(1.0f), entity->position);
		glm::mat4 rotation		= glm::mat4_cast(entity->orientation);
		glm::mat4 scale			= glm::scale(glm::mat4(1.0f), entity->scale);
		glm::mat4 modelMatrix	= translation * rotation * scale;

		rPtr->processTechniques([&](auto technique) { technique->processPasses([&](auto pass) {
			auto itPassData = std::find_if(mImpl->passesData.begin(), mImpl->passesData.end(), [&](const Impl::PassData& passData) {
				return passData.pass == pass;
			});
			if (itPassData != mImpl->passesData.end()) {
				rPtr->addBindable(
					meshData.modelMatrix.emplace_back(
						std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uModelMatrix", *itPassData->program, modelMatrix)
					)
				);

				if (skin) {
					auto jointMatrices = calculateJointMatrices(*skin, modelMatrix);
					std::size_t numJoints = std::min(jointMatrices.size(), static_cast<std::size_t>(kMaxJoints));

					rPtr->addBindable(
						meshData.jointMatrices.emplace_back(
							std::make_shared<graphics::UniformVariableValueVector<glm::mat4, kMaxJoints>>(
								"uJointMatrices", *itPassData->program, jointMatrices.data(), numJoints
							)
						)
					);
				}
			}
			else {
				SOMBRA_WARN_LOG << "RenderableMesh has a Pass " << pass << " not added to the GraphicsManager";
			}
		}); });

		mGraphicsEngine.addRenderable(rPtr);

		SOMBRA_INFO_LOG << "Entity " << entity << " with RenderableMesh " << rPtr << " added successfully";
	}


	void GraphicsManager::addTerrainEntity(Entity* entity, RenderableTerrainUPtr renderable)
	{
		if (!entity || !renderable) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added as RenderableTerrain";
			return;
		}

		graphics::RenderableTerrain* rPtr = renderable.get();
		auto pair = mImpl->renderableTerrainEntities.emplace(entity, std::move(renderable));
		if (!pair.second) {
			SOMBRA_WARN_LOG << "Entity " << entity << " already has a RenderableTerrain";
			return;
		}

		auto& terrainData = pair.first->second;
		if (mImpl->activeCamera) {
			rPtr->setHighestLodLocation(mImpl->activeCamera->getPosition());
		}

		// Add the RenderableTerrain model matrix uniform
		glm::mat4 translation	= glm::translate(glm::mat4(1.0f), entity->position);
		glm::mat4 rotation		= glm::mat4_cast(entity->orientation);
		glm::mat4 modelMatrix	= translation * rotation;

		rPtr->processTechniques([&](auto technique) { technique->processPasses([&](auto pass) {
			auto itPassData = std::find_if(mImpl->passesData.begin(), mImpl->passesData.end(), [&](const Impl::PassData& passData) {
				return passData.pass == pass;
			});
			if (itPassData != mImpl->passesData.end()) {
				rPtr->addBindable(
					terrainData.modelMatrix.emplace_back(
						std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uModelMatrix", *itPassData->program, modelMatrix)
					)
				);
			}
			else {
				SOMBRA_WARN_LOG << "RenderableTerrain has a Pass " << pass << " not added to the GraphicsManager";
			}
		}); });

		mGraphicsEngine.addRenderable(rPtr);

		SOMBRA_INFO_LOG << "Entity " << entity << " with RenderableTerrain " << rPtr << " added successfully";
	}


	void GraphicsManager::removeEntity(Entity* entity)
	{
		auto itCamera = mImpl->cameraEntities.find(entity);
		if (itCamera != mImpl->cameraEntities.end()) {
			if (itCamera->second.get() == mImpl->activeCamera) {
				mImpl->activeCamera = nullptr;
			}
			mImpl->cameraEntities.erase(itCamera);
			SOMBRA_INFO_LOG << "Camera Entity " << entity << " removed successfully";
		}

		auto itLight = mImpl->lightEntities.find(entity);
		if (itLight != mImpl->lightEntities.end()) {
			mImpl->lightEntities.erase(itLight);
			SOMBRA_INFO_LOG << "ILight Entity " << entity << " removed successfully";
		}

		auto [itRMeshBegin, itRMeshEnd] = mImpl->renderableMeshEntities.equal_range(entity);
		for (auto itRMesh = itRMeshBegin; itRMesh != itRMeshEnd;) {
			mGraphicsEngine.removeRenderable(itRMesh->second.renderable.get());
			itRMesh = mImpl->renderableMeshEntities.erase(itRMesh);
			SOMBRA_INFO_LOG << "Mesh Entity " << entity << " removed successfully";
		}

		auto itRenderableTerrain = mImpl->renderableTerrainEntities.find(entity);
		if (itRenderableTerrain != mImpl->renderableTerrainEntities.end()) {
			mGraphicsEngine.removeRenderable(itRenderableTerrain->second.renderable.get());
			mImpl->renderableTerrainEntities.erase(itRenderableTerrain);
			SOMBRA_INFO_LOG << "RenderableTerrain Entity " << entity << " removed successfully";
		}
	}


	void GraphicsManager::update()
	{
		SOMBRA_INFO_LOG << "Update start";

		SOMBRA_DEBUG_LOG << "Updating the Cameras";
		bool activeCameraUpdated = false;
		for (auto& [entity, camera] : mImpl->cameraEntities) {
			if (entity->updated.any()) {
				camera->setPosition(entity->position);
				camera->setTarget(entity->position + glm::vec3(0.0f, 0.0f, 1.0f) * entity->orientation);
				camera->setUp({ 0.0f, 1.0f, 0.0f });

				if (camera.get() == mImpl->activeCamera) {
					activeCameraUpdated = true;
				}
			}
		}

		SOMBRA_DEBUG_LOG << "Updating the ILights";
		bool lightsUpdated = false;
		for (auto& pair : mImpl->lightEntities) {
			Entity* entity = pair.first;
			ILight* light = pair.second.get();

			if (entity->updated.any()) {
				lightsUpdated = true;
				if (auto dLight = (dynamic_cast<DirectionalLight*>(light))) {
					dLight->direction = glm::vec3(0.0f, 0.0f, 1.0f) * entity->orientation;
				}
				else if (auto pLight = (dynamic_cast<PointLight*>(light))) {
					pLight->position = entity->position;
				}
				else if (auto sLight = (dynamic_cast<SpotLight*>(light))) {
					sLight->position = entity->position;
					sLight->direction = glm::vec3(0.0f, 0.0f, 1.0f) * entity->orientation;
				}
			}
		}

		SOMBRA_DEBUG_LOG << "Updating the Meshes";
		for (auto& [entity, meshData] : mImpl->renderableMeshEntities) {
			if (entity->updated.any()) {
				glm::mat4 translation	= glm::translate(glm::mat4(1.0f), entity->position);
				glm::mat4 rotation		= glm::mat4_cast(entity->orientation);
				glm::mat4 scale			= glm::scale(glm::mat4(1.0f), entity->scale);
				glm::mat4 modelMatrix	= translation * rotation * scale;

				for(auto& uniform : meshData.modelMatrix) {
					uniform->setValue(modelMatrix);
				}

				if (meshData.skin) {
					auto jointMatrices = calculateJointMatrices(*meshData.skin, modelMatrix);
					std::size_t numJoints = std::min(jointMatrices.size(), static_cast<std::size_t>(kMaxJoints));

					for(auto& uniform : meshData.jointMatrices) {
						uniform->setValue(jointMatrices.data(), numJoints);
					}
				}
			}
		}

		SOMBRA_DEBUG_LOG << "Updating the RenderableTerrains";
		for (auto& [entity, terrainData] : mImpl->renderableTerrainEntities) {
			if (entity->updated.any()) {
				glm::mat4 translation	= glm::translate(glm::mat4(1.0f), entity->position);
				glm::mat4 rotation		= glm::mat4_cast(entity->orientation);
				glm::mat4 modelMatrix	= translation * rotation;

				for(auto& uniform : terrainData.modelMatrix) {
					uniform->setValue(modelMatrix);
				}
			}

			if (activeCameraUpdated) {
				terrainData.renderable->setHighestLodLocation(mImpl->activeCamera->getPosition());
			}
		}

		SOMBRA_DEBUG_LOG << "Updating the Passes";
		if (activeCameraUpdated) {
			for (auto& passData : mImpl->passesData) {
				passData.viewMatrix->setValue(mImpl->activeCamera->getViewMatrix());
				passData.projectionMatrix->setValue(mImpl->activeCamera->getProjectionMatrix());
			}
		}

		if (lightsUpdated) {
			unsigned int uNumLights = std::min(static_cast<unsigned int>(mImpl->lightEntities.size()), kMaxLights);
			std::array<Impl::ShaderBaseLight, kMaxLights> uBaseLights;
			std::array<glm::vec3, kMaxLights> uLightsPositions;
			std::array<glm::vec3, kMaxLights> uLightsDirections;

			unsigned int i = 0;
			for (auto& pair : mImpl->lightEntities) {
				uBaseLights[i].color = { pair.second->color, 1.0f };
				uBaseLights[i].intensity = pair.second->intensity;
				if (auto dLight = dynamic_cast<const DirectionalLight*>(pair.second.get())) {
					uBaseLights[i].type = static_cast<unsigned int>(Impl::ShaderBaseLight::Type::DirectionalLight);
					uBaseLights[i].inverseRange = 0.0f;
					uLightsPositions[i] = glm::vec3(0.0f);
					uLightsDirections[i] = dLight->direction;
				}
				else if (auto pLight = dynamic_cast<const PointLight*>(pair.second.get())) {
					uBaseLights[i].type = static_cast<unsigned int>(Impl::ShaderBaseLight::Type::PointLight);
					uBaseLights[i].inverseRange = pLight->inverseRange;
					uLightsPositions[i] = pLight->position;
					uLightsDirections[i] = glm::vec3(0.0f);
				}
				else if (auto sLight = dynamic_cast<const SpotLight*>(pair.second.get())) {
					uBaseLights[i].type = static_cast<unsigned int>(Impl::ShaderBaseLight::Type::SpotLight);
					uBaseLights[i].inverseRange = sLight->inverseRange;
					uBaseLights[i].lightAngleScale = 1.0f / std::max(0.001f, std::cos(sLight->innerConeAngle) - std::cos(sLight->outerConeAngle));
					uBaseLights[i].lightAngleOffset = -std::cos(sLight->outerConeAngle) * uBaseLights[i].lightAngleScale;
					uLightsPositions[i] = sLight->position;
					uLightsDirections[i] = sLight->direction;
				}

				if (++i >= uNumLights) { break; }
			}

			mImpl->lightsBuffer->copy(uBaseLights.data(), uBaseLights.size());
			for (auto& passData : mImpl->passesData) {
				if (passData.lightsBlock) {
					passData.numLights->setValue(uNumLights);
					passData.lightsPositions->setValue(uLightsPositions.data(), uLightsPositions.size());
					passData.lightsDirections->setValue(uLightsDirections.data(), uLightsDirections.size());
				}
			}
		}

		SOMBRA_INFO_LOG << "Update end";
	}


	void GraphicsManager::render()
	{
		SOMBRA_INFO_LOG << "Render start";
		mGraphicsEngine.render();
		SOMBRA_INFO_LOG << "Render end";
	}

// Private functions
	void GraphicsManager::onResizeEvent(const ResizeEvent& event)
	{
		auto width = static_cast<std::size_t>(event.getWidth());
		auto height = static_cast<std::size_t>(event.getHeight());

		graphics::GraphicsOperations::setViewport(0, 0, width, height);
	}

}
