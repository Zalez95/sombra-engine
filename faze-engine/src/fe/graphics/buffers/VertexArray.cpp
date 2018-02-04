#include "fe/graphics/GLWrapper.h"
#include "fe/graphics/buffers/VertexArray.h"
#include "fe/graphics/buffers/VertexBuffer.h"

namespace fe { namespace graphics {

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
		glBindVertexArray(mArrayID);
	}


	void VertexArray::unbind() const
	{
		glBindVertexArray(0);
	}

}}
