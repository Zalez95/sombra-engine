#ifndef RENDERABLE_SHADER_H
#define RENDERABLE_SHADER_H

#include "../../graphics/Pass.h"
#include "../../graphics/Technique.h"
#include "../events/EventManager.h"
#include "../Repository.h"
#include "TypeRefs.h"

namespace se::app {

	/**
	 * Class Step. It acts as a wrapper for the graphics Passes, so the
	 * resources can be cleared successfully
	 */
	class RenderableShaderStep
	{
	private:	// Attributes
		/** The Pass of the RenderableShaderStep */
		std::shared_ptr<graphics::Pass> mPass;

		/** The Program Resources added to the RenderableShaderStep */
		std::vector<Repository::ResourceRef<ProgramRef>> mProgramResources;

		/** The Texture Resources added to the RenderableShaderStep */
		std::vector<Repository::ResourceRef<TextureRef>> mTextureResources;

	public:		// Functions
		/** Creates a new RenderableShaderStep
		 *
		 * @param	renderer the Renderer used for creating the Step */
		RenderableShaderStep(graphics::Renderer& renderer) :
			mPass(std::make_shared<graphics::Pass>(renderer)) {};

		/** @return	the Pass wrapped by the RenderableShader */
		const std::shared_ptr<graphics::Pass>& getPass() const
		{ return mPass; };

		/** @return	a pointer to a copy of the current RenderableShaderStep */
		std::unique_ptr<RenderableShaderStep> clone() const;

		/** Adds the given Resource to the Step
		 *
		 * @param	resource the Resource to add
		 * @param	addBindable if we want also to add the Resource
		 *			bindable or not
		 * @return	a reference to the current Step object */
		template <typename T>
		RenderableShaderStep& addResource(
			const Repository::ResourceRef<T>& resource, bool addBindable = true
		);

		/** Iterates through all the Program of the Step calling the given
		 * callback function
		 *
		 * @param	callback the function to call for each Resource */
		template <typename F>
		void processPrograms(F&& callback) const;

		/** Iterates through all the Textures of the Step calling the given
		 * callback function
		 *
		 * @param	callback the function to call for each Resource */
		template <typename F>
		void processTextures(F&& callback) const;

		/** Removes the given Resource from the Step
		 *
		 * @param	resource the Resource to remove
		 * @param	removeBindable if we want also to remove the Resource
		 *			bindable or not
		 * @return	a reference to the current Step object */
		template <typename T>
		RenderableShaderStep& removeResource(
			const Repository::ResourceRef<T>& resource,
			bool removeBindable = true
		);

		/** Adds the given Bindable to the Step
		 *
		 * @param	bindable the Bindable to add
		 * @return	a reference to the current Step object */
		RenderableShaderStep& addBindable(
			const graphics::Context::BindableRef& bindable
		);

		/** Iterates through all the Bindables of the Step calling the given
		 * callback function
		 *
		 * @param	callback the function to call for each Bindable */
		template <typename F>
		void processBindables(F&& callback) const;

		/** Removes a Bindable from the current Step
		 *
		 * @param	bindable the Bindable to remove
		 * @return	a reference to the current Step object */
		RenderableShaderStep& removeBindable(
			const graphics::Context::BindableRef& bindable
		);
	};


	/**
	 * Class RenderableShader. It acts as a wrapper for the graphics Techniques
	 * so the Passes added/removed can be notifyied to the different Systems
	 */
	class RenderableShader :
		public std::enable_shared_from_this<RenderableShader>
	{
	public:		// Nested types
		using StepResource = Repository::ResourceRef<RenderableShaderStep>;
		using TechniqueSPtr = std::shared_ptr<graphics::Technique>;

	private:	// Attributes
		/** The Technique used for rendering the Renderables */
		std::shared_ptr<graphics::Technique> mTechnique;

		/** The RenderableShaderSteps added to the RenderableShader */
		std::vector<StepResource> mSteps;

		/** The EventManager used for notifying of RenderableShader updates */
		EventManager& mEventManager;

	public:		// Functions
		/** Creates a new RenderableShader
		 *
		 * @param	eventManager the EventManager used for notifying of
		 *			RenderableShader updates */
		RenderableShader(EventManager& eventManager) :
			mTechnique(std::make_shared<graphics::Technique>()),
			mEventManager(eventManager) {};

		/** @return	the Technique wrapped by the RenderableShader */
		TechniqueSPtr getTechnique() const { return mTechnique; };

		/** @return	a pointer to a copy of the current RenderableShader */
		std::unique_ptr<RenderableShader> clone() const;

		/** Adds the given RenderableShaderStep to the RenderableShader,
		 * notifying the Systems
		 *
		 * @param	step a pointer to the RenderableShaderStep to add
		 * @return	a reference to the current RenderableShader object */
		RenderableShader& addStep(const StepResource& step);

		/** Iterates through all the RenderableShaderSteps of the
		 * RenderableShader calling the given callback function
		 *
		 * @param	callback the function to call for each
		 *			RenderableShaderStep */
		template <typename F>
		void processSteps(F&& callback) const;

		/** Removes the given RenderableShaderStep from the RenderableShader,
		 * notifying the Systems
		 *
		 * @param	step a pointer to the RenderableShaderStep to remove
		 * @return	a reference to the current RenderableShader object */
		RenderableShader& removeStep(const StepResource& step);
	};


	template <typename F>
	void RenderableShaderStep::processBindables(F&& callback) const
	{
		mPass->processBindables(callback);
	}


	template <typename F>
	void RenderableShaderStep::processPrograms(F&& callback) const
	{
		for (const auto& prog : mProgramResources) {
			callback(prog);
		}
	}


	template <typename F>
	void RenderableShaderStep::processTextures(F&& callback) const
	{
		for (const auto& tex : mTextureResources) {
			callback(tex);
		}
	}


	template <typename F>
	void RenderableShader::processSteps(F&& callback) const
	{
		for (const auto& step : mSteps) {
			callback(step);
		}
	}

}

#endif		// RENDERABLE_SHADER_H
