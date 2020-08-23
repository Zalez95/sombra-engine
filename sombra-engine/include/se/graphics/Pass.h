#ifndef PASS_H
#define PASS_H

#include <memory>
#include <vector>
#include <functional>
#include "core/Bindable.h"

namespace se::graphics {

	class Renderable;
	class Renderer;


	/**
	 * Class Pass, It's a Bindable that represents one of the multiple Steps
	 * that a Technique can be splitted into
	 */
	class Pass : public Bindable
	{
	private:	// Nested types
		using BindableSPtr = std::shared_ptr<Bindable>;
		using BindableCallback = std::function<void(const BindableSPtr&)>;

	private:	// Attributes
		/** The Renderer where the Renderables will be submitted */
		Renderer& mRenderer;

		/** All the Bindables of the Pass */
		std::vector<BindableSPtr> mBindables;

	public:		// Functions
		/** Creates a new Pass
		 *
		 * @param	renderer the Renderer of the Pass */
		Pass(Renderer& renderer) : mRenderer(renderer) {};

		/** Submits the given Renderable for rendering with the current Pass
		 *
		 * @param	renderable the Renderable to submit */
		void submit(Renderable& renderable);

		/** Adds the given Bindable to the Pass
		 *
		 * @param	bindable a pointer to the Bindable to add
		 * @return	a reference to the current Pass object */
		Pass& addBindable(BindableSPtr bindable);

		/** Iterates through all the Bindables of the Pass calling the given
		 * callback function
		 *
		 * @param	callback the function to call for each Bindable */
		void processBindables(const BindableCallback& callback);

		/** Removes a Bindable from the current Pass
		 *
		 * @param	bindable a pointer to the Bindable to remove
		 * @return	a reference to the current Pass object */
		Pass& removeBindable(BindableSPtr bindable);

		/** Binds the current Pass for using it in the following operations
		 * @note	the Bindables added to the Pass will be bound in the same
		 *			order in which they where added to the Pass */
		void bind() const override;

		/** Unbinds the current Pass so it can't be used in the following
		 * operations
		 * @note	the Bindables added to the Pass will be unbound in the
		 *			reverse order in which they where added to the Pass */
		void unbind() const override;
	};

}

#endif		// PASS_H
