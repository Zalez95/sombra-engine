#ifndef TECHNIQUE_H
#define TECHNIQUE_H

#include <memory>
#include <vector>
#include "core/Bindable.h"

namespace se::graphics {

	class Pass;
	class Renderable;


	/**
	 * Class Technique, it's a set of Passes used for rendering Renderables
	 */
	class Technique
	{
	private:	// Nested types
		using PassSPtr = std::shared_ptr<Pass>;

	private:	// Attributes
		/** All the Passes of the Technique */
		std::vector<PassSPtr> mPasses;

	public:		// Functions
		/** Adds the given Pass to the Technique
		 *
		 * @param	pass a pointer to the Pass to add to the Technique
		 * @return	a reference to the current Technique object */
		Technique& addPass(const PassSPtr& pass);

		/** Iterates through all the Passes of the Technique calling the given
		 * callback function
		 *
		 * @param	callback the function to call for each Pass */
		template <typename F>
		void processPasses(F&& callback) const;

		/** Removes a Pass from the current Technique
		 *
		 * @param	pass a pointer to the Pass to remove
		 * @return	a reference to the current Technique object */
		Technique& removePass(const PassSPtr& pass);

		/** Submits the given Renderable for rendering with the current
		 * Technique
		 *
		 * @param	renderable the Renderable to submit
		 * @note	the renderable will be submitted to the Passes in the same
		 *			order in which they where added to the Technique */
		void submit(Renderable& renderable);
	};


	template <typename F>
	void Technique::processPasses(F&& callback) const
	{
		for (auto& pass : mPasses) {
			callback(pass);
		}
	}

}

#endif		// TECHNIQUE_H
