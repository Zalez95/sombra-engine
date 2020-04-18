#ifndef APP_SYSTEM_H
#define APP_SYSTEM_H

#include "ComponentDatabase.h"
#include "TransformsComponent.h"
#include "../utils/Repository.h"
#include "../graphics/3d/Camera.h"
#include "../graphics/3d/Renderable3D.h"
#include "../graphics/3d/Lights.h"
#include "../animation/AnimationNode.h"
#include "../physics/RigidBody.h"

namespace se::app {

	using NameComponent = std::string;
	using CameraRepository = utils::Repository<
		graphics::Camera, unsigned short
	>;
	using Renderable3DRepository = utils::Repository<
		graphics::Renderable3D, unsigned short
	>;
	using PointLightRepository = utils::Repository<
		graphics::PointLight, unsigned short
	>;
	using AnimationNodeRepository = utils::Repository<
		animation::AnimationNode, unsigned short
	>;
	using RigidBodyRepository = utils::Repository<
		physics::RigidBody, unsigned short
	>;


	/**
	 * Class AppComponentDB
	 */
	class AppComponentDB : public ComponentDatabase<
		unsigned short,
		NameComponent,
		TransformsComponent,
		CameraRepository::Reference,
		Renderable3DRepository::Reference,
		PointLightRepository::Reference,
		AnimationNodeRepository::Reference,
		RigidBodyRepository::Reference
	> {
	public:		// Functions
		/** Copy constructor */
		AppComponentDB(const AppComponentDB&) = delete;

		/** Copy assignment operator */
		void operator=(const AppComponentDB&) = delete;

		/** @return	the only instance of the AppComponentDB */
		static AppComponentDB& getInstance();

	private:
		/** Creates a new AppComponentDB */
		AppComponentDB() = default;
	};


	/**
	 * The Ids of the Components in the AppComponentDB
	 */
	enum class ComponentIds
	{
		Name = 0,
		Transforms,
		Camera,
		Renderable3D,
		PointLight,
		AnimationNode,
		RigidBody,
		NumComponentTypes
	};

}

#endif		// APP_SYSTEM_H
