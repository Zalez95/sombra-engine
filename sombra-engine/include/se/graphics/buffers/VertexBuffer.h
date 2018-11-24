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

		/** The number of components per generic Vertex Attribute */
		unsigned int mComponentSize;

	public:		// Functions
		/** Creates a new VertexBuffer
		 *
		 * @param	data a pointer to the data of the buffer (float)
		 * @param	count the number of components in the data array
		 * @param	componentSize the number of components per generic
		 * 			Vertex Attribute */
		VertexBuffer(
			const float* data,
			unsigned int count, unsigned int componentSize
		);

		/** Creates a new VertexBuffer
		 *
		 * @param	data a pointer to the data of the buffer (unsigned short)
		 * @param	count the number of components in the data array
		 * @param	componentSize the number of components per generic
		 * 			Vertex Attribute */
		VertexBuffer(
			const unsigned short* data,
			unsigned int count, unsigned int componentSize
		);
		VertexBuffer(const VertexBuffer& other) = delete;
		VertexBuffer(VertexBuffer&& other);

		/** Class destructor */
		~VertexBuffer();

		/** Assignment operator */
		VertexBuffer& operator=(const VertexBuffer& other) = delete;
		VertexBuffer& operator=(VertexBuffer&& other);

		/** @return	the number of components per generic Vertex Attribute */
		inline unsigned int getComponentSize() const { return mComponentSize; };

		/** Binds the Vertex Buffer Object */
		void bind() const;

		/** Unbinds the Vertex Buffer Object */
		void unbind() const;
	};

}

#endif		// VERTEX_BUFFER_H
