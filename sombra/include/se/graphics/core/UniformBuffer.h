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

		/** @return	the size of the contents of the buffer in number of
		 *			elements */
		template <typename T>
		std::size_t size() const;

		/** @return	the size of the contents of the buffer in bytes */
		std::size_t size() const;

		/** Sets the binding point of the UniformBuffer
		 *
		 * @param	slot the binding point where the Uniform Buffer Object will
		 *			be bound */
		void setBindingPoint(unsigned int slot) { mSlot = slot; };

		/** Resizes and sets the buffer data
		 *
		 * @param	data a pointer to the data of the buffer, if it's nullptr
		 *			it will only resize the buffer
		 * @param	count the number of elements in the data array */
		template <typename T>
		void resizeAndCopy(const T* data, std::size_t count);

		/** Resizes and sets the buffer data
		 *
		 * @param	data a pointer to the data of the new buffer, if it's
		 *			nullptr it will only resize the buffer
		 * @param	size the size of the data buffer */
		void resizeAndCopy(const void* data, std::size_t size);

		/** Sets the buffer data
		 *
		 * @param	data a pointer to the data of the buffer
		 * @param	count the number of elements in the data array
		 * @param	offset the offset into the buffer where the data will be
		 *			copied */
		template <typename T>
		void copy(const T* data, std::size_t count, std::size_t offset = 0);

		/** Sets the buffer data
		 *
		 * @param	data a pointer to the data of the new buffer
		 * @param	size the size of the data buffer
		 * @param	offset the offset into the buffer where the data will be
		 *			copied */
		void copy(const void* data, std::size_t size, std::size_t offset = 0);

		/** @copydoc Bindable::clone() */
		virtual std::unique_ptr<Bindable> clone() const override;

		/** Binds the Uniform Buffer Object */
		virtual void bind() const override;

		/** Unbinds the Uniform Buffer Object */
		virtual void unbind() const override;
	};


	template <typename T>
	std::size_t UniformBuffer::size() const
	{
		return size() / sizeof(T);
	}


	template <typename T>
	void UniformBuffer::resizeAndCopy(const T* data, std::size_t count)
	{
		resizeAndCopy(static_cast<const void*>(data), count * sizeof(T));
	}


	template <typename T>
	void UniformBuffer::copy(
		const T* data, std::size_t count, std::size_t offset
	) {
		copy(
			static_cast<const void*>(data), count * sizeof(T),
			offset * sizeof(T)
		);
	}

}

#endif		// UNIFORM_BUFFER_H
