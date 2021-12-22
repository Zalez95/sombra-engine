#include <sol/sol.hpp>>
#include "se/utils/Log.h"
#include "se/app/ScriptSystem.h"
#include "se/app/Application.h"

namespace se::app {

	ScriptSystem::ScriptSystem(Application& application) :
		ISystem(application.getEntityDatabase()), mApplication(application)
	{
		mEntityDatabase.addSystem(this, EntityDatabase::ComponentMask().set<ScriptComponent>());
		mApplication.getEventManager().subscribe(this, Topic::Key);
		mApplication.getEventManager().subscribe(this, Topic::MouseMove);
		mApplication.getEventManager().subscribe(this, Topic::MouseScroll);
		mApplication.getEventManager().subscribe(this, Topic::MouseButton);
		mApplication.getEventManager().subscribe(this, Topic::WindowResize);
		mApplication.getEventManager().subscribe(this, Topic::Script);

		const auto& windowData = mApplication.getExternalTools().windowManager->getWindowData();
		mScriptSharedState.windowWidth = static_cast<float>(windowData.width);
		mScriptSharedState.windowHeight = static_cast<float>(windowData.height);
		mScriptSharedState.entityDatabase = &mApplication.getEntityDatabase();
		mScriptSharedState.eventManager = &mApplication.getEventManager();

		mLuaState = new sol::state();
		mLuaState->open_libraries(sol::lib::base);
	}


	ScriptSystem::~ScriptSystem()
	{
		mApplication.getEventManager().unsubscribe(this, Topic::Script);
		mApplication.getEventManager().unsubscribe(this, Topic::WindowResize);
		mApplication.getEventManager().unsubscribe(this, Topic::MouseButton);
		mApplication.getEventManager().unsubscribe(this, Topic::MouseScroll);
		mApplication.getEventManager().unsubscribe(this, Topic::MouseMove);
		mApplication.getEventManager().unsubscribe(this, Topic::Key);
		mEntityDatabase.removeSystem(this);

		delete mLuaState;
	}


	bool ScriptSystem::notify(const IEvent& event)
	{
		return tryCall(&ScriptSystem::onKeyEvent, event)
			|| tryCall(&ScriptSystem::onMouseMoveEvent, event)
			|| tryCall(&ScriptSystem::onMouseScrollEvent, event)
			|| tryCall(&ScriptSystem::onMouseButtonEvent, event)
			|| tryCall(&ScriptSystem::onWindowResizeEvent, event)
			|| tryCall(&ScriptSystem::onScriptEvent, event);
	}


	void ScriptSystem::update()
	{
		SOMBRA_DEBUG_LOG << "Updating the Scripts";

		mEntityDatabase.iterateComponents<ScriptComponent>(
			[this](Entity, ScriptComponent* script) {
				script->onUpdate(mDeltaTime, mScriptSharedState);
			},
			true
		);

		SOMBRA_DEBUG_LOG << "Update end";
	}

// Private functions
	void ScriptSystem::onNewScript(Entity entity, ScriptComponent* script)
	{
		addUser(script->getScript());
		script->setup(&mApplication.getEventManager(), entity);
		script->onAdd(mScriptSharedState);
		SOMBRA_INFO_LOG << "Entity " << entity << " with Script " << script << " added successfully";
	}


	void ScriptSystem::onRemoveScript(Entity entity, ScriptComponent* script)
	{
		script->onRemove(mScriptSharedState);
		removeUser(script->getScript());
		SOMBRA_INFO_LOG << "Entity " << entity << " with Script " << script << " removed successfully";
	}


	void ScriptSystem::onKeyEvent(const KeyEvent& event)
	{
		SOMBRA_INFO_LOG << event;
		mScriptSharedState.keys[event.getKeyCode()] = (event.getState() != KeyEvent::State::Released);
	}


	void ScriptSystem::onMouseMoveEvent(const MouseMoveEvent& event)
	{
		SOMBRA_INFO_LOG << event;
		mScriptSharedState.mouseX = static_cast<float>(event.getX());
		mScriptSharedState.mouseY = static_cast<float>(event.getY());
	}


	void ScriptSystem::onMouseScrollEvent(const MouseScrollEvent& event)
	{
		SOMBRA_INFO_LOG << event;
		mScriptSharedState.scrollOffsetX = static_cast<float>(event.getXOffset());
		mScriptSharedState.scrollOffsetY = static_cast<float>(event.getYOffset());
	}


	void ScriptSystem::onMouseButtonEvent(const MouseButtonEvent& event)
	{
		SOMBRA_INFO_LOG << event;
		mScriptSharedState.mouseButtons[event.getButtonCode()] = (event.getState() == MouseButtonEvent::State::Pressed);
	}


	void ScriptSystem::onWindowResizeEvent(const WindowResizeEvent& event)
	{
		SOMBRA_INFO_LOG << event;
		mScriptSharedState.windowWidth = static_cast<float>(event.getWidth());
		mScriptSharedState.windowHeight = static_cast<float>(event.getHeight());
	}


	void ScriptSystem::onScriptEvent(const ScriptEvent& event)
	{
		SOMBRA_INFO_LOG << event;

		auto [script] = mEntityDatabase.getComponents<ScriptComponent>(event.getEntity(), true);
		if (script) {
			switch (event.getOperation()) {
				case ScriptEvent::Operation::Add: {
					addUser(script->getScript());
					script->onAdd(mScriptSharedState);
				} break;
				case ScriptEvent::Operation::Remove: {
					script->onRemove(mScriptSharedState);
					removeUser(script->getScript());
				} break;
			}
		}
	}


	void ScriptSystem::addUser(const ScriptRef& script)
	{
		if (!script) { return; }

		auto it = std::find_if(mScriptsData.begin(), mScriptsData.end(), [&](const auto& tData) {
			return tData.scriptRef == script;
		});
		if (it == mScriptsData.end()) {
			it = mScriptsData.emplace(ScriptData{ script, 0 });
		}

		++it->userCount;
	}


	void ScriptSystem::removeUser(const ScriptRef& script)
	{
		if (!script) { return; }

		auto it = std::find_if(mScriptsData.begin(), mScriptsData.end(), [&](const auto& tData) {
			return tData.script == script;
		});
		if (it != mScriptsData.end()) {
			if (--it->userCount == 0) {
				mScriptsData.erase(it);
			}
		}
	}

}
