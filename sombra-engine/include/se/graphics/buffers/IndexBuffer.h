#ifndef INDEX_BUFFER_H
#define INDEX_BUFFER_H

namespace se::graphics {

	/**
	 * Class IndexBuffer, it's used for creating, binding and unbinding an
	 * Index Buffer Object.
	 *
	 * An Index Buffer Object is a buffer with the indices to the vertices
	 * that form the faces of the mesh
	 */
	class IndexBuffer
	{
	private:	// Attributes
		/** The id of the index buffer */
		unsigned int mBufferId;

		/** The number of indices of the buffer */
		unsigned int mIndexCount;

	public:		// Functions
		/** Creates a new IndexBuffer
		 *
		 * @param	data a pointer to the data of the of the buffer
		 * @param	count the number of components in the data array */
		IndexBuffer(const unsigned short* data, unsigned int count);
		IndexBuffer(const IndexBuffer& other) = delete;
		IndexBuffer(IndexBuffer&& other);

		/** Class destructor */
		~IndexBuffer();

		/** Assignment operator */
		IndexBuffer& operator=(const IndexBuffer& other) = delete;
		IndexBuffer& operator=(IndexBuffer&& other);

		/** @return	the number of indices the buffer */
		unsigned int getIndexCount() const { return mIndexCount; };

		/** Binds te Index Buffer Object */
		void bind() const;

		/** Unbinds the Index Buffer Object */
		void unbind() const;
	};

}

#endif		// INDEX_BUFFER_H
