#ifndef VERTEX_BUFFER_H
#define VERTEX_BUFFER_H

namespace fe { namespace graphics {

	/**
	 * Class VertexBuffer, it's used for creating, binding and unbinding a
	 * Vertex Buffer Object
	 * <br>A Vertex Buffer Object is a buffer with the vertex data of a mesh
	 * (position, color, normals, UVs...)
	 */
	class VertexBuffer
	{
	private:	// Attributes
		/** The ID of the Buffer Array */
		unsigned int mBufferID;

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

		/** Class destructor */
		~VertexBuffer();

		/** @return	the number of components per generic Vertex Attribute */
		inline unsigned int getComponentSize() const { return mComponentSize; };

		/** Binds the Vertex Buffer Object */
		void bind() const;

		/** Unbinds the Vertex Buffer Object */
		void unbind() const;
	};

}}

#endif		// VERTEX_BUFFER_H