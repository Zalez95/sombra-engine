#ifndef RENDERER_3D_H
#define RENDERER_3D_H

#include "../Renderer.h"

namespace se::graphics {

	class Renderable3D;


	/**
	 * Class Renderer3D, it's a Renderer used for rendering 3D Renderables.
	 */
	class Renderer3D : public Renderer
	{
	public:		// Nested types
		/** Class Filter, it's used for checking if the Renderable3Ds should be
		 * rendered or not (filter them) */
		class Filter
		{
		public:		// Functions
			/** Class destructor */
			virtual ~Filter() = default;

			/** Checks if the given Renderable3D should be submitted for
			 * rendering
			 *
			 * @param	renderable the Renderable3D to check
			 * @return	true if it should be submitted for rendering, false
			 *			otherwise */
			virtual bool shouldBeRendered(Renderable3D& renderable) = 0;
		};

		using FilterSPtr = std::shared_ptr<Filter>;

	protected:	// Attributes
		/** The Filters used for testing if the Renderable3Ds should be
		 * rendered or not */
		std::vector<FilterSPtr> mFilters;

	public:		// Functions
		/** Creates a new Renderer3D
		 *
		 * @param	name the name of the new Renderer3D */
		Renderer3D(const std::string& name) : Renderer(name) {};

		/** Class destructor */
		virtual ~Renderer3D() = default;

		/** Adds the given Filter to the Renderer3D
		 *
		 * @param	filter a pointer to the new Filter to add
		 * @return	the current Renderer3D object */
		Renderer3D& addFilter(FilterSPtr filter);

		/** Iterates through all the Filters of the Renderer3D calling the given
		 * callback function
		 *
		 * @param	callback the function to call for each Filter */
		template <typename F>
		void processFilters(F&& callback) const;

		/** Removes the given Filter from the Renderer3D
		 *
		 * @param	filter a pointer to the Filter to remove
		 * @return	the current Renderer3D object */
		Renderer3D& removeFilter(FilterSPtr filter);

		/** @copydoc Renderer::submit(Renderable&, Pass&)
		 * @note	the renderable will be tested with the Filters prior to
		 *			being submitted */
		virtual void submit(Renderable& renderable, Pass& pass) override;
	protected:
		/** Submits the given Renderable3D for rendering
		 *
		 * @param	renderable the Renderable3D to submit for rendering
		 * @param	pass the Pass with which the Renderable3D will be drawn */
		virtual
		void submitRenderable3D(Renderable3D& renderable, Pass& pass) = 0;
	};


	template <typename F>
	void Renderer3D::processFilters(F&& callback) const
	{
		for (auto& filter : mFilters) {
			callback(filter);
		}
	}

}

#endif		// RENDERER_3D_H
