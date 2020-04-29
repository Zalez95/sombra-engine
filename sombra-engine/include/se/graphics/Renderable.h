#ifndef RENDERABLE_H
#define RENDERABLE_H

#include <vector>
#include <memory>
#include <functional>

namespace se::graphics {

	class Technique;


	/**
	 * Class Renderable, it's any Object that can be drawn with the usage of
	 * a Technique
	 */
	class Renderable
	{
	protected:	// Nested types
		using TechniqueSPtr = std::shared_ptr<Technique>;
		using TechniqueCallback = std::function<void(TechniqueSPtr)>;

	protected:	// Attributes
		/** All the Techniques of the Renderable */
		std::vector<TechniqueSPtr> mTechniques;

	public:		// Functions
		/** Class destructor */
		virtual ~Renderable() = default;

		/** Adds a Technique to the current Renderable
		 *
		 * @param	technique a pointer to the Technique to add */
		void addTechnique(TechniqueSPtr technique);

		/** Iterates through all the technique of the Renderable calling the
		 * given callback function
		 *
		 * @param	callback the function to call for each Technique */
		void processTechniques(const TechniqueCallback& callback);

		/** Removes a Technique from the current Renderable
		 *
		 * @param	technique a pointer to the Technique to remove */
		void removeTechnique(TechniqueSPtr technique);

		/** Submits the Renderable to its Techniques */
		void submit();
	};

}

#endif		// RENDERABLE_H
