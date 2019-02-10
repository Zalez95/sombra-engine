#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace se::graphics {

	/**
	 * The different data types that can be stored in a buffer/texture
	 */
 	enum class TypeId : int
	{
		Byte,	UnsignedByte,
		Short,	UnsignedShort,
		Int,	UnsignedInt,
		Float,	HalfFloat,
		Double
	};


	/**
	 * The Color format for the Textures
	 */
	enum class ColorFormat
	{
		Red,
		Green,
		Blue,
		Alpha,
		RGB,
		RGBA
	};


	/**
	 * Defines the interpolation method used for mipmapping if enabled
	 */
	enum class TextureFilter
	{
		Nearest,
		Linear
	};


	/**
	 * Defines the behavior of the texture when a requested position falls
	 * out of the [0.0, 1.0] range.
	 * Repeat		the texture repeats if it's out of the range
	 * ClampToEdge	the texture finishes at the last pixel of the texture
	 */
	enum class TextureWrap
	{
		Repeat,
		ClampToEdge
	};

}

#endif		// CONSTANTS_H
