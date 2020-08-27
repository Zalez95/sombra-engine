#include "se/utils/Log.h"
#include "se/graphics/Technique.h"
#include "se/graphics/core/Program.h"
#include "se/graphics/3D/RenderableTerrain.h"
#include "se/app/CameraSystem.h"
#include "se/app/Application.h"
#include "se/app/EntityDatabase.h"
#include "se/app/graphics/MeshComponent.h"
#include "se/app/TransformsComponent.h"
#include "se/app/graphics/CameraComponent.h"

namespace se::app {

	CameraSystem::CameraSystem(Application& application) :
		ISystem(application.getEntityDatabase()), mApplication(application),
		mCameraEntity(kNullEntity), mCameraUpdated(false)
	{
		mApplication.getEventManager().subscribe(this, Topic::Camera);
		mEntityDatabase.addSystem(this, EntityDatabase::ComponentMask()
			.set<CameraComponent>()
			.set<MeshComponent>()
			.set<graphics::RenderableTerrain>()
		);
	}


	CameraSystem::~CameraSystem()
	{
		mEntityDatabase.removeSystem(this);
		mApplication.getEventManager().unsubscribe(this, Topic::Camera);
	}


	void CameraSystem::notify(const IEvent& event)
	{
		tryCall(&CameraSystem::onCameraEvent, event);
	}


	void CameraSystem::onNewEntity(Entity entity)
	{
		auto [transforms, camera, mesh, rTerrain] = mEntityDatabase.getComponents<
			TransformsComponent, CameraComponent, MeshComponent, graphics::RenderableTerrain
		>(entity);

		if (camera) {
			if (transforms) {
				// The Camera initial data is overridden by the entity one
				camera->setPosition(transforms->position);
				camera->setTarget(transforms->position + glm::vec3(0.0f, 0.0f, 1.0f) * transforms->orientation);
				camera->setUp({ 0.0f, 1.0f, 0.0f });
			}

			SOMBRA_INFO_LOG << "Entity " << entity << " with Camera " << camera << " added successfully";
		}

		std::vector<std::size_t> passDataIndices;
		if (mesh) {
			for (auto& rMesh : mesh->rMeshes) {
				processPasses(rMesh, passDataIndices);
			}
		}
		if (rTerrain) {
			processPasses(*rTerrain, passDataIndices);
		}
		mEntityPasses.emplace(entity, std::move(passDataIndices));
	}


	void CameraSystem::onRemoveEntity(Entity entity)
	{
		if (mCameraEntity == entity) {
			mCameraEntity = kNullEntity;
			mCameraUpdated = true;
			SOMBRA_INFO_LOG << "Active Camera Entity " << entity << " removed";
		}

		auto it = mEntityPasses.find(entity);
		if (it != mEntityPasses.end()) {
			for (auto iPass : it->second) {
				mPassesData[iPass].userCount--;
				if (mPassesData[iPass].userCount == 0) {
					mPassesData.erase(mPassesData.begin().setIndex(iPass));
				}
			}

			mEntityPasses.erase(it);
		}

		SOMBRA_INFO_LOG << "Entity " << entity << " removed successfully";
	}


	void CameraSystem::update()
	{
		SOMBRA_DEBUG_LOG << "Updating the Cameras";

		mEntityDatabase.iterateComponents<TransformsComponent, CameraComponent>(
			[&](Entity entity, TransformsComponent* transforms, CameraComponent* camera) {
				if (transforms->updated.any()) {
					camera->setPosition(transforms->position);
					camera->setTarget(transforms->position + glm::vec3(0.0f, 0.0f, 1.0f) * transforms->orientation);
					camera->setUp({ 0.0f, 1.0f, 0.0f });

					if (mCameraEntity == entity) {
						mCameraUpdated = true;
					}
				}
			}
		);

		if (mCameraUpdated) {
			auto [camera] = mEntityDatabase.getComponents<CameraComponent>(mCameraEntity);
			if (camera) {
				for (auto& passData : mPassesData) {
					passData.viewMatrix->setValue(camera->getViewMatrix());
					passData.projectionMatrix->setValue(camera->getProjectionMatrix());
				}
			}

			mCameraUpdated = false;
		}

		SOMBRA_INFO_LOG << "Update end";
	}

// Private functions
	void CameraSystem::onCameraEvent(const ContainerEvent<Topic::Camera, Entity>& event)
	{
		mCameraEntity = event.getValue();
		mCameraUpdated = true;
	}


	void CameraSystem::processPasses(graphics::Renderable3D& renderable, std::vector<std::size_t>& output)
	{
		CameraComponent* activeCamera = std::get<0>(mEntityDatabase.getComponents<CameraComponent>(mCameraEntity));

		renderable.processTechniques([&](auto technique) { technique->processPasses([&](auto pass) {
			auto it = std::find_if(mPassesData.begin(), mPassesData.end(), [&](const auto& passData) { return passData.pass == pass; });
			if (it == mPassesData.end()) {
				std::shared_ptr<graphics::Program> program;
				pass->processBindables([&](const auto& bindable) {
					if (auto tmp = std::dynamic_pointer_cast<graphics::Program>(bindable)) {
						program = tmp;
					}
				});

				if (program) {
					it = mPassesData.emplace();
					it->userCount++;
					it->pass = pass;
					it->viewMatrix = std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uViewMatrix", *program);
					it->projectionMatrix = std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uProjectionMatrix", *program);
					if (activeCamera) {
						it->viewMatrix->setValue(activeCamera->getViewMatrix());
						it->projectionMatrix->setValue(activeCamera->getProjectionMatrix());
					}

					pass->addBindable(it->viewMatrix)
						.addBindable(it->projectionMatrix);
					output.push_back(it.getIndex());
				}
				else {
					SOMBRA_WARN_LOG << "Renderable3D has a Pass " << pass << " with no program";
				}
			}
			else {
				it->userCount++;
				output.push_back(it.getIndex());
			}
		}); });
	}

}
