#ifndef BINDABLE_H
#define BINDABLE_H

#include <memory>

namespace se::graphics {

	/**
	 * Class Bindable, it represents any kind of object that can be
	 * bound/unbound in the underlying Graphics API for using in the next
	 * operations of the graphics state machine
	 */
	class Bindable
	{
	public:		// Functions
		/** Class destructor */
		virtual ~Bindable() = default;

		/** @return	a pointer to a copy of the current Bindable, nullptr if the
		 *			Bindable can't be copied */
		virtual std::unique_ptr<Bindable> clone() const { return nullptr; };

		/** Binds the current object for using it in the following operations */
		virtual void bind() const = 0;

		/** Unbinds the current object so it can't be used in the following
		 * operations */
		virtual void unbind() const = 0;
	};

}

#endif		// BINDABLE_H
