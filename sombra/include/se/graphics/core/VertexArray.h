#ifndef VERTEX_ARRAY_H
#define VERTEX_ARRAY_H

#include "Constants.h"
#include "Bindable.h"

namespace se::graphics {

	class VertexBuffer;


	/**
	 * Class VertexArray, it's used for creating, binding and unbinding a
	 * Vertex Array Object
	 *
	 * A Vertex Array Object is an object that stores all the information
	 * (the attribute data) for a complete Mesh
	 */
	class VertexArray : public Bindable
	{
	private:	// Attributes
		/** The id of the Vertex Array Object */
		unsigned int mArrayId;

	public:		// Functions
		/** Creates a new VertexArray */
		VertexArray();
		VertexArray(const VertexArray& other) = delete;
		VertexArray(VertexArray&& other);

		/** Class destructor */
		~VertexArray();

		/** Assignment operator */
		VertexArray& operator=(const VertexArray& other) = delete;
		VertexArray& operator=(VertexArray&& other);

		/** @return	the maximum number of Vertex Array Object attributes */
		static unsigned int getMaxAttributes();

		/** Enables a Vertex Array Object attribute
		 *
		 * @param	index the index of the VertexArray attribute to enable */
		void enableAttribute(unsigned int index);

		/** Returns if the given vertex attribute is enabled or not
		 *
		 * @param	index the index of the attribute to check
		 * @return	true if it's enabled, false otherwise */
		bool isAttributeEnabled(unsigned int index) const;

		/** Sets the bound Vertex Buffer Object as an attribute of the Vertex
		 * Array Object
		 *
		 * @param	index the index of the attribute where we want to bind the
		 *			given VBO
		 * @param	type the TypeId of the data stored in the VBO
		 * @param	normalized specifies if the vertexBuffer values should be
		 *			normalized or not
		 * @param	componentSize the number of components per vertex attribute
		 * @param	stride the byte offset between each vertex attribute, zero
		 *			means that the data is tightly packed
		 * @param	offset the byte offset from the start of the buffer that
		 *			contains the vertex data where attribute starts
		 * @note	the VAO must be bound before calling this function */
		void setVertexAttribute(
			unsigned int index, TypeId type, bool normalized, int componentSize,
			std::size_t stride, std::size_t offset = 0
		);

		/** Behaves like @see setVertexAttribute with the difference that the
		 * shader attribute will be treated as doubles */
		void setVertexDoubleAttribute(
			unsigned int index, TypeId type, int componentSize,
			std::size_t stride, std::size_t offset = 0
		);

		/** Behaves like @see setVertexAttribute with the difference that the
		 * shader attribute will be treated as integers */
		void setVertexIntegerAttribute(
			unsigned int index, TypeId type, int componentSize,
			std::size_t stride, std::size_t offset = 0
		);

		/** @return	true if the given vertex attribute has a double data
		 *			type, false otherwise */
		bool isDoubleAttribute(unsigned int index) const;

		/** @return	true if the given vertex attribute has an integer data
		 *			type, false otherwise */
		bool isIntegerAttribute(unsigned int index) const;

		/** Returns the attribute configuration
		 *
		 * @param	index the index of the attribute to check
		 * @param	type the TypeId of the data stored in the VBO
		 *			(return parameter)
		 * @param	normalized specifies if the vertexBuffer has been
		 *			normalized or not (return parameter)
		 * @param	componentSize the number of components per vertex attribute
		 *			(return parameter)
		 * @param	stride the byte offset between each vertex attribute
		 *			(return parameter)
		 * @param	offset the byte offset from the start of the buffer that
		 *			contains the vertex data where attribute starts
		 *			(return parameter) */
		void getVertexAttribute(
			unsigned int index, TypeId& type, bool& normalized,
			int& componentSize, std::size_t& stride, std::size_t& offset
		) const;

		/** Returns if the given VertexBuffer is bound to the given attribute
		 *
		 * @param	index the index of the attribute to check
		 * @param	vbo the Vertex Buffer to check
		 * @return	true if vbo is bound to the index, false otherwise */
		bool checkVertexAttributeVBOBound(
			unsigned int index, const VertexBuffer& vbo
		) const;

		/** Copies the vertex attribute config from the given VertexArray to
		 * the current one
		 *
		 * @param	index the index of the attribute to copy
		 * @param	other the other VertexArray to copy from */
		void copyVertexAttribute(unsigned int index, const VertexArray& other);

		/** Sets rate that a vertex attribute advance each time a instance is
		 * drawn
		 *
		 * @param	index the index of the attribute to set the divisor
		 * @param	divisor the rate that the attribute advances for each
		 *			instance */
		void setAttributeDivisor(unsigned int index, unsigned int divisor);

		/** Returns rate that a vertex attribute advance each time a instance is
		 * drawn
		 *
		 * @param	index the index of the attribute to check
		 * @return	the rate that the attribute advances for each instance */
		unsigned int getAttributeDivisor(unsigned int index) const;

		/** Disables a Vertex Array Object attribute
		 *
		 * @param	index the index of the VertexArray attribute to disable */
		void disableAttribute(unsigned int index);

		/** @copydoc Bindable::clone()
		 * @note	the IndexBuffers and VertexBuffers won't be bound to the
		 *			new VAO */
		virtual std::unique_ptr<Bindable> clone() const override
		{ return std::make_unique<VertexArray>(); };

		/** Binds the Vertex Array Object */
		virtual void bind() const override;

		/** Unbinds the Vertex Array Object */
		virtual void unbind() const override;
	};

}

#endif		// VERTEX_ARRAY_H
