#ifndef I_VIEW_PROJECTION_UPDATER_H
#define I_VIEW_PROJECTION_UPDATER_H

#include <queue>
#include <unordered_map>
#include <future>
#include <glm/glm.hpp>
#include "se/graphics/Pass.h"
#include "se/graphics/Technique.h"
#include "se/graphics/Renderable.h"
#include "se/utils/PackedVector.h"
#include "se/app/graphics/TypeRefs.h"

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

		/** Struct PassData, holds the shared Pass uniform variables between the
		 * Techniques */
		struct PassData
		{
			std::size_t userCount = 0;
			PassSPtr pass;
			UniformVVRef<glm::mat4> viewMatrix;
			UniformVVRef<glm::mat4> projectionMatrix;
		};

		/** Struct TechniqueData, holds the shared Technique data between the
		 * Renderables */
		struct TechniqueData
		{
			std::size_t userCount = 0;
			TechniqueSPtr technique;
			IndexVector passIndices;
		};

		/** Holds the data of the new uniforms to process */
		struct NewUniform
		{
			/** First bit tells if its view (0) or projection (1) matrix,
			 * second one if it's inside the pass (0) or a new one (1) */
			unsigned char type;
			PassSPtr pass;
			std::future<UniformVVRef<glm::mat4>> uniformMat;
		};

	protected:	// Attributes
		/** The graphics Context used for creating the uniforms */
		graphics::Context& mContext;

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

		/** The new uniforms to add to passes, it's needed because
		 * we can't use @see mMutex inside the Context functions */
		std::queue<NewUniform> mNewUniforms;

		/** The mutex used for protecting the attributes of the
		 * IViewProjectionUpdater */
		std::mutex mMutex;

	public:		// Functions
		/** Creates a new IViewProjectionUpdater
		 *
		 * @param	context the graphics Context used for creating the uniforms
		 * @param	viewMatUniformName the name of the View matrix uniform
		 *			variable
		 * @param	projectionMatUniform Name te name of the Projection matrix
		 *			uniform variable */
		IViewProjectionUpdater(
			graphics::Context& context,
			const char* viewMatUniformName,
			const char* projectionMatUniformName
		);

		/** Class destructor */
		virtual ~IViewProjectionUpdater() = default;

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

		/** Creates and submits the operations used for updating the Passes
		 * uniform variables with the new view and projection matrix
		 *
		 * @param	viewMatrix the new value of the view matrix
		 * @param	projectionMatrix the new value of the projection matrix */
		void updateUniformsDeferred(
			const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix
		);

		/** Updates the Passes uniform variables with the new view and
		 * projection matrix
		 *
		 * @param	q the graphics Context Query object used for accesing the
		 *			uniforms bindables
		 * @param	viewMatrix the new value of the view matrix
		 * @param	projectionMatrix the new value of the projection matrix
		 * @note	this function must be called from the main graphics
		 *			thread */
		void updateUniforms(
			graphics::Context::Query& q,
			const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix
		);
	protected:
		/** Checks if the IViewProjectionUpdater must add the uniform variables
		 * to the given Pass
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

		/** Processes all the new uniform matrices added on @see addPass */
		void processNewUniforms();
	};

}

#endif		// I_VIEW_PROJECTION_UPDATER_H
