#ifndef VERTEX_ARRAY_H
#define VERTEX_ARRAY_H

#include <vector>
#include "../Constants.h"

namespace se::graphics {

	class VertexBuffer;


	/**
	 * Class VertexArray, it's used for creating, binding and unbinding a
	 * Vertex Array Object
	 *
	 * A Vertex Array Object is an object that stores all the information
	 * (the attribute data) for a complete Mesh
	 */
	class VertexArray
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

		/** Sets the bound Vertex Buffer Object as an attribute of the Vertex
		 * Array Object
		 *
		 * @param	index the index of the attribute where we want to bind the
		 *			given VBO
		 * @param	type the TypeId of the data stored in the VBO
		 * @param	normalized specifies if the vertexBuffer values should be
		 *			normalized or not
		 * @param	componentSize the number of components per vertex attribute
		 * @param	stride the byte offset between each vertex attribute
		 * @note	the VAO must be bound before calling this function */
		void setVertexAttribute(
			unsigned int index,
			TypeId type, bool normalized, int componentSize, int stride
		);

		/** Binds the Vertex Array Object */
		void bind() const;

		/** Unbinds the Vertex Array Object */
		void unbind() const;
	};

}

#endif		// VERTEX_ARRAY_H
