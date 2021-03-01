#include "se/utils/Log.h"
#include "se/app/ScriptSystem.h"
#include "se/app/Application.h"
#include "se/app/EntityDatabase.h"
#include "se/app/ScriptComponent.h"

namespace se::app {

	ScriptSystem::ScriptSystem(Application& application) :
		ISystem(application.getEntityDatabase()), mApplication(application)
	{
		mEntityDatabase.addSystem(this, EntityDatabase::ComponentMask().set<ScriptComponent>());
		mApplication.getEventManager().subscribe(this, Topic::Key);
		mApplication.getEventManager().subscribe(this, Topic::MouseMove);
		mApplication.getEventManager().subscribe(this, Topic::MouseScroll);
		mApplication.getEventManager().subscribe(this, Topic::MouseButton);
		mApplication.getEventManager().subscribe(this, Topic::Resize);

		const auto& windowData = mApplication.getExternalTools().windowManager->getWindowData();
		mUserInput.windowWidth = static_cast<float>(windowData.width);
		mUserInput.windowHeight = static_cast<float>(windowData.height);
	}


	ScriptSystem::~ScriptSystem()
	{
		mApplication.getEventManager().unsubscribe(this, Topic::Resize);
		mApplication.getEventManager().unsubscribe(this, Topic::MouseButton);
		mApplication.getEventManager().unsubscribe(this, Topic::MouseScroll);
		mApplication.getEventManager().unsubscribe(this, Topic::MouseMove);
		mApplication.getEventManager().unsubscribe(this, Topic::Key);
		mEntityDatabase.removeSystem(this);
	}


	void ScriptSystem::onNewEntity(Entity entity)
	{
		auto [script] = mEntityDatabase.getComponents<ScriptComponent>(entity);
		script->setup(mApplication.getEntityDatabase(), mApplication.getEventManager(), entity);
		script->onCreate(mUserInput);

		SOMBRA_INFO_LOG << "Entity " << entity << " with Script " << script << " added successfully";
	}


	void ScriptSystem::onRemoveEntity(Entity entity)
	{
		auto [script] = mEntityDatabase.getComponents<ScriptComponent>(entity);
		script->onDestroy(mUserInput);

		SOMBRA_INFO_LOG << "Entity " << entity << " removed successfully";
	}


	void ScriptSystem::update()
	{
		SOMBRA_DEBUG_LOG << "Updating the Scripts";

		mEntityDatabase.iterateComponents<ScriptComponent>([this](Entity, ScriptComponent* script) {
			script->onUpdate(mDeltaTime, mUserInput);
		});

		SOMBRA_INFO_LOG << "Update end";
	}


	void ScriptSystem::notify(const IEvent& event)
	{
		tryCall(&ScriptSystem::onKeyEvent, event);
		tryCall(&ScriptSystem::onMouseMoveEvent, event);
		tryCall(&ScriptSystem::onMouseScrollEvent, event);
		tryCall(&ScriptSystem::onMouseButtonEvent, event);
		tryCall(&ScriptSystem::onResizeEvent, event);
	}

// Private functions
	void ScriptSystem::onKeyEvent(const KeyEvent& event)
	{
		mUserInput.keys[event.getKeyCode()] = (event.getState() != KeyEvent::State::Released);
	}


	void ScriptSystem::onMouseMoveEvent(const MouseMoveEvent& event)
	{
		mUserInput.mouseX = static_cast<float>(event.getX());
		mUserInput.mouseY = static_cast<float>(event.getY());
	}


	void ScriptSystem::onMouseScrollEvent(const MouseScrollEvent& event)
	{
		mUserInput.scrollOffsetX = static_cast<float>(event.getXOffset());
		mUserInput.scrollOffsetY = static_cast<float>(event.getYOffset());
	}


	void ScriptSystem::onMouseButtonEvent(const MouseButtonEvent& event)
	{
		mUserInput.mouseButtons[event.getButtonCode()] = (event.getState() == MouseButtonEvent::State::Pressed);
	}


	void ScriptSystem::onResizeEvent(const ResizeEvent& event)
	{
		mUserInput.windowWidth = static_cast<float>(event.getWidth());
		mUserInput.windowHeight = static_cast<float>(event.getHeight());
	}

}
