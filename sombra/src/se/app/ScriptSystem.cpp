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

		const auto& windowData = mApplication.getExternalTools().windowManager->getWindowData();
		mUserInput.windowWidth = static_cast<float>(windowData.width);
		mUserInput.windowHeight = static_cast<float>(windowData.height);
	}


	ScriptSystem::~ScriptSystem()
	{
		mApplication.getEventManager().unsubscribe(this, Topic::WindowResize);
		mApplication.getEventManager().unsubscribe(this, Topic::MouseButton);
		mApplication.getEventManager().unsubscribe(this, Topic::MouseScroll);
		mApplication.getEventManager().unsubscribe(this, Topic::MouseMove);
		mApplication.getEventManager().unsubscribe(this, Topic::Key);
		mEntityDatabase.removeSystem(this);
	}


	bool ScriptSystem::notify(const IEvent& event)
	{
		return tryCall(&ScriptSystem::onKeyEvent, event)
			|| tryCall(&ScriptSystem::onMouseMoveEvent, event)
			|| tryCall(&ScriptSystem::onMouseScrollEvent, event)
			|| tryCall(&ScriptSystem::onMouseButtonEvent, event)
			|| tryCall(&ScriptSystem::onWindowResizeEvent, event);
	}


	void ScriptSystem::update()
	{
		SOMBRA_DEBUG_LOG << "Updating the Scripts";

		mEntityDatabase.iterateComponents<ScriptComponent>(
			[this](Entity, ScriptComponent* script) {
				script->onUpdate(mDeltaTime, mUserInput);
			},
			true
		);

		SOMBRA_DEBUG_LOG << "Update end";
	}

// Private functions
	void ScriptSystem::onNewScript(Entity entity, ScriptComponent* script)
	{
		script->setup(mApplication.getEntityDatabase(), mApplication.getEventManager(), entity);
		script->onCreate(mUserInput);
		SOMBRA_INFO_LOG << "Entity " << entity << " with Script " << script << " added successfully";
	}


	void ScriptSystem::onRemoveScript(Entity entity, ScriptComponent* script)
	{
		script->onDestroy(mUserInput);
		SOMBRA_INFO_LOG << "Entity " << entity << " with Script " << script << " removed successfully";
	}


	void ScriptSystem::onKeyEvent(const KeyEvent& event)
	{
		SOMBRA_INFO_LOG << event;
		mUserInput.keys[event.getKeyCode()] = (event.getState() != KeyEvent::State::Released);
	}


	void ScriptSystem::onMouseMoveEvent(const MouseMoveEvent& event)
	{
		SOMBRA_INFO_LOG << event;
		mUserInput.mouseX = static_cast<float>(event.getX());
		mUserInput.mouseY = static_cast<float>(event.getY());
	}


	void ScriptSystem::onMouseScrollEvent(const MouseScrollEvent& event)
	{
		SOMBRA_INFO_LOG << event;
		mUserInput.scrollOffsetX = static_cast<float>(event.getXOffset());
		mUserInput.scrollOffsetY = static_cast<float>(event.getYOffset());
	}


	void ScriptSystem::onMouseButtonEvent(const MouseButtonEvent& event)
	{
		SOMBRA_INFO_LOG << event;
		mUserInput.mouseButtons[event.getButtonCode()] = (event.getState() == MouseButtonEvent::State::Pressed);
	}


	void ScriptSystem::onWindowResizeEvent(const WindowResizeEvent& event)
	{
		SOMBRA_INFO_LOG << event;
		mUserInput.windowWidth = static_cast<float>(event.getWidth());
		mUserInput.windowHeight = static_cast<float>(event.getHeight());
	}

}
