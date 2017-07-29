#ifndef INDEX_BUFFER_H
#define INDEX_BUFFER_H

#include <GL/glew.h>

namespace graphics {

	/**
	 * Class IndexBuffer, it's used for creating, binding and unbinding an
	 * Index Buffer Object.
	 * <br>An Index Buffer Object is a buffer with the indices to the vertices
	 * that form the faces of the mesh
	 */
	class IndexBuffer
	{
	private:	// Attributes
		/** The ID of the index buffer */
		GLuint mBufferID;

		/** The number of indices of the buffer */
		GLuint mIndexCount;

	public:		// Functions
		/** Creates a new IndexBuffer
		 * 
		 * @param	data a pointer to the data of the of the buffer
		 * @param	count the number of components in the data array */
		IndexBuffer(const GLushort* data, GLuint count);

		/** Class destructor */
		~IndexBuffer();

		/** @return	the number of indices the buffer */
		GLuint getIndexCount() const { return mIndexCount; };

		/** Binds te Index Buffer Object */
		void bind() const;

		/** Unbinds the Index Buffer Object */
		void unbind() const;
	};

}

#endif		// INDEX_BUFFER_H
