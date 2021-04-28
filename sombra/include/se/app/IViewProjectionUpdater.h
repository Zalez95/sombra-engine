#ifndef I_VIEW_PROJECTION_UPDATER_H
#define I_VIEW_PROJECTION_UPDATER_H

#include <unordered_map>
#include <glm/glm.hpp>
#include "../utils/PackedVector.h"
#include "../graphics/Pass.h"
#include "../graphics/Renderable.h"
#include "../graphics/core/UniformVariable.h"
#include "RenderableShader.h"

namespace se::app {

	/**
	 * Class IViewProjectionUpdater, it's used for updating the view and
	 * projection matrix uniform variables on the Renderables' shaders.
	 */
	class IViewProjectionUpdater
	{
	protected:	// Nested types
		using RenderableShaderSPtr = std::shared_ptr<RenderableShader>;
		using PassSPtr = std::shared_ptr<graphics::Pass>;
		using IndexVector = std::vector<std::size_t>;
		using Mat4Uniform = std::shared_ptr<
			graphics::UniformVariableValue<glm::mat4>
		>;

		/** Struct PassData, holds the shared Pass uniform variables between the
		 * Shaders */
		struct PassData
		{
			std::size_t userCount = 0;
			PassSPtr pass;
			Mat4Uniform viewMatrix;
			Mat4Uniform projectionMatrix;
		};

		/** Struct ShaderData, holds the shared Shader data between the
		 * Renderables */
		struct ShaderData
		{
			std::size_t userCount = 0;
			RenderableShaderSPtr shader;
			IndexVector passIndices;
		};

	protected:	// Attributes
		/** The name of the View matrix uniform variable */
		std::string mViewMatUniformName;

		/** The name of the Projection matrix uniform variable */
		std::string mProjectionMatUniformName;

		/** The shared uniform variables of the Passes */
		utils::PackedVector<PassData> mPassesData;

		/** Holds the passes of the Shaders */
		utils::PackedVector<ShaderData> mShadersData;

		/** Maps each Renderable with its respective Shader indices */
		std::unordered_map<graphics::Renderable*, IndexVector>
			mRenderableShaders;

	public:		// Functions
		/** Creates a new IViewProjectionUpdater
		 *
		 * @param	viewMatUniformName the name of the View matrix uniform
		 *			variable
		 * @param	projectionMatUniform Name te name of the Projection matrix
		 *			uniform variable */
		IViewProjectionUpdater(
			const char* viewMatUniformName,
			const char* projectionMatUniformName
		);

		/** Class destructor */
		virtual ~IViewProjectionUpdater() = default;

		/** Adds the given Renderable
		 *
		 * @param	renderable the Renderable to add */
		void addRenderable(graphics::Renderable& renderable);

		/** Removes the given Renderable
		 *
		 * @param	renderable the Renderable to remove */
		void removeRenderable(graphics::Renderable& renderable);

		/** Adds the given renderable with the given shader to the
		 * IViewProjectionUpdater so its passes be updated with the new view
		 * and projection matrices.
		 *
		 * @param	renderable the Renderable to add
		 * @param	shader a pointer to the new Shader */
		void addRenderableShader(
			graphics::Renderable& renderable, const RenderableShaderSPtr& shader
		);

		/** Removes the given renderable with the given shader from the
		 * IViewProjectionUpdater so its passes won't longer updated with the
		 * new view and projection matrices.
		 *
		 * @param	renderable the Renderable to remove
		 * @param	shader a pointer to the Shader to remove */
		void removeRenderableShader(
			graphics::Renderable& renderable, const RenderableShaderSPtr& shader
		);

		/** Updates the stored Shader data with the given new pass
		 *
		 * @param	shader a pointer to the Shader updated
		 * @param	pass a pointer to the new Pass added to the Shader */
		void onAddShaderPass(
			const RenderableShaderSPtr& shader, const PassSPtr& pass
		);

		/** Updates the stored Shader data with the given removed pass
		 *
		 * @param	shader a pointer to the Shader updated
		 * @param	pass a pointer to the Pass removed from the Shader */
		void onRemoveShaderPass(
			const RenderableShaderSPtr& shader, const PassSPtr& pass
		);

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
		virtual bool shouldAddUniforms(const PassSPtr& pass) const = 0;
	private:
		/** Adds the given pass to the Shader, and adds its uniforms if they
		 * weren't added before
		 *
		 * @param	iShader the index of the Shader
		 * @param	pass a pointer to the new Pass */
		void addPass(std::size_t iShader, const PassSPtr& pass);

		/** Removes the given pass from the Shader, and removes its uniforms
		 * if it's the last shader user
		 *
		 * @param	iShader the index of the Shader
		 * @param	the index of the Pass to remove */
		void removePass(std::size_t iShader, std::size_t iPass);

		/** Removes the given Shader
		 *
		 * @param	iShader the index of the Shader */
		void removeShader(std::size_t iShader);
	};

}

#endif		// I_VIEW_PROJECTION_UPDATER_H
