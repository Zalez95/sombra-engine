#ifndef UNIFORM_BUFFER_H
#define UNIFORM_BUFFER_H

#include "Constants.h"

namespace se::graphics {

	/**
	 * Class UniformBuffer, it's a buffer used to hold uniform variables that
	 * can be shared between multiple programs
	 */
	class UniformBuffer
	{
	private:	// Attributes
		/** The id of the Uniform Buffer Object */
		unsigned int mBufferId;

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

		/** Sets the buffer data
		 *
		 * @param	data a pointer to the data of the buffer
		 * @param	count the number of elements in the data array */
		template <typename T>
		void setData(const T* data, std::size_t count = 1);

		/** Sets the buffer data
		 *
		 * @param	data a pointer to the data of the new buffer
		 * @param	size the size of the data buffer */
		void setData(const void* data, std::size_t size);

		/** Binds the Vertex Buffer Object
		 *
		 * @param	slot the binding point where the Uniform Buffer Object will
		 *			be bound */
		void bind(unsigned int slot) const;

		/** Unbinds the Vertex Buffer Object */
		void unbind() const;
	};


	template <typename T>
	void UniformBuffer::setData(const T* data, std::size_t count)
	{
		setData(static_cast<const void*>(data), count * sizeof(T));
	}

}

#endif		// UNIFORM_BUFFER_H
