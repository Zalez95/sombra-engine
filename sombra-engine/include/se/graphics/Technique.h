#ifndef TECHNIQUE_H
#define TECHNIQUE_H

#include <vector>
#include <memory>
#include <functional>
#include "core/Bindable.h"

namespace se::graphics {

	class Renderable;


	/**
	 * Class Step, TODO:
	 */
	class Step : public Bindable
	{
	protected:	// Nested types
		using BindableSPtr = std::shared_ptr<Bindable>;

	protected:	// Attributes
		/** All the Bindables of the Step */
		std::vector<BindableSPtr> mBindables;

	public:		// Functions
		/** Class destructor */
		virtual ~Step() = default;

		/** Submits the given Renderable for rendering with the current Step
		 *
		 * @param	renderable the Renderable to submit */
		virtual void submit(Renderable& renderable) = 0;

		/** Adds the given Bindable to the Step
		 *
		 * @param	bindable a pointer to the Bindable to add */
		Step& addBindable(BindableSPtr bindable);

		/** Removes a Bindable from the current Step
		 *
		 * @param	bindable a pointer to the Bindable to remove */
		Step& removeBindable(BindableSPtr bindable);

		/** Binds the current Step for using it in the following operations
		 * @note	the Bindables added to the Step will be bound in the same
		 *			order in which they where added to the Step */
		void bind() const override;

		/** Unbinds the current Step so it can't be used in the following
		 * operations
		 * @note	the Bindables added to the Step will be unbound in the
		 *			reverse order in which they where added to the Step */
		void unbind() const override;
	};


	/**
	 * Class Technique, TODO:
	 */
	class Technique
	{
	private:	// Nested types
		using StepSPtr = std::shared_ptr<Step>;
		using StepCallback = std::function<void(StepSPtr)>;

	private:	// Attributes
		/** All the Steps of the Technique */
		std::vector<StepSPtr> mSteps;

	public:		// Functions
		/** Adds the given Step to the Technique
		 *
		 * @param	step a pointer to the Step to add to the Technique */
		Technique& addStep(StepSPtr step);

		/** Iterates through all the steps of the Technique calling the given
		 * callback function
		 *
		 * @param	callback the function to call for each Step */
		void processSteps(const StepCallback& callback);

		/** Removes a Step from the current Technique
		 *
		 * @param	step a pointer to the Step to remove */
		Technique& removeStep(StepSPtr step);

		/** Submits the given Renderable for rendering with the current
		 * Technique
		 *
		 * @param	renderable the Renderable to submit
		 * @note	the renderable will be submitted to the Steps in the same
		 *			order in which they where added to the Technique */
		void submit(Renderable& renderable);
	};

}

#endif		// TECHNIQUE_H
