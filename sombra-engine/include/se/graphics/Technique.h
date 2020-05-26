#ifndef TECHNIQUE_H
#define TECHNIQUE_H

#include <memory>
#include <vector>
#include <functional>
#include "core/Bindable.h"

namespace se::graphics {

	class Pass;
	class Renderable;


	/**
	 * Class Technique, TODO:
	 */
	class Technique
	{
	private:	// Nested types
		using PassSPtr = std::shared_ptr<Pass>;
		using PassCallback = std::function<void(PassSPtr)>;

	private:	// Attributes
		/** All the Passes of the Technique */
		std::vector<PassSPtr> mPasses;

	public:		// Functions
		/** Adds the given Pass to the Technique
		 *
		 * @param	pass a pointer to the Pass to add to the Technique */
		Technique& addPass(PassSPtr pass);

		/** Iterates through all the Passes of the Technique calling the given
		 * callback function
		 *
		 * @param	callback the function to call for each Pass */
		void processPasses(const PassCallback& callback);

		/** Removes a Pass from the current Technique
		 *
		 * @param	pass a pointer to the Pass to remove */
		Technique& removePass(PassSPtr pass);

		/** Submits the given Renderable for rendering with the current
		 * Technique
		 *
		 * @param	renderable the Renderable to submit
		 * @note	the renderable will be submitted to the Passes in the same
		 *			order in which they where added to the Technique */
		void submit(Renderable& renderable);
	};

}

#endif		// TECHNIQUE_H
