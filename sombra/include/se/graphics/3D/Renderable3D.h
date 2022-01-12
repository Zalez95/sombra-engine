#ifndef RENDERABLE_3D_H
#define RENDERABLE_3D_H

#include <unordered_map>
#include <glm/glm.hpp>
#include "../Renderable.h"

namespace se::graphics {

	class Pass;


	/**
	 * Class Renderable3D, it's a 3D Renderable. It can hold Bindables that
	 * override the behaviour of the Passes
	 */
	class Renderable3D : public Renderable
	{
	protected:	// Nested types
		using BindableRefVector = std::vector<Context::BindableRef>;

	protected:	// Attributes
		/** Maps each Pass with the Bindables to bind before drawing */
		std::unordered_map<Pass*, BindableRefVector> mPassBindables;

	public:		// Functions
		/** Creates a new Renderable3D */
		Renderable3D() = default;
		Renderable3D(const Renderable3D& other) = default;
		Renderable3D(Renderable3D&& other) = default;

		/** Class destructor */
		virtual ~Renderable3D() = default;

		/** Assignment operator */
		Renderable3D& operator=(const Renderable3D& other) = default;
		Renderable3D& operator=(Renderable3D&& other) = default;

		/** @return	the minimum and maximum position of Renderable3D in each
		 *			direction */
		virtual std::pair<glm::vec3, glm::vec3> getBounds() const = 0;

		/** Adds a Bindable to the current Renderable3D. This bindable will be
		 * bound only for the given Pass
		 *
		 * @param	pass a pointer to the Pass used for binding the Bindable
		 * @param	bindable a pointer to the Bindable to add
		 * @return	a reference to the current Renderable3D object */
		Renderable3D& addPassBindable(
			Pass* pass, const Context::BindableRef& bindable
		);

		/** Iterates through all the Bindables mapped with the given Pass of
		 * the Renderable3D calling the given callback function
		 *
		 * @param	pass a pointer to the Pass related to the Bindables
		 * @param	callback the function to call for each Bindable */
		template <typename F>
		void processPassBindables(Pass* pass, F&& callback) const;

		/** Removes all the bindables related to the given Pass
		 *
		 * @param	pass a pointer to the Pass related to the Bindables */
		Renderable3D& clearBindables(Pass* pass);

		/** Removes a Bindable from the current Renderable3D
		 *
		 * @param	pass a pointer to the Pass used for binding the Bindable
		 * @param	bindable a pointer to the Bindable to remove
		 * @return	a reference to the current Renderable3D object */
		Renderable3D& removePassBindable(
			Pass* pass, const Context::BindableRef& bindable
		);

		/** Binds all the Bindables added to the current Renderable3D related
		 * to the given Pass
		 *
		 * @param	q the Context Query object used for accesing to the
		 *			Bindables
		 * @param	pass a pointer to the Pass used for binding the Bindables */
		void bind(Context::Query& q, Pass* pass) const;

		/** Unbinds all the Bindables added to the current Renderable3D related
		 * to the given Pass
		 *
		 * @param	q the Context Query object used for accesing to the
		 *			Bindables
		 * @param	pass a pointer to the Pass used for binding the Bindables */
		void unbind(Context::Query& q, Pass* pass) const;
	};


	template <typename F>
	void Renderable3D::processPassBindables(Pass* pass, F&& callback) const
	{
		auto it = mPassBindables.find(pass);
		if (it != mPassBindables.end()) {
			for (auto& bindable : it->second) {
				callback(bindable);
			}
		}
	}

}

#endif		// RENDERABLE_3D_H
