#ifndef VERTEX_ARRAY_H
#define VERTEX_ARRAY_H

#include <vector>

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

		/** Adds the given Vertex Buffer Object to the Vertex Array Object
		 *
		 * @param	vertexBuffer the VBO that we want to add to the current VAO
		 * @param	index the index of the attribute where we want to bind the
		 *			given VBO */
		void addBuffer(const VertexBuffer& vertexBuffer, unsigned int index);

		/** Binds the Vertex Array Object */
		void bind() const;

		/** Unbinds the Vertex Array Object */
		void unbind() const;
	};

}

#endif		// VERTEX_ARRAY_H
