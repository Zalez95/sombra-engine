#ifndef I_VIEW_PROJECTION_UPDATER_H
#define I_VIEW_PROJECTION_UPDATER_H

#include <unordered_map>
#include <glm/glm.hpp>
#include "../../utils/PackedVector.h"
#include "../../graphics/Pass.h"
#include "../../graphics/Renderable.h"
#include "../../graphics/core/UniformVariable.h"

namespace se::app {

	/**
	 * Class IViewProjectionUpdater, it's used for updating the view and
	 * projection matrix uniform variables on the Renderables' shaders.
	 */
	class IViewProjectionUpdater
	{
	protected:	// Nested types
		using PassSPtr = std::shared_ptr<graphics::Pass>;
		using IndexVector = std::vector<std::size_t>;
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
		/** The name of the View matrix uniform variable */
		std::string mViewMatUniformName;

		/** The name of the Projection matrix uniform variable */
		std::string mProjectionMatUniformName;

		/** The shared uniform variables of the Passes */
		utils::PackedVector<PassData> mPassesData;

		/** Maps each Renderable with its respective PassData indices */
		std::unordered_map<graphics::Renderable*, IndexVector>
			mRenderablePasses;

	public:		// Functions
		/** Creates a new IViewProjectionUpdater
		 *
		 * @param	viewMatUniformName the name of the View matrix uniform
		 *			variable
		 * @param	projectionMatUniform Name te name of the Projection matrix
		 *			uniform variable */
		IViewProjectionUpdater(
			const std::string& viewMatUniformName,
			const std::string& projectionMatUniformName
		);

		/** Class destructor */
		virtual ~IViewProjectionUpdater() = default;

		/** Adds the given renderable from the IViewProjectionUpdater so its
		 * passes will longer be updated with the new view and projection
		 * matrices.
		 *
		 * @param	renderable the Renderable to add */
		void addRenderable(graphics::Renderable& renderable);

		/** Removes the given renderable from the IViewProjectionUpdater. Its
		 * passes will no longer be updated if they aren't used by any other
		 * Renderable added to the the IViewProjectionUpdater.
		 *
		 * @param	renderable the Renderable to remove */
		void removeRenderable(graphics::Renderable& renderable);

		/** Updates the Passes uniform variables sources with the new view
		 * and projection matrix */
		void update();
	protected:
		/** @return	the current value of the view matrix */
		virtual glm::mat4 getViewMatrix() const = 0;

		/** @return	the current value of the projection matrix */
		virtual glm::mat4 getProjectionMatrix() const = 0;

		/** Checks if the IViewProjectionUpdater must add the uniform variables
		 * to the given Pass
		 *
		 * @param	pass a pointer to the Pass to check
		 * @return	true if the uniform variables must be added to the Pass,
		 *			false otherwise */
		virtual bool shouldAddUniforms(PassSPtr pass) const = 0;
	};

}

#endif		// I_VIEW_PROJECTION_UPDATER_H
