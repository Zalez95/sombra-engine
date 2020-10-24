#include "se/utils/Log.h"
#include "se/app/ScriptSystem.h"
#include "se/app/Application.h"
#include "se/app/EntityDatabase.h"
#include "se/app/ScriptComponent.h"

namespace se::app {

	ScriptSystem::ScriptSystem(Application& application) : ISystem(application.getEntityDatabase())
	{
		mEntityDatabase.addSystem(this, EntityDatabase::ComponentMask().set<ScriptComponent>());
	}


	ScriptSystem::~ScriptSystem()
	{
		mEntityDatabase.removeSystem(this);
	}


	void ScriptSystem::onNewEntity(Entity entity)
	{
		auto [script] = mEntityDatabase.getComponents<ScriptComponent>(entity);
		script->onCreate();

		SOMBRA_INFO_LOG << "Entity " << entity << " with Script " << script << " added successfully";
	}


	void ScriptSystem::onRemoveEntity(Entity entity)
	{
		auto [script] = mEntityDatabase.getComponents<ScriptComponent>(entity);
		script->onDestroy();

		SOMBRA_INFO_LOG << "Entity " << entity << " removed successfully";
	}


	void ScriptSystem::update()
	{
		SOMBRA_DEBUG_LOG << "Updating the Scripts";

		mEntityDatabase.iterateComponents<ScriptComponent>([this](Entity, ScriptComponent* script) {
			script->onUpdate(mDeltaTime);
		});

		SOMBRA_INFO_LOG << "Update end";
	}

}
