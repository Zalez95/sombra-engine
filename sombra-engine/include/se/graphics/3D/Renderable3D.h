#ifndef RENDERABLE_3D_H
#define RENDERABLE_3D_H

#include "../Renderable.h"
#include "../core/Bindable.h"

namespace se::graphics {

	/**
	 * Class Renderable3D, it's a 3D Renderable that can be drawn with a
	 * Technique
	 */
	class Renderable3D : public Renderable, Bindable
	{
	private:	// Nested types
		using BindableSPtr = std::shared_ptr<Bindable>;
		using BindableCallback = std::function<void(const BindableSPtr&)>;

	private:	// Attributes
		/** All the Bindables of the Renderable3D */
		std::vector<BindableSPtr> mBindables;

	public:		// Functions
		/** Class destructor */
		virtual ~Renderable3D() = default;

		/** Adds a Bindable to the current Renderable3D
		 *
		 * @param	bindable a pointer to the Bindable to add
		 * @return	a reference to the current Renderable3D object */
		Renderable3D& addBindable(BindableSPtr bindable);

		/** Iterates through all the Bindables of the Renderable calling the
		 * given callback function
		 *
		 * @param	callback the function to call for each Bindable */
		void processBindables(const BindableCallback& callback);

		/** Removes a Bindable from the current Renderable3D
		 *
		 * @param	bindable a pointer to the Bindable to remove
		 * @return	a reference to the current Renderable3D object */
		Renderable3D& removeBindable(BindableSPtr bindable);

		/** Draws the current Renderable3D (drawcall) */
		virtual void draw() = 0;

		/** Binds the Bindables of the Renderable3D
		 * @note	the Bindables added to the Renderable3D will be bound in
		 *			the same order in which they where added to the
		 *			Renderable3D */
		virtual void bind() const override;

		/** Unbinds the Bindables of the Renderable3D
		 * @note	the Bindables added to the Renderable3D will be unbound in
		 *			the reverse order in which they where added to the
		 *			Renderable3D */
		virtual void unbind() const override;
	};

}

#endif		// RENDERABLE_3D_H
