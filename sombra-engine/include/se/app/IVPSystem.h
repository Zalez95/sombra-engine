#ifndef IVP_SYSTEM_H
#define IVP_SYSTEM_H

#include <unordered_map>
#include <glm/glm.hpp>
#include "../utils/PackedVector.h"
#include "../graphics/Pass.h"
#include "../graphics/core/UniformVariable.h"
#include "../graphics/3D/Renderable3D.h"
#include "ISystem.h"

namespace se::app {

	class Application;


	/**
	 * Class IVPSystem, it's a System used for updating the view and projection
	 * matrices on the Entities' shaders.
	 */
	class IVPSystem : public ISystem
	{
	protected:	// Nested types
		using PassSPtr = std::shared_ptr<graphics::Pass>;
		using Mat4Uniform = std::shared_ptr<
			graphics::UniformVariableValue<glm::mat4>
		>;

		/** Struct PassData, holds the shared uniform variables between the
		 * Renderables */
		struct PassData
		{
			std::size_t userCount = 0;
			PassSPtr pass;
			Mat4Uniform viewMatrix;
			Mat4Uniform projectionMatrix;
		};

	protected:	// Attributes
		/** The Application that holds the Entities */
		Application& mApplication;

		/** The name of the View matrix uniform variable */
		std::string mViewMatUniform;

		/** The name of the Projection matrix uniform variable */
		std::string mProjectionMatUniform;

		/** The shared uniform variables of the Passes */
		utils::PackedVector<PassData> mPassesData;

		/** Maps each Entity with its respective PassData indices */
		std::unordered_map<Entity, std::vector<std::size_t>> mEntityPasses;

	public:		// Functions
		/** Creates a new IVPSystem
		 *
		 * @param	application a reference to the Application that holds the
		 *			current System
		 * @param	viewMatUniform The name of the View matrix uniform variable
		 * @param	projectionMatUniform The name of the Projection matrix
		 *			uniform variable */
		IVPSystem(
			Application& application,
			const std::string& viewMatUniform,
			const std::string& projectionMatUniform
		);

		/** Class destructor */
		virtual ~IVPSystem();

		/** @copydoc ISystem::onNewEntity(Entity) */
		virtual void onNewEntity(Entity entity);

		/** @copydoc ISystem::onRemoveEntity(Entity) */
		virtual void onRemoveEntity(Entity entity);

		/** Updates the Passes uniform variables sources with the new view
		 * and projection matrix */
		virtual void update() override;
	protected:
		/** @return	the current value of the view matrix */
		virtual glm::mat4 getViewMatrix() const = 0;

		/** @return	the current value of the projection matrix */
		virtual glm::mat4 getProjectionMatrix() const = 0;

		/** Checks if the System must add the uniform variables to the given
		 * Pass
		 *
		 * @param	pass a pointer to the Pass to check
		 * @return	true if the uniform variables must be added to the Pass,
		 *			false otherwise */
		virtual bool shouldAddUniforms(PassSPtr pass) const = 0;

		/** Processes the passes of the given renderable adding them the view
		 * and projection uniform variables
		 *
		 * @param	renderable the Renderable3D to process
		 * @param	output the vector where the indices of the passes in
		 *			@see mPassesData will be appended */
		void processPasses(
			graphics::Renderable3D& renderable, std::vector<std::size_t>& output
		);
	};

}

#endif		// IVP_SYSTEM_H
