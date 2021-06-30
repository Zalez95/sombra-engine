#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <bitset>

namespace se::graphics {

	/** The different data types that can be stored in a buffer/texture */
 	enum class TypeId
	{
		Byte,	UnsignedByte,
		Short,	UnsignedShort,
		Int,	UnsignedInt,
		Float,	HalfFloat,
		Double
	};


	/** Returns the size in bytes of the given type
	 *
	 * @param	type the type to check
	 * @return	the size in bytes */
	constexpr std::size_t toTypeSize(TypeId type)
	{
		switch (type) {
			case TypeId::Byte:
				return sizeof(char);
			case TypeId::UnsignedByte:
				return sizeof(unsigned char);
			case TypeId::Short:
				return sizeof(short);
			case TypeId::UnsignedShort:
				return sizeof(unsigned short);
			case TypeId::Int:
				return sizeof(int);
			case TypeId::UnsignedInt:
				return sizeof(unsigned int);
			case TypeId::Float:
				return sizeof(float);
			case TypeId::HalfFloat:
				return sizeof(unsigned short);
			default:
				return sizeof(double);
		}
	}


	/** The different types of shaders */
	enum class ShaderType
	{
		Vertex,
		Geometry,
		Fragment
	};


	/** The different primitives (sets of vertices) that can be drawn */
	enum class PrimitiveType
	{
		Point,
		Line,
		LineLoop,
		LineStrip,
		Triangle,
		TriangleFan,
		TriangleStrip
	};


	/** The different targets that a Texture can be bound to */
	enum class TextureTarget
	{
		Texture1D,
		Texture2D,
		Texture3D,
		CubeMap
	};


	/** The different operations that can be enabled/disabled */
 	enum class Operation
	{
		Culling,
		DepthTest,
		ScissorTest,
		Blending
	};


	/** The Color format for the Textures */
	enum class ColorFormat
	{
		R,
		RG,
		RGB,
		RGBA,
		Depth, Depth16, Depth24, Depth32,
		DepthStencil,
		RInteger,
		RGInteger,
		RGBInteger,
		RGBAInteger,
		R8, R16ui, R16f, R32ui, R32f,
		RG8, RG16ui, RG16f, RG32ui, RG32f,
		RGB8, RGB16ui, RGB16f, RGB32ui, RGB32f,
		RGBA8, RGBA16ui, RGBA16f, RGBA32ui, RGBA32f
	};


	/** Converts the given ColorFormat to its unsized counterpart
	 *
	 * @param	format the ColorFormat to convert
	 * @return	the unsized version of the ColorFormat */
	constexpr ColorFormat toUnSizedColorFormat(ColorFormat format)
	{
		switch (format) {
			case ColorFormat::Depth:
			case ColorFormat::Depth16:
			case ColorFormat::Depth24:
			case ColorFormat::Depth32:
				return ColorFormat::Depth;
			case ColorFormat::DepthStencil:
				return ColorFormat::DepthStencil;
			case ColorFormat::R:
			case ColorFormat::RInteger:
			case ColorFormat::R8:
			case ColorFormat::R16ui:
			case ColorFormat::R16f:
			case ColorFormat::R32ui:
			case ColorFormat::R32f:
				return ColorFormat::R;
			case ColorFormat::RG:
			case ColorFormat::RGInteger:
			case ColorFormat::RG8:
			case ColorFormat::RG16ui:
			case ColorFormat::RG16f:
			case ColorFormat::RG32ui:
			case ColorFormat::RG32f:
				return ColorFormat::RG;
			case ColorFormat::RGB:
			case ColorFormat::RGBInteger:
			case ColorFormat::RGB8:
			case ColorFormat::RGB16ui:
			case ColorFormat::RGB16f:
			case ColorFormat::RGB32ui:
			case ColorFormat::RGB32f:
				return ColorFormat::RGB;
			default:
				return ColorFormat::RGBA;
		}
	}


	/** Returns the number of components of the given ColorFormat
	 *
	 * @param	format the ColorFormat to check
	 * @return	the number of components */
	constexpr std::size_t toNumberOfComponents(ColorFormat format)
	{
		switch (toUnSizedColorFormat(format)) {
			case ColorFormat::Depth:
				return 1;
			case ColorFormat::DepthStencil:
				return 2;
			case ColorFormat::R:
				return 1;
			case ColorFormat::RG:
				return 2;
			case ColorFormat::RGB:
				return 3;
			default:
				return 4;
		}
	}


	/** Defines the interpolation method used for mipmapping if enabled */
	enum class TextureFilter
	{
		Nearest,
		Linear,
		NearestMipMapNearest,
		LinearMipMapNearest,
		NearestMipMapLinear,
		LinearMipMapLinear
	};


	/** Defines the behavior of the texture when a requested position falls
	 * out of the [0.0, 1.0] range.
	 * Repeat		the texture repeats if it's out of the range
	 * ClampToEdge	the texture finishes at the last pixel of the texture */
	enum class TextureWrap
	{
		Repeat,
		MirroredRepeat,
		ClampToEdge,
		ClampToBorder
	};


	/** Indicates how the renderer should interpret the alpha color value */
	enum class AlphaMode
	{
		Opaque,
		Mask,
		Blend
	};


	/** Used for indicate face properties */
	enum class FaceMode
	{
		Front,
		Back,
		FrontAndBack
	};


	/** The different targets that a FrameBuffer can be bound to */
	enum class FrameBufferTarget
	{
		Read,
		Write,
		Both
	};


	/** The different FrameBuffer buffers to operate with */
	namespace FrameBufferMask
	{
		static constexpr std::size_t kStencil = 0;
		static constexpr std::size_t kDepth = 1;
		static constexpr std::size_t kColor = 2;
		static constexpr std::size_t kNumMasks = 3;

		using Mask = std::bitset<static_cast<int>(kNumMasks)>;
	}


	/** The different types of attachments of a texture to a FrameBuffer */
	namespace FrameBufferAttachment
	{
		static constexpr unsigned int kStencil = 0;
		static constexpr unsigned int kDepth = 1;
		static constexpr unsigned int kColor0 = 2;
		static constexpr unsigned int kMaxColorAttachments = 8;
	}

}

#endif		// CONSTANTS_H
