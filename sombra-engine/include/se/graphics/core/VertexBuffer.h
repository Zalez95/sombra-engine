#ifndef VERTEX_BUFFER_H
#define VERTEX_BUFFER_H

#include "Bindable.h"
#include "Constants.h"

namespace se::graphics {

	/**
	 * Class VertexBuffer, it's used for creating, binding and unbinding a
	 * Vertex Buffer Object
	 *
	 * A Vertex Buffer Object is a buffer with the vertex data of a Mesh
	 * (position, color, normals, UVs...)
	 */
	class VertexBuffer : public Bindable
	{
	private:	// Attributes
		/** The id of the Buffer Array */
		unsigned int mBufferId;

	public:		// Functions
		/** Creates a new VertexBuffer */
		VertexBuffer();
		VertexBuffer(const VertexBuffer& other) = delete;
		VertexBuffer(VertexBuffer&& other);

		/** Class destructor */
		~VertexBuffer();

		/** Assignment operator */
		VertexBuffer& operator=(const VertexBuffer& other) = delete;
		VertexBuffer& operator=(VertexBuffer&& other);

		/** Resizes and sets the buffer data
		 *
		 * @param	data a pointer to the data of the buffer
		 * @param	count the number of elements in the data array */
		template <typename T>
		void resizeAndCopy(const T* data, std::size_t count);

		/** Resizes and sets the buffer data
		 *
		 * @param	data a pointer to the data of the new buffer
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

		/** Binds the Vertex Buffer Object */
		void bind() const override;

		/** Unbinds the Vertex Buffer Object */
		void unbind() const override;
	};


	template <typename T>
	void VertexBuffer::resizeAndCopy(const T* data, std::size_t count)
	{
		resizeAndCopy(static_cast<const void*>(data), count * sizeof(T));
	}


	template <typename T>
	void VertexBuffer::copy(
		const T* data, std::size_t count, std::size_t offset
	) {
		copy(
			static_cast<const void*>(data), count * sizeof(T),
			offset * sizeof(T)
		);
	}

}

#endif		// VERTEX_BUFFER_H
