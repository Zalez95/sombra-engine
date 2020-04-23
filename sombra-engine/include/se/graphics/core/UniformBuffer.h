#ifndef UNIFORM_BUFFER_H
#define UNIFORM_BUFFER_H

#include <cstddef>
#include "Bindable.h"

namespace se::graphics {

	/**
	 * Class UniformBuffer, it's a buffer used for holding uniform variables
	 * that can be shared between multiple programs
	 */
	class UniformBuffer : public Bindable
	{
	private:	// Attributes
		/** The id of the Uniform Buffer Object */
		unsigned int mBufferId;

		/** The binding point where the Uniform Buffer Object will be bound */
		unsigned int mSlot;

	public:		// Functions
		/** Creates a new UniformBuffer */
		UniformBuffer();

		UniformBuffer(const UniformBuffer& other) = delete;
		UniformBuffer(UniformBuffer&& other);

		/** Class destructor */
		~UniformBuffer();

		/** Assignment operator */
		UniformBuffer& operator=(const UniformBuffer& other) = delete;
		UniformBuffer& operator=(UniformBuffer&& other);

		/** Sets the binding point of the UniformBuffer
		 *
		 * @param	slot the binding point where the Uniform Buffer Object will
		 *			be bound */
		void setBindingPoint(unsigned int slot) { mSlot = slot; };

		/** Sets the buffer data
		 *
		 * @param	data a pointer to the data of the buffer
		 * @param	count the number of elements in the data array */
		template <typename T>
		void copy(const T* data, std::size_t count = 1);

		/** Sets the buffer data
		 *
		 * @param	data a pointer to the data of the new buffer
		 * @param	size the size of the data buffer */
		void copy(const void* data, std::size_t size);

		/** Binds the Uniform Buffer Object */
		void bind() const override;

		/** Unbinds the Uniform Buffer Object */
		void unbind() const override;
	};


	template <typename T>
	void UniformBuffer::copy(const T* data, std::size_t count)
	{
		copy(static_cast<const void*>(data), count * sizeof(T));
	}

}

#endif		// UNIFORM_BUFFER_H
