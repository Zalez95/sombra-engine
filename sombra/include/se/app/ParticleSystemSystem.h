#ifndef PARTICLE_SYSTEM_SYSTEM_H
#define PARTICLE_SYSTEM_SYSTEM_H

#include "ISystem.h"

namespace se::app {

	class Application;


	/**
	 * Class ParticleSystemSystem, it's a System used for updating the Entities'
	 * ParticleSystemComponents
	 */
	class ParticleSystemSystem : public ISystem
	{
	private:	// Attributes
		/** The Application that holds the GraphicsEngine used for rendering
		 * the ParticleSystems */
		Application& mApplication;

	public:		// Functions
		/** Creates a new ParticleSystemSystem
		 *
		 * @param	application a reference to the Application that holds the
		 *			current System */
		ParticleSystemSystem(Application& application);

		/** Class destructor */
		~ParticleSystemSystem();

		/** Function that the EntityDatabase will call when an Entity is
		 * added
		 *
		 * @param	entity the new Entity */
		virtual void onNewEntity(Entity entity);

		/** Function that the EntityDatabase will call when an Entity is
		 * removed
		 *
		 * @param	entity the Entity to remove */
		virtual void onRemoveEntity(Entity entity);

		/** Updates the RenderableTerrains with the Entities */
		virtual void update() override;
	};

}

#endif		// PARTICLE_SYSTEM_SYSTEM_H
