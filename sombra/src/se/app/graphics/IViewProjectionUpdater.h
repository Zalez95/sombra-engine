#ifndef I_VIEW_PROJECTION_UPDATER_H
#define I_VIEW_PROJECTION_UPDATER_H

#include <unordered_map>
#include <glm/glm.hpp>
#include "se/graphics/Renderable.h"
#include "se/graphics/core/UniformVariable.h"
#include "se/app/graphics/RenderableShader.h"

namespace se::app {

	/**
	 * Class IViewProjectionUpdater, it's used for updating the view and
	 * projection matrix uniform variables on the Renderables' shaders.
	 */
	class IViewProjectionUpdater
	{
	protected:	// Nested types
		using RenderableShaderSPtr = std::shared_ptr<RenderableShader>;
		using RenderableShaderStepSPtr = std::shared_ptr<RenderableShaderStep>;
		using IndexVector = std::vector<std::size_t>;
		using Mat4Uniform = graphics::UniformVariableValue<glm::mat4>;
		using Mat4UniformSPtr = std::shared_ptr<Mat4Uniform>;

		/** Struct StepData, holds the shared Step uniform variables between the
		 * Shaders */
		struct StepData
		{
			std::size_t userCount = 0;
			RenderableShaderStepSPtr step;
			Mat4UniformSPtr viewMatrix;
			Mat4UniformSPtr projectionMatrix;
		};

		/** Struct ShaderData, holds the shared Shader data between the
		 * Renderables */
		struct ShaderData
		{
			std::size_t userCount = 0;
			RenderableShaderSPtr shader;
			IndexVector stepIndices;
		};

	protected:	// Attributes
		/** The name of the View matrix uniform variable */
		std::string mViewMatUniformName;

		/** The name of the Projection matrix uniform variable */
		std::string mProjectionMatUniformName;

		/** The shared uniform variables of the Steps */
		utils::PackedVector<StepData> mStepsData;

		/** Holds the steps of the Shaders */
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
		 * IViewProjectionUpdater so its steps be updated with the new view
		 * and projection matrices.
		 *
		 * @param	renderable the Renderable to add
		 * @param	shader a pointer to the new Shader */
		void addRenderableShader(
			graphics::Renderable& renderable, const RenderableShaderSPtr& shader
		);

		/** Removes the given renderable with the given shader from the
		 * IViewProjectionUpdater so its steps won't longer updated with the
		 * new view and projection matrices.
		 *
		 * @param	renderable the Renderable to remove
		 * @param	shader a pointer to the Shader to remove */
		void removeRenderableShader(
			graphics::Renderable& renderable, const RenderableShaderSPtr& shader
		);

		/** Updates the stored Shader data with the given new
		 * RenderableShaderStep
		 *
		 * @param	shader a pointer to the Shader updated
		 * @param	step a pointer to the new RenderableShaderStep added to the
		 *			Shader */
		void onAddShaderStep(
			const RenderableShaderSPtr& shader,
			const RenderableShaderStepSPtr& step
		);

		/** Updates the stored Shader data with the given removed
		 * RenderableShaderStep
		 *
		 * @param	shader a pointer to the Shader updated
		 * @param	step a pointer to the RenderableShaderStep removed from the
		 *			Shader */
		void onRemoveShaderStep(
			const RenderableShaderSPtr& shader,
			const RenderableShaderStepSPtr& step
		);

		/** Updates the Steps uniform variables sources with the new view
		 * and projection matrix */
		void update();
	protected:
		/** @return	the current value of the view matrix */
		virtual glm::mat4 getViewMatrix() const = 0;

		/** @return	the current value of the projection matrix */
		virtual glm::mat4 getProjectionMatrix() const = 0;

		/** Checks if the IViewProjectionUpdater must add the uniform variables
		 * to the given RenderableShaderStep
		 *
		 * @param	step a pointer to the RenderableShaderStep to check
		 * @return	true if the uniform variables must be added to the
		 *			RenderableShaderStep, false otherwise */
		virtual
		bool shouldAddUniforms(const RenderableShaderStepSPtr& step) const = 0;
	private:
		/** Adds the given RenderableShaderStep to the Shader, and adds its
		 * uniforms if they weren't added before
		 *
		 * @param	iShader the index of the Shader
		 * @param	step a pointer to the new RenderableShaderStep */
		void addStep(std::size_t iShader, const RenderableShaderStepSPtr& step);

		/** Removes the given RenderableShaderStep from the Shader, and removes
		 * its uniforms if it's the last shader user
		 *
		 * @param	iShader the index of the Shader
		 * @param	the index of the RenderableShaderStep to remove */
		void removeStep(std::size_t iShader, std::size_t iStep);

		/** Removes the given Shader
		 *
		 * @param	iShader the index of the Shader */
		void removeShader(std::size_t iShader);
	};

}

#endif		// I_VIEW_PROJECTION_UPDATER_H
