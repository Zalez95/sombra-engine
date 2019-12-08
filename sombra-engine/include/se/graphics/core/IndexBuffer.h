#ifndef INDEX_BUFFER_H
#define INDEX_BUFFER_H

#include "Constants.h"

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

		/** The TypeId of the indices of the buffer */
		TypeId mIndexType;

		/** The number of indices of the buffer */
		std::size_t mIndexCount;

	public:		// Functions
		/** Creates a new IndexBuffer
		 *
		 * @param	data a pointer to the data of the buffer
		 * @param	size the size of the data buffer
		 * @param	type the TypeId of the data of the buffer
		 * @param	count the number of indices in the buffer */
		IndexBuffer(
			const void* data, std::size_t size,
			TypeId type, std::size_t count
		);

		/** Creates a new IndexBuffer
		 *
		 * @param	data a pointer to the data of the buffer
		 * @param	type the TypeId of the data of the buffer
		 * @param	count the number of indices in the buffer */
		template <typename T>
		IndexBuffer(const T* data, TypeId type, std::size_t count);

		IndexBuffer(const IndexBuffer& other) = delete;
		IndexBuffer(IndexBuffer&& other);

		/** Class destructor */
		~IndexBuffer();

		/** Assignment operator */
		IndexBuffer& operator=(const IndexBuffer& other) = delete;
		IndexBuffer& operator=(IndexBuffer&& other);

		/** @return	the TypeId of the indices of the buffer */
		TypeId getIndexType() const { return mIndexType; };

		/** @return	the number of indices the buffer */
		std::size_t getIndexCount() const { return mIndexCount; };

		/** Binds te Index Buffer Object */
		void bind() const;

		/** Unbinds the Index Buffer Object */
		void unbind() const;
	private:
		/** Creates the actual buffer
		 *
		 * @param	data a pointer to the data of the new buffer
		 * @param	size the size of the data buffer */
		void createBuffer(const void* data, std::size_t size);
	};


	template <typename T>
	IndexBuffer::IndexBuffer(const T* data, TypeId type, std::size_t count) :
		mIndexType(type), mIndexCount(count)
	{
		createBuffer(data, count * sizeof(T));
	}

}

#endif		// INDEX_BUFFER_H
