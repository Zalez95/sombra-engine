#ifndef PASS_H
#define PASS_H

#include "Context.h"

namespace se::graphics {

	class Renderable;
	class Renderer;


	/**
	 * Class Pass, It's a Bindable that represents one of the multiple steps
	 * that a Technique can be splitted into. It's used for rendering
	 * multiple Renderables using a single Renderer. It can also hold a common
	 * state to all those Renderables that will be used during the rendering.
	 */
	class Pass
	{
	private:	// Attributes
		/** The Renderer where the Renderables will be submitted */
		Renderer* mRenderer;

		/** All the Bindables of the Pass */
		std::vector<Context::BindableRef> mBindables;

	public:		// Functions
		/** Creates a new Pass
		 *
		 * @param	renderer the Renderer of the Pass */
		Pass(Renderer& renderer) : mRenderer(&renderer) {};

		/** Class destructor */
		virtual ~Pass() = default;

		/** @return	the Renderer used by the Pass for submitting the meshes */
		Renderer& getRenderer() { return *mRenderer; };

		/** @return	the Renderer used by the Pass for submitting the meshes */
		const Renderer& getRenderer() const { return *mRenderer; };

		/** Submits the given Renderable for rendering with the current Pass
		 *
		 * @param	renderable the Renderable to submit */
		void submit(Renderable& renderable);

		/** Adds the given Bindable to the Pass
		 *
		 * @param	bindable a reference to the Bindable to add
		 * @return	a reference to the current Pass object */
		Pass& addBindable(const Context::BindableRef& bindable);

		/** Iterates through all the Bindables of the Pass calling the given
		 * callback function
		 *
		 * @param	callback the function to call for each Bindable */
		template <typename F>
		void processBindables(F&& callback) const;

		/** Removes a Bindable from the current Pass
		 *
		 * @param	bindable a reference to the Bindable to remove
		 * @return	a reference to the current Pass object */
		Pass& removeBindable(const Context::BindableRef& bindable);

		/** Binds the current Pass for using it in the following operations
		 *
		 * @param	q the Context Query object used for accesing to the
		 *			Bindables
		 * @note	the Bindables added to the Pass will be bound in the same
		 *			order in which they where added to the Pass */
		virtual void bind(Context::Query& q) const;

		/** Unbinds the current Pass so it can't be used in the following
		 * operations
		 *
		 * @param	q the Context Query object used for accesing to the
		 *			Bindables
		 * @note	the Bindables added to the Pass will be unbound in the
		 *			reverse order in which they where added to the Pass */
		virtual void unbind(Context::Query& q) const;
	};


	template <typename F>
	void Pass::processBindables(F&& callback) const
	{
		for (auto& bindable : mBindables) {
			callback(bindable);
		}
	}

}

#endif		// PASS_H
