#include "fe/graphics/buffers/VertexArray.h"
#include <GL/glew.h>
#include "fe/graphics/buffers/VertexBuffer.h"

namespace fe { namespace graphics {

	VertexArray::VertexArray()
	{
		glGenVertexArrays(1, &mArrayID);
	}


	VertexArray::~VertexArray()
	{
		glDeleteVertexArrays(1, &mArrayID);
	}


	void VertexArray::addBuffer(const VertexBuffer* vertexBuffer, unsigned int index)
	{
		bind();

		vertexBuffer->bind();
		glEnableVertexAttribArray(index);
		glVertexAttribPointer(index, vertexBuffer->getComponentSize(), GL_FLOAT, GL_FALSE, 0, 0);

		unbind();
	}


	void VertexArray::bind() const
	{
		glBindVertexArray(mArrayID);
	}


	void VertexArray::unbind() const
	{
		glBindVertexArray(0);
	}

}}
