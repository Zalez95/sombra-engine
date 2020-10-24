#include "se/animation/CompositeAnimator.h"
#include "se/audio/Buffer.h"
#include "se/physics/forces/Force.h"
#include "se/physics/constraints/Constraint.h"
#include "se/graphics/core/Program.h"
#include "se/graphics/Pass.h"
#include "se/graphics/Technique.h"
#include "se/graphics/core/Texture.h"
#include "se/app/SkinComponent.h"
#include "se/app/LightComponent.h"
#include "se/app/Scene.h"
#include "se/app/EntityDatabase.h"

namespace se::app {

	Scene::Scene(const char* name, Application& application) : name(name), application(application)
	{
		repository.init<std::string, Skin>();
		repository.init<std::string, LightSource>();
		repository.init<std::string, animation::CompositeAnimator>();
		repository.init<std::string, audio::Buffer>();
		repository.init<std::string, physics::Force>();
		repository.init<std::string, physics::Constraint>();
		repository.init<std::string, graphics::Program>();
		repository.init<std::string, graphics::Pass>();
		repository.init<std::string, graphics::Texture>();
		repository.init<std::string, graphics::Technique>();
		repository.init<graphics::Pass*, graphics::Program>();

		entities.reserve(application.getEntityDatabase().getMaxEntities());
	}


	Scene::~Scene()
	{
		for (auto entity : entities) {
			application.getEntityDatabase().removeEntity(entity);
		}
	}

}
