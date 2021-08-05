#ifndef SHADER_EVENT_H
#define SHADER_EVENT_H

#include "../graphics/RenderableShader.h"
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
		using RenderableShaderStepSPtr = std::shared_ptr<RenderableShaderStep>;

		/** The different operations to perform with the ShaderEvent */
		enum class Operation { Add, Remove };

	private:	// Attributes
		/** The Operation of the ShaderEvent */
		Operation mOperation;

		/** The RenderableShader updated */
		RenderableShaderSPtr mRenderableShader;

		/** The RenderableShaderStep added/removed from
		 * @see mRenderableShader */
		RenderableShaderStepSPtr mStep;

	public:		// Functions
		/** Creates a new ShaderEvent used for notifying of a RenderableShader
		 * add/remove Operation on a Renderable Component
		 *
		 * @param	operation the Operation to perform
		 * @param	renderableShader the RenderableShader updated
		 * @param	step the Step added/removed */
		ShaderEvent(
			Operation operation,
			const RenderableShaderSPtr& renderableShader,
			const RenderableShaderStepSPtr& step
		) : mOperation(operation),
			mRenderableShader(renderableShader), mStep(step) {};

		/** @return	the Operation to perform */
		Operation getOperation() const { return mOperation; };

		/** @return	a pointer to the RenderableShader to notify */
		const RenderableShaderSPtr& getShader() const
		{ return mRenderableShader; };

		/** @return	a pointer to the RenderableShaderStep to notify */
		const RenderableShaderStepSPtr& getStep() const
		{ return mStep; };
	private:
		/** Prints the operation to the given stream
		 *
		 * @param	os the stream to print to
		 * @param	o the operation to print
		 * @return	the stream */
		friend constexpr std::ostream& operator<<(
			std::ostream& os, const Operation& o
		) {
			switch (o) {
				case Operation::Add:	return os << "Operation::Add";
				case Operation::Remove:	return os << "Operation::Remove";
				default:				return os;
			}
		};

		/** @copydoc Event::printTo() */
		virtual void printTo(std::ostream& os) const override
		{
			os	<< "{ kTopic : " << kTopic
				<< ", mRenderableShader : " << mRenderableShader.get()
				<< ", mStep : " << mStep.get() << " }";
		};
	};

}

#endif		// SHADER_EVENT_H
