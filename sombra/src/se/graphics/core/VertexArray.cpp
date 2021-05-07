#include "se/graphics/core/VertexArray.h"
#include "se/graphics/core/VertexBuffer.h"
#include "GLWrapper.h"

namespace se::graphics {

	VertexArray::VertexArray()
	{
		GL_WRAP( glGenVertexArrays(1, &mArrayId) );
		SOMBRA_TRACE_LOG << "Created VAO " << mArrayId;
	}


	VertexArray::VertexArray(VertexArray&& other) : mArrayId(other.mArrayId)
	{
		other.mArrayId = 0;
	}


	VertexArray::~VertexArray()
	{
		if (mArrayId != 0) {
			GL_WRAP( glDeleteVertexArrays(1, &mArrayId) );
			SOMBRA_TRACE_LOG << "Deleted VAO " << mArrayId;
		}
	}


	VertexArray& VertexArray::operator=(VertexArray&& other)
	{
		if (mArrayId != 0) {
			GL_WRAP( glDeleteVertexArrays(1, &mArrayId) );
			SOMBRA_TRACE_LOG << "Deleted VAO " << mArrayId;
		}

		mArrayId = other.mArrayId;
		other.mArrayId = 0;

		return *this;
	}


	unsigned int VertexArray::getMaxAttributes()
	{
		int maxAttributes = 0;
		GL_WRAP( glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxAttributes) );
		return maxAttributes;
	}


	void VertexArray::enableAttribute(unsigned int index)
	{
		bind();
		GL_WRAP( glEnableVertexAttribArray(index) );
	}


	bool VertexArray::isAttributeEnabled(unsigned int index) const
	{
		int enabled = 0;
		bind();
		GL_WRAP( glGetVertexAttribIiv(index, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled) );
		return enabled != 0;
	}


	void VertexArray::setVertexAttribute(
		unsigned int index,
		TypeId type, bool normalized, int componentSize, std::size_t stride, std::size_t offset
	) {
		bind();
		GL_WRAP( glVertexAttribPointer(
			index, componentSize, toGLType(type), normalized,
			static_cast<GLsizei>(stride), reinterpret_cast<const void*>(offset)
		) );
	}


	void VertexArray::setVertexDoubleAttribute(
		unsigned int index,
		TypeId type, int componentSize, std::size_t stride, std::size_t offset
	) {
		bind();
		GL_WRAP( glVertexAttribLPointer(
			index, componentSize, toGLType(type),
			static_cast<GLsizei>(stride), reinterpret_cast<const void*>(offset)
		) );
	}


	void VertexArray::setVertexIntegerAttribute(
		unsigned int index,
		TypeId type, int componentSize, std::size_t stride, std::size_t offset
	) {
		bind();
		GL_WRAP( glVertexAttribIPointer(
			index, componentSize, toGLType(type),
			static_cast<GLsizei>(stride), reinterpret_cast<const void*>(offset)
		) );
	}


	bool VertexArray::isDoubleAttribute(unsigned int index) const
	{
		int res = 0;
		bind();
		GL_WRAP( glGetVertexAttribIiv(index, GL_VERTEX_ATTRIB_ARRAY_LONG, &res) );
		return res != 0;
	}


	bool VertexArray::isIntegerAttribute(unsigned int index) const
	{
		int res = 0;
		bind();
		GL_WRAP( glGetVertexAttribIiv(index, GL_VERTEX_ATTRIB_ARRAY_INTEGER, &res) );
		return res != 0;
	}


	void VertexArray::getVertexAttribute(
		unsigned int index, TypeId& type, bool& normalized,
		int& componentSize, std::size_t& stride, std::size_t& offset
	) const
	{
		int iType = 0, iNormalized = 0, iComponentSize = 0;
		unsigned int uStride = 0, uOffset = 0;
		bind();
		GL_WRAP( glGetVertexAttribIiv(index, GL_VERTEX_ATTRIB_ARRAY_TYPE, &iType) );
		GL_WRAP( glGetVertexAttribIiv(index, GL_VERTEX_ATTRIB_ARRAY_NORMALIZED, &iNormalized) );
		GL_WRAP( glGetVertexAttribIiv(index, GL_VERTEX_ATTRIB_ARRAY_SIZE, &iComponentSize) );
		GL_WRAP( glGetVertexAttribIuiv(index, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &uStride) );
		GL_WRAP( glGetVertexAttribIuiv(index, GL_VERTEX_ATTRIB_RELATIVE_OFFSET, &uOffset) );
		type = fromGLType(iType);
		normalized = iNormalized != 0;
		componentSize = iComponentSize;
		stride = uStride;
		offset = uOffset;
	}


	bool VertexArray::checkVertexAttributeVBOBound(unsigned int index, const VertexBuffer& vbo) const
	{
		unsigned int vertexBufferId;
		bind();
		GL_WRAP( glGetVertexAttribIuiv(index, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &vertexBufferId) );
		return vertexBufferId == vbo.getVertexBufferId();
	}


	void VertexArray::setAttributeDivisor(unsigned int index, unsigned int divisor)
	{
		bind();
		GL_WRAP( glVertexAttribDivisor(index, divisor) );
	}


	unsigned int VertexArray::getAttributeDivisor(unsigned int index) const
	{
		unsigned int divisor = 0;
		bind();
		GL_WRAP( glGetVertexAttribIuiv(index, GL_VERTEX_ATTRIB_ARRAY_DIVISOR, &divisor) );
		return divisor;
	}


	void VertexArray::disableAttribute(unsigned int index)
	{
		bind();
		GL_WRAP( glDisableVertexAttribArray(index) );
	}


	void VertexArray::bind() const
	{
		GL_WRAP( glBindVertexArray(mArrayId) );
	}


	void VertexArray::unbind() const
	{
		GL_WRAP( glBindVertexArray(0) );
	}

}
