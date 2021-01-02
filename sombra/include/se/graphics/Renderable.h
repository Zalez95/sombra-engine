#ifndef RENDERABLE_H
#define RENDERABLE_H

#include <vector>
#include <memory>

namespace se::graphics {

	class Technique;


	/**
	 * Class Renderable, it's the interface that every Object that can be
	 * drawn must implement
	 */
	class Renderable
	{
	protected:	// Nested types
		using TechniqueSPtr = std::shared_ptr<Technique>;

	protected:	// Attributes
		/** All the Techniques of the Renderable */
		std::vector<TechniqueSPtr> mTechniques;

	public:		// Functions
		/** Creates a new Renderable */
		Renderable() = default;
		Renderable(const Renderable& other) = default;
		Renderable(Renderable&& other) = default;

		/** Class destructor */
		virtual ~Renderable() = default;

		/** Assignment operator */
		Renderable& operator=(const Renderable& other) = default;
		Renderable& operator=(Renderable&& other) = default;

		/** Adds a Technique to the current Renderable
		 *
		 * @param	technique a pointer to the Technique to add
		 * @return	a reference to the current Renderable object */
		Renderable& addTechnique(TechniqueSPtr technique);

		/** Iterates through all the technique of the Renderable calling the
		 * given callback function
		 *
		 * @param	callback the function to call for each Technique */
		template <typename F>
		void processTechniques(F callback) const;

		/** Removes a Technique from the current Renderable
		 *
		 * @param	technique a pointer to the Technique to remove
		 * @return	a reference to the current Renderable object */
		Renderable& removeTechnique(TechniqueSPtr technique);

		/** Submits the Renderable to its Techniques */
		void submit();
	};


	template <typename F>
	void Renderable::processTechniques(F callback) const
	{
		for (auto& technique : mTechniques) {
			callback(technique);
		}
	}

}

#endif		// RENDERABLE_H
