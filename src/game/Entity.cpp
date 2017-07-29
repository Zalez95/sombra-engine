#include "Entity.h"
#include "InputComponent.h"
#include "PhysicsComponent.h"
#include "GraphicsComponent.h"

namespace game {

	Entity::Entity(
		const std::string& name,
		std::unique_ptr<InputComponent> inputComponent,
		std::unique_ptr<PhysicsComponent> physicsComponent,
		std::unique_ptr<GraphicsComponent> graphicsComponent
	) : mName(name),
		mInputComponent(std::move(inputComponent)),
		mPhysicsComponent(std::move(physicsComponent)),
		mGraphicsComponent(std::move(graphicsComponent)) {}


	Entity::~Entity() {}


	void Entity::update(float delta)
	{
		if (mInputComponent)
			mInputComponent->update(*this, delta);
		if (mPhysicsComponent)
			mPhysicsComponent->update(*this, delta);
		if (mGraphicsComponent)
			mGraphicsComponent->update(*this, delta);
	}

}
