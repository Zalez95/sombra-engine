#include <string>
#include "se/animation/IAnimator.h"
#include "se/audio/Buffer.h"
#include "se/physics/forces/Force.h"
#include "se/physics/constraints/Constraint.h"
#include "se/graphics/core/Program.h"
#include "se/graphics/Pass.h"
#include "se/graphics/Technique.h"
#include "se/app/graphics/Skin.h"
#include "se/app/graphics/LightSource.h"
#include "se/app/Scene.h"

namespace se::app {

	Scene::Scene()
	{
		repository.init<std::string, Skin>();
		repository.init<std::string, LightSource>();
		repository.init<std::string, animation::IAnimator>();
		repository.init<std::string, audio::Buffer>();
		repository.init<std::string, physics::Force>();
		repository.init<std::string, physics::Constraint>();
		repository.init<std::string, graphics::Program>();
		repository.init<std::string, graphics::Pass>();
		repository.init<std::string, graphics::Technique>();
		repository.init<graphics::Pass*, graphics::Program>();
	}

}
