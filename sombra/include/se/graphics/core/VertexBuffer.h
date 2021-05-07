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

		/** @return the id of the VertexBuffer */
		inline unsigned int getVertexBufferId() const { return mBufferId; };

		/** @return	the size of the contents of the buffer in number of
		 *			elements */
		template <typename T>
		std::size_t size() const;

		/** @return	the size of the contents of the buffer in bytes */
		std::size_t size() const;

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

		/** Returns the buffer data
		 *
		 * @param	data the pointer where the buffer contents will be written
		 * @param	count the number of elements to read
		 * @param	offset the offset from the start of the buffer from which
		 *			the data will be returned */
		template <typename T>
		void read(T* data, std::size_t count, std::size_t offset = 0) const;

		/** Returns the buffer data
		 *
		 * @param	data the pointer where the buffer contents will be written
		 * @param	size the size in bytes of the data to read
		 * @param	offset the offset from the start of the buffer from which
		 *			the data will be returned */
		void read(void* data, std::size_t size, std::size_t offset = 0) const;

		/** Binds the Vertex Buffer Object */
		void bind() const override;

		/** Unbinds the Vertex Buffer Object */
		void unbind() const override;
	};


	template <typename T>
	std::size_t VertexBuffer::size() const
	{
		return size() / sizeof(T);
	}


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


	template <typename T>
	void VertexBuffer::read(
		T* data, std::size_t count, std::size_t offset
	) const
	{
		read(
			static_cast<void*>(data), count * sizeof(T),
			offset * sizeof(T)
		);
	}

}

#endif		// VERTEX_BUFFER_H
