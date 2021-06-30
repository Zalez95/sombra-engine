#include <GL/glew.h>
#include "se/utils/Log.h"
#include "se/graphics/core/Constants.h"

#define GL_WRAP(x)					\
	se::graphics::glClearError();	\
	x;								\
	se::graphics::glLogError(#x, __FUNCTION__, __LINE__);


namespace se::graphics {

	inline void glClearError()
	{
		while (glGetError() != GL_NO_ERROR);
	}


	inline void glLogError(const char* glFunction, const char* function, int line)
	{
		GLenum error;
		while ((error = glGetError()) != GL_NO_ERROR) {
			std::string errorTag = "";
			switch (error) {
				case GL_INVALID_ENUM:					errorTag = "INVALID_ENUM";					break;
				case GL_INVALID_VALUE:					errorTag = "INVALID_VALUE";					break;
				case GL_INVALID_OPERATION:				errorTag = "INVALID_OPERATION";				break;
				case GL_STACK_OVERFLOW:					errorTag = "STACK_OVERFLOW";				break;
				case GL_STACK_UNDERFLOW:				errorTag = "STACK_UNDERFLOW";				break;
				case GL_OUT_OF_MEMORY:					errorTag = "OUT_OF_MEMORY";					break;
				case GL_INVALID_FRAMEBUFFER_OPERATION:	errorTag = "INVALID_FRAMEBUFFER_OPERATION";	break;
			}

			const char* errorString = reinterpret_cast<const char*>( glGetString(error) );

			utils::Log::getInstance()(utils::LogLevel::Error) << FORMAT_LOCATION(function, line)
				<< "OpenGL function \"" << glFunction << "\" returned error code " << error
				<< " (" << errorTag << "): \"" << (errorString? errorString : "") << "\"";
		}
	}


	constexpr GLenum toGLType(TypeId type)
	{
		switch (type) {
			case TypeId::Byte:			return GL_BYTE;
			case TypeId::UnsignedByte:	return GL_UNSIGNED_BYTE;
			case TypeId::Short:			return GL_SHORT;
			case TypeId::UnsignedShort:	return GL_UNSIGNED_SHORT;
			case TypeId::Int:			return GL_INT;
			case TypeId::UnsignedInt:	return GL_UNSIGNED_INT;
			case TypeId::Float:			return GL_FLOAT;
			case TypeId::HalfFloat:		return GL_HALF_FLOAT;
			case TypeId::Double:		return GL_DOUBLE;
			default:					return GL_NONE;
		}
	}


	constexpr TypeId fromGLType(GLenum type)
	{
		switch (type) {
			case GL_BYTE:			return TypeId::Byte;
			case GL_UNSIGNED_BYTE:	return TypeId::UnsignedByte;
			case GL_SHORT:			return TypeId::Short;
			case GL_UNSIGNED_SHORT:	return TypeId::UnsignedShort;
			case GL_INT:			return TypeId::Int;
			case GL_UNSIGNED_INT:	return TypeId::UnsignedInt;
			case GL_FLOAT:			return TypeId::Float;
			case GL_HALF_FLOAT:		return TypeId::HalfFloat;
			case GL_DOUBLE:			return TypeId::Double;
			default:				return TypeId::Byte;
		}
	}


	constexpr GLenum toGLPrimitive(PrimitiveType primitive)
	{
		switch (primitive) {
			case PrimitiveType::Point:			return GL_POINT;
			case PrimitiveType::Line:			return GL_LINES;
			case PrimitiveType::LineLoop:		return GL_LINE_LOOP;
			case PrimitiveType::LineStrip:		return GL_LINE_STRIP;
			case PrimitiveType::Triangle:		return GL_TRIANGLES;
			case PrimitiveType::TriangleFan:	return GL_TRIANGLE_FAN;
			case PrimitiveType::TriangleStrip:	return GL_TRIANGLE_STRIP;
			default:							return GL_NONE;
		}
	}


	constexpr GLenum toGLShader(ShaderType type)
	{
		switch (type) {
			case ShaderType::Vertex:	return GL_VERTEX_SHADER;
			case ShaderType::Geometry:	return GL_GEOMETRY_SHADER;
			case ShaderType::Fragment:	return GL_FRAGMENT_SHADER;
			default:					return GL_NONE;
		}
	}


	constexpr GLenum toGLFaceMode(FaceMode mode)
	{
		switch (mode) {
			case FaceMode::Front:			return GL_FRONT;
			case FaceMode::Back:			return GL_BACK;
			case FaceMode::FrontAndBack:	return GL_FRONT_AND_BACK;
			default:						return GL_NONE;
		}
	}


	constexpr GLenum toGLFrameBufferTarget(FrameBufferTarget target)
	{
		switch (target) {
			case FrameBufferTarget::Read:	return GL_READ_FRAMEBUFFER;
			case FrameBufferTarget::Write:	return GL_DRAW_FRAMEBUFFER;
			case FrameBufferTarget::Both:	return GL_FRAMEBUFFER;
			default:						return GL_NONE;
		}
	}


	constexpr GLbitfield toGLFrameBufferMask(const FrameBufferMask::Mask& mask)
	{
		GLbitfield ret = 0;
		ret |= mask[FrameBufferMask::kStencil]? GL_STENCIL_BUFFER_BIT : 0;
		ret |= mask[FrameBufferMask::kDepth]? GL_DEPTH_BUFFER_BIT : 0;
		ret |= mask[FrameBufferMask::kColor]? GL_COLOR_BUFFER_BIT : 0;
		return ret;
	}


	constexpr GLenum toGLFrameBufferAttachment(unsigned int attachment)
	{
		switch (attachment) {
			case FrameBufferAttachment::kStencil:	return GL_STENCIL_ATTACHMENT;
			case FrameBufferAttachment::kDepth:		return GL_DEPTH_ATTACHMENT;
			default:
				unsigned int colorIndex = attachment - FrameBufferAttachment::kColor0;
				return GL_COLOR_ATTACHMENT0 + colorIndex;
		}
	}


	constexpr int toGLTextureTarget(TextureTarget target)
	{
		switch (target) {
			case TextureTarget::Texture1D:	return GL_TEXTURE_1D;
			case TextureTarget::Texture2D:	return GL_TEXTURE_2D;
			case TextureTarget::Texture3D:	return GL_TEXTURE_3D;
			case TextureTarget::CubeMap:	return GL_TEXTURE_CUBE_MAP;
			default:						return GL_NONE;
		}
	}


	constexpr GLenum toGLOperation(Operation operation)
	{
		switch (operation) {
			case Operation::Culling:		return GL_CULL_FACE;
			case Operation::DepthTest:		return GL_DEPTH_TEST;
			case Operation::ScissorTest:	return GL_SCISSOR_TEST;
			case Operation::Blending:		return GL_BLEND;
			default:						return GL_NONE;
		}
	}


	constexpr GLenum toGLColorFormat(ColorFormat format)
	{
		switch (format) {
			case ColorFormat::R:			return GL_RED;
			case ColorFormat::RG:			return GL_RG;
			case ColorFormat::RGB:			return GL_RGB;
			case ColorFormat::RGBA:			return GL_RGBA;
			case ColorFormat::Depth:		return GL_DEPTH_COMPONENT;
			case ColorFormat::Depth16:		return GL_DEPTH_COMPONENT16;
			case ColorFormat::Depth24:		return GL_DEPTH_COMPONENT24;
			case ColorFormat::Depth32:		return GL_DEPTH_COMPONENT32F;
			case ColorFormat::DepthStencil:	return GL_DEPTH_STENCIL;
			case ColorFormat::RInteger:		return GL_RED_INTEGER;
			case ColorFormat::RGInteger:	return GL_RG_INTEGER;
			case ColorFormat::RGBInteger:	return GL_RGB_INTEGER;
			case ColorFormat::RGBAInteger:	return GL_RGBA_INTEGER;
			case ColorFormat::R8:			return GL_R8;
			case ColorFormat::R16ui:		return GL_R16UI;
			case ColorFormat::R16f:			return GL_R16F;
			case ColorFormat::R32ui:		return GL_R32UI;
			case ColorFormat::R32f:			return GL_R32F;
			case ColorFormat::RG8:			return GL_RG8;
			case ColorFormat::RG16ui:		return GL_RG16UI;
			case ColorFormat::RG16f:		return GL_RG16F;
			case ColorFormat::RG32ui:		return GL_RG32UI;
			case ColorFormat::RG32f:		return GL_RG32F;
			case ColorFormat::RGB8:			return GL_RGB8;
			case ColorFormat::RGB16ui:		return GL_RGB16UI;
			case ColorFormat::RGB16f:		return GL_RGB16F;
			case ColorFormat::RGB32ui:		return GL_RGB32UI;
			case ColorFormat::RGB32f:		return GL_RGB32F;
			case ColorFormat::RGBA8:		return GL_RGBA8;
			case ColorFormat::RGBA16ui:		return GL_RGBA16UI;
			case ColorFormat::RGBA16f:		return GL_RGBA16F;
			case ColorFormat::RGBA32ui:		return GL_RGBA32UI;
			case ColorFormat::RGBA32f:		return GL_RGBA32F;
			default:						return GL_NONE;
		}
	}


	constexpr ColorFormat fromGLColorFormat(GLenum format)
	{
		switch (format) {
			case GL_RG:					return ColorFormat::RG;
			case GL_RGB:				return ColorFormat::RGB;
			case GL_RGBA:				return ColorFormat::RGBA;
			case GL_DEPTH_COMPONENT:	return ColorFormat::Depth;
			case GL_DEPTH_COMPONENT16:	return ColorFormat::Depth16;
			case GL_DEPTH_COMPONENT24:	return ColorFormat::Depth24;
			case GL_DEPTH_COMPONENT32F:	return ColorFormat::Depth32;
			case GL_DEPTH_STENCIL:		return ColorFormat::DepthStencil;
			case GL_RED_INTEGER:		return ColorFormat::RInteger;
			case GL_RG_INTEGER:			return ColorFormat::RGInteger;
			case GL_RGB_INTEGER:		return ColorFormat::RGBInteger;
			case GL_RGBA_INTEGER:		return ColorFormat::RGBAInteger;
			case GL_R8:					return ColorFormat::R8;
			case GL_R16UI:				return ColorFormat::R16ui;
			case GL_R16F:				return ColorFormat::R16f;
			case GL_R32UI:				return ColorFormat::R32ui;
			case GL_R32F:				return ColorFormat::R32f;
			case GL_RG8:				return ColorFormat::RG8;
			case GL_RG16UI:				return ColorFormat::RG16ui;
			case GL_RG16F:				return ColorFormat::RG16f;
			case GL_RG32UI:				return ColorFormat::RG32ui;
			case GL_RG32F:				return ColorFormat::RG32f;
			case GL_RGB8:				return ColorFormat::RGB8;
			case GL_RGB16UI:			return ColorFormat::RGB16ui;
			case GL_RGB16F:				return ColorFormat::RGB16f;
			case GL_RGB32UI:			return ColorFormat::RGB32ui;
			case GL_RGB32F:				return ColorFormat::RGB32f;
			case GL_RGBA8:				return ColorFormat::RGBA8;
			case GL_RGBA16UI:			return ColorFormat::RGBA16ui;
			case GL_RGBA16F:			return ColorFormat::RGBA16f;
			case GL_RGBA32UI:			return ColorFormat::RGBA32ui;
			case GL_RGBA32F:			return ColorFormat::RGBA32f;
			default:					return ColorFormat::R;
		}
	}


	constexpr int toGLFilter(TextureFilter filter)
	{
		switch (filter) {
			case TextureFilter::Nearest:				return GL_NEAREST;
			case TextureFilter::Linear:					return GL_LINEAR;
			case TextureFilter::NearestMipMapNearest:	return GL_NEAREST_MIPMAP_NEAREST;
			case TextureFilter::LinearMipMapNearest:	return GL_LINEAR_MIPMAP_NEAREST;
			case TextureFilter::NearestMipMapLinear:	return GL_NEAREST_MIPMAP_LINEAR;
			case TextureFilter::LinearMipMapLinear:		return GL_LINEAR_MIPMAP_LINEAR;
			default:									return GL_NONE;
		}
	}


	constexpr TextureFilter fromGLFilter(int filter)
	{
		switch (filter) {
			case GL_NEAREST:				return TextureFilter::Nearest;
			case GL_LINEAR:					return TextureFilter::Linear;
			case GL_NEAREST_MIPMAP_NEAREST:	return TextureFilter::NearestMipMapNearest;
			case GL_LINEAR_MIPMAP_NEAREST:	return TextureFilter::LinearMipMapNearest;
			case GL_NEAREST_MIPMAP_LINEAR:	return TextureFilter::NearestMipMapLinear;
			default:						return TextureFilter::LinearMipMapLinear;
		}
	}


	constexpr int toGLWrap(TextureWrap wrap)
	{
		switch (wrap) {
			case TextureWrap::Repeat:			return GL_REPEAT;
			case TextureWrap::MirroredRepeat:	return GL_MIRRORED_REPEAT;
			case TextureWrap::ClampToEdge:		return GL_CLAMP_TO_EDGE;
			case TextureWrap::ClampToBorder:	return GL_CLAMP_TO_BORDER;
			default:							return GL_NONE;
		}
	}


	constexpr TextureWrap fromGLWrap(int wrap)
	{
		switch (wrap) {
			case GL_REPEAT:				return TextureWrap::Repeat;
			case GL_MIRRORED_REPEAT:	return TextureWrap::MirroredRepeat;
			case GL_CLAMP_TO_EDGE:		return TextureWrap::ClampToEdge;
			default:					return TextureWrap::ClampToBorder;
		}
	}

}
