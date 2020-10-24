#ifndef RENDERABLE_3D_H
#define RENDERABLE_3D_H

#include <unordered_map>
#include "../Renderable.h"

namespace se::graphics {

	class Pass;
	class Bindable;


	/**
	 * Class Renderable3D, it's a 3D Renderable. It can hold Bindables that
	 * override the behaviour of the Passes
	 */
	class Renderable3D : public Renderable
	{
	private:	// Nested types
		using BindableSPtr = std::shared_ptr<Bindable>;

	private:	// Attributes
		/** Maps each Pass with the Bindables to bind before drawing */
		std::unordered_map<Pass*, std::vector<BindableSPtr>> mPassBindables;

	public:		// Functions
		/** Class destructor */
		virtual ~Renderable3D() = default;

		/** Adds a Bindable to the current Renderable3D. This bindable will be
		 * bound only for the given Pass
		 *
		 * @param	pass a pointer to the Pass used for binding the Bindable
		 * @param	bindable a pointer to the Bindable to add
		 * @return	a reference to the current Renderable3D object */
		Renderable3D& addPassBindable(Pass* pass, BindableSPtr bindable);

		/** Removes a Bindable from the current Renderable3D
		 *
		 * @param	pass a pointer to the Pass used for binding the Bindable
		 * @param	bindable a pointer to the Bindable to remove
		 * @return	a reference to the current Renderable3D object */
		Renderable3D& removePassBindable(Pass* pass, BindableSPtr bindable);

		/** Binds all the Bindables added to the current Renderable3D related
		 * to the given Pass
		 *
		 * @param	pass a pointer to the Pass used for binding the Bindables */
		void bind(Pass* pass) const;

		/** Unbinds all the Bindables added to the current Renderable3D related
		 * to the given Pass
		 *
		 * @param	pass a pointer to the Pass used for binding the Bindables */
		void unbind(Pass* pass) const;

		/** Draws the current Renderable3D (drawcall) */
		virtual void draw() = 0;
	};

}

#endif		// RENDERABLE_3D_H
