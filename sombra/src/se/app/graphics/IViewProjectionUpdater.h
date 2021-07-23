#ifndef I_VIEW_PROJECTION_UPDATER_H
#define I_VIEW_PROJECTION_UPDATER_H

#include <unordered_map>
#include <glm/glm.hpp>
#include "se/graphics/Pass.h"
#include "se/graphics/Technique.h"
#include "se/graphics/Renderable.h"
#include "se/graphics/core/UniformVariable.h"
#include "se/utils/PackedVector.h"

namespace se::app {

	/**
	 * Class IViewProjectionUpdater, it's used for updating the view and
	 * projection matrix uniform variables of the Renderables' shaders.
	 */
	class IViewProjectionUpdater
	{
	protected:	// Nested types
		using PassSPtr = std::shared_ptr<graphics::Pass>;
		using TechniqueSPtr = std::shared_ptr<graphics::Technique>;
		using IndexVector = std::vector<std::size_t>;
		using Mat4Uniform = graphics::UniformVariableValue<glm::mat4>;
		using Mat4UniformSPtr = std::shared_ptr<Mat4Uniform>;

		/** Struct PassData, holds the shared Pass uniform variables between the
		 * Techniques */
		struct PassData
		{
			std::size_t userCount = 0;
			PassSPtr pass;
			Mat4UniformSPtr viewMatrix;
			Mat4UniformSPtr projectionMatrix;
		};

		/** Struct TechniqueData, holds the shared Technique data between the
		 * Renderables */
		struct TechniqueData
		{
			std::size_t userCount = 0;
			TechniqueSPtr technique;
			IndexVector passIndices;
		};

	protected:	// Attributes
		/** The name of the View matrix uniform variable */
		std::string mViewMatUniformName;

		/** The name of the Projection matrix uniform variable */
		std::string mProjectionMatUniformName;

		/** The shared uniform variables of the Pass */
		utils::PackedVector<PassData> mPassesData;

		/** Holds the techniques of the Renderables */
		utils::PackedVector<TechniqueData> mTechniquesData;

		/** Maps each Renderable with its respective Technique indices */
		std::unordered_map<graphics::Renderable*, IndexVector>
			mRenderableTechniques;

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

		/** Adds the given Renderable with the given Technique to the
		 * IViewProjectionUpdater so its passes be updated with the new view
		 * and projection matrices.
		 *
		 * @param	renderable the Renderable to add
		 * @param	technique a pointer to the new Technique */
		void addRenderableTechnique(
			graphics::Renderable& renderable, const TechniqueSPtr& technique
		);

		/** Removes the given Renderable with the given Technique from the
		 * IViewProjectionUpdater so its passes won't longer updated with the
		 * new view and projection matrices.
		 *
		 * @param	renderable the Renderable to remove
		 * @param	technique a pointer to the Technique to remove */
		void removeRenderableTechnique(
			graphics::Renderable& renderable, const TechniqueSPtr& technique
		);

		/** Updates the stored Technique data with the given new Technique
		 *
		 * @param	technique a pointer to the Technique updated
		 * @param	pass a pointer to the new Pass added to the Technique */
		void onAddTechniquePass(
			const TechniqueSPtr& technique, const PassSPtr& pass
		);

		/** Updates the stored Technique data with the given removed Technique
		 *
		 * @param	technique a pointer to the Technique updated
		 * @param	pass a pointer to the Pass removed from the Technique */
		void onRemoveTechniquePass(
			const TechniqueSPtr& technique, const PassSPtr& pass
		);

		/** Updates the Passes uniform variables with the new view and
		 * projection matrix
		 *
		 * @param	viewMatrix the new value of the view matrix
		 * @param	projectionMatrix the new value of the projection matrix */
		void update(
			const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix
		);
	protected:
		/** Checks if the IViewProjectionUpdater must add the uniform variables
		 * to the given Technique
		 *
		 * @param	pass a pointer to the Pass to check
		 * @return	true if the uniform variables must be added to the Pass,
		 *			false otherwise */
		virtual bool shouldAddUniforms(const PassSPtr& pass) const = 0;
	private:
		/** Adds the given Pass to the Technique, and adds its uniforms if they
		 * weren't added before
		 *
		 * @param	iTechnique the index of the Technique
		 * @param	pass a pointer to the new Pass */
		void addPass(std::size_t iTechnique, const PassSPtr& pass);

		/** Removes the given Pass from the Technique, and removes its uniforms
		 * if it's the last Technique user
		 *
		 * @param	iTechnique the index of the Technique
		 * @param	iPass the index of the Pass to remove */
		void removePass(std::size_t iTechnique, std::size_t iPass);

		/** Removes the given Technique
		 *
		 * @param	iTechnique the index of the Technique */
		void removeTechnique(std::size_t iTechnique);
	};

}

#endif		// I_VIEW_PROJECTION_UPDATER_H
