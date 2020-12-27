#ifndef RMESH_SYSTEM_H
#define RMESH_SYSTEM_H

#include <memory>
#include <unordered_map>
#include "../utils/MathUtils.h"
#include "../graphics/Pass.h"
#include "../graphics/core/UniformVariable.h"
#include "ISystem.h"
#include "SkinComponent.h"

namespace se::app {

	class Application;


	/**
	 * Class RMeshSystem, it's a System used for updating the Entities'
	 * RenderableMesh data
	 */
	class RMeshSystem : public ISystem
	{
	private:	// Nested types
		/** Holds the uniform variables of a RenderableMesh */
		struct RenderableMeshUniforms
		{
			/** The pass used for retrieving the uniforms */
			std::shared_ptr<graphics::Pass> pass;

			/** The model matrix uniform variable */
			std::shared_ptr<graphics::UniformVariableValue<glm::mat4>>
				modelMatrix;

			/** The joint matrices uniform variables (only for skeletal
			 * animation) */
			std::shared_ptr<graphics::UniformVariableValueVector<
				glm::mat4, Skin::kMaxJoints
			>> jointMatrices;
		};

	private:	// Attributes
		/** The Application that holds the GraphicsEngine used for rendering
		 * the RenderableMeshes */
		Application& mApplication;

		/** The RenderableMesh uniform variables mapped by the Entity */
		std::unordered_map<Entity, std::vector<RenderableMeshUniforms>>
			mEntityUniforms;

	public:		// Functions
		/** Creates a new RMeshSystem
		 *
		 * @param	application a reference to the Application that holds the
		 *			current System */
		RMeshSystem(Application& application);

		/** Class destructor */
		~RMeshSystem();

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

		/** Updates the RenderableMeshes with the Entities */
		virtual void update() override;
	};

}

#endif		// RMESH_SYSTEM_H
