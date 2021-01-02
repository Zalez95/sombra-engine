#ifndef RENDERABLE_SHADER_H
#define RENDERABLE_SHADER_H

#include "../graphics/Technique.h"
#include "events/EventManager.h"

namespace se::app {

	/**
	 * Class RenderableShader. It acts as a wrapper for the graphics Techniques
	 * so the Passes added/removed can be notifyied to the different Systems
	 */
	class RenderableShader :
		public std::enable_shared_from_this<RenderableShader>
	{
	private:
		using PassSPtr = std::shared_ptr<graphics::Pass>;
		using TechniqueSPtr = std::shared_ptr<graphics::Technique>;

	private:
		/** The Technique used for rendering the Renderables */
		TechniqueSPtr mTechnique;

		/** The EventManager used for notifying of RenderableShader updates */
		EventManager& mEventManager;

	public:
		/** Creates a new RenderableShader
		 *
		 * @param	eventManager the EventManager used for notifying of
		 *			RenderableShader updates */
		RenderableShader(EventManager& eventManager) :
			mTechnique(std::make_shared<graphics::Technique>()),
			mEventManager(eventManager) {};

		/** @return	the Technique wrapped by the RenderableShader */
		TechniqueSPtr getTechnique() const { return mTechnique; };

		/** Adds the given pass to the RenderableShader, notifying the Systems
		 *
		 * @param	pass a pointer to the Pass to add
		 * @return	a reference to the current RenderableShader object */
		RenderableShader& addPass(const PassSPtr& pass);

		/** Removes the given pass from the RenderableShader, notifying the
		 * Systems
		 *
		 * @param	pass a pointer to the Pass to remove
		 * @return	a reference to the current RenderableShader object */
		RenderableShader& removePass(const PassSPtr& pass);
	};

}

#endif		// RENDERABLE_SHADER_H
