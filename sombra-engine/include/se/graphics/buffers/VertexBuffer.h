#ifndef VERTEX_BUFFER_H
#define VERTEX_BUFFER_H

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
		/** Creates a new VertexBuffer
		 *
		 * @param	data a pointer to the data of the buffer
		 * @param	size the size of the data buffer */
		VertexBuffer(const void* data, std::size_t size);

		/** Creates a new VertexBuffer
		 *
		 * @param	data a pointer to the data of the buffer
		 * @param	count the number of components in the data array */
		template <typename T>
		VertexBuffer(const T* data, std::size_t count);

		VertexBuffer(const VertexBuffer& other) = delete;
		VertexBuffer(VertexBuffer&& other);

		/** Class destructor */
		~VertexBuffer();

		/** Assignment operator */
		VertexBuffer& operator=(const VertexBuffer& other) = delete;
		VertexBuffer& operator=(VertexBuffer&& other);

		/** Binds the Vertex Buffer Object */
		void bind() const;

		/** Unbinds the Vertex Buffer Object */
		void unbind() const;
	private:
		/** Creates the actual buffer
		 *
		 * @param	data a pointer to the data of the new buffer
		 * @param	size the size of the data buffer */
		void createBuffer(const void* data, std::size_t size);
	};


	template <typename T>
	VertexBuffer::VertexBuffer(const T* data, std::size_t count)
	{
		createBuffer(data, count * sizeof(T));
	}

}

#endif		// VERTEX_BUFFER_H
