#ifndef SHADER_EVENT_H
#define SHADER_EVENT_H

#include "../RenderableShader.h"
#include "Event.h"

namespace se::app {

	/**
	 * Class ShaderEvent, its an event used for notify of a RenderableShader
	 * update
	 */
	class ShaderEvent : public Event<Topic::Shader>
	{
	public:		// Nested types
		using RenderableShaderSPtr = std::shared_ptr<RenderableShader>;
		using PassSPtr = std::shared_ptr<graphics::Pass>;

		/** The different operations to perform with the ShaderEvent */
		enum class Operation { Add, Remove };

	private:	// Attributes
		/** The Operation of the ShaderEvent */
		Operation mOperation;

		/** The RenderableShader updated */
		RenderableShaderSPtr mRenderableShader;

		/** The Pass added/removed from @see mRenderableShader */
		PassSPtr mPass;

	public:		// Functions
		/** Creates a new ShaderEvent used for notifying of a RenderableShader
		 * add/remove Operation on a Renderable Component
		 *
		 * @param	operation the Operation to perform
		 * @param	renderableShader the RenderableShader updated
		 * @param	pass the Pass added/removed */
		ShaderEvent(
			Operation operation,
			RenderableShaderSPtr renderableShader, PassSPtr pass
		) : mOperation(operation),
			mRenderableShader(renderableShader), mPass(pass) {};

		/** @return	the Operation to perform */
		Operation getOperation() const { return mOperation; };

		/** @return	a pointer to the RenderableShader to notify */
		RenderableShaderSPtr getShader() const { return mRenderableShader; };

		/** @return	a pointer to the Pass to notify */
		PassSPtr getPass() const { return mPass; };
	};

}

#endif		// SHADER_EVENT_H
