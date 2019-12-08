#ifndef VERTEX_BUFFER_H
#define VERTEX_BUFFER_H

#include "Constants.h"

namespace se::graphics {

	/**
	 * Class VertexBuffer, it's used for creating, binding and unbinding a
	 * Vertex Buffer Object
	 *
	 * A Vertex Buffer Object is a buffer with the vertex data of a Mesh
	 * (position, color, normals, UVs...)
	 */
	class VertexBuffer
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

		/** Sets the buffer data
		 *
		 * @param	data a pointer to the data of the buffer
		 * @param	count the number of elements in the data array */
		template <typename T>
		void setData(const T* data, std::size_t count);

		/** Sets the buffer data
		 *
		 * @param	data a pointer to the data of the new buffer
		 * @param	size the size of the data buffer */
		void setData(const void* data, std::size_t size);

		/** Binds the Vertex Buffer Object */
		void bind() const;

		/** Unbinds the Vertex Buffer Object */
		void unbind() const;
	};


	template <typename T>
	void VertexBuffer::setData(const T* data, std::size_t count)
	{
		setData(static_cast<const void*>(data), count * sizeof(T));
	}

}

#endif		// VERTEX_BUFFER_H
