#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cstddef>

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


	/** The Color format for the Textures */
	enum class ColorFormat
	{
		Red,
		RG,
		RGB,
		RGBA,
		Depth, Depth16, Depth24, Depth32,
		DepthStencil,
		RedInteger,
		RGInteger,
		RGBInteger,
		RGBAInteger,
		Red32ui,
		RG32ui,
		RGB8, RGB16ui, RGB16f, RGB32ui, RGB32f,
		RGBA8, RGBA16ui, RGBA16f, RGBA32ui, RGBA32f
	};


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


	/** The different targets that a FrameBuffer can be bound to */
	enum class FrameBufferTarget
	{
		Read,
		Write,
		Both
	};


	/** The different types of attachments of a texture to a FrameBuffer */
	enum class FrameBufferAttachment
	{
		Stencil,
		Depth,
		Color
	};

}

#endif		// CONSTANTS_H
