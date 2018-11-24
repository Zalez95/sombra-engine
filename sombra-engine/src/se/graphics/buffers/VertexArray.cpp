#include "se/graphics/GLWrapper.h"
#include "se/graphics/buffers/VertexArray.h"
#include "se/graphics/buffers/VertexBuffer.h"

namespace se::graphics {

	VertexArray::VertexArray()
	{
		GL_WRAP( glGenVertexArrays(1, &mArrayID) );
	}


	VertexArray::~VertexArray()
	{
		GL_WRAP( glDeleteVertexArrays(1, &mArrayID) );
	}


	void VertexArray::addBuffer(const VertexBuffer* vertexBuffer, unsigned int index)
	{
		bind();

		vertexBuffer->bind();
		GL_WRAP( glEnableVertexAttribArray(index) );
		GL_WRAP( glVertexAttribPointer(index, vertexBuffer->getComponentSize(), GL_FLOAT, GL_FALSE, 0, 0) );

		unbind();
	}


	void VertexArray::bind() const
	{
		GL_WRAP( glBindVertexArray(mArrayID) );
	}


	void VertexArray::unbind() const
	{
		GL_WRAP( glBindVertexArray(0) );
	}

}
