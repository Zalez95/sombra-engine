#ifndef TEXTURE_UTILS_H
#define TEXTURE_UTILS_H

#include "Image.h"
#include "TypeRefs.h"

namespace se::app {

	/**
	 * Class TextureUtils, holds all the Texture transformation functions
	 */
	class TextureUtils
	{
	public:		// Functions
		/** Creates a new CubeMap Texture from the given equirectangular one
		 *
		 * @param	source the source Texture to convert
		 * @param	size the width and height of each of the CubeMap images
		 * @return	the new CubeMap Texture, it will be in the same Context
		 *			than the source one */
		static TextureRef equirectangularToCubeMap(
			const TextureRef& source, std::size_t size
		);

		/** Creates a new CubeMap Texture from the given one by convoluting all
		 * its values
		 *
		 * @param	source the source Texture to convert
		 * @param	size the width and height of each of the CubeMap images
		 * @return	the new CubeMap Texture, it will be in the same Context
		 *			than the source one */
		static TextureRef convoluteCubeMap(
			const TextureRef& source, std::size_t size
		);

		/** Creates a new CubeMap Texture from the given one by prefiltering it
		 * with different roughness values, and storing the resulting it the
		 * different mipmap levels of the new CubeMap texture
		 *
		 * @param	source the source Texture to convert
		 * @param	size the width and height of each of the CubeMap images
		 * @return	the new CubeMap Texture, it will be in the same Context
		 *			than the source one */
		static TextureRef prefilterCubeMap(
			const TextureRef& source, std::size_t size
		);

		/** Calculates the specular BRDF and stores it to a texture
		 *
		 * @param	context the graphics Context used for creating the Texture
		 * @param	size the width and height of the output texture
		 * @return	the new precomputed BRDF Texture */
		static TextureRef precomputeBRDF(
			graphics::Context& context, std::size_t size
		);

		/** Creates a normal map Texture from the given height map one with the
		 * normal vectors stored in local space
		 *
		 * @param	source the source Texture to convert
		 * @param	width the width of the source and output textures
		 * @param	height the height of the source and output textures
		 * @return	the new normal map Texture, it will be in the same Context
		 *			than the source one */
		static TextureRef heightmapToNormalMapLocal(
			const TextureRef& source, std::size_t width, std::size_t height
		);

		/** Generates an image from the given texture
		 *
		 * @param	source the source texture
		 * @param	type the type of the source texture values
		 * @param	color the color format of the source texture values
		 * @param	width the width of the new image
		 * @param	height the height of the new image
		 * @return	the new Image */
		template <typename T>
		static Image<T> textureToImage(
			const TextureRef& source,
			graphics::TypeId type, graphics::ColorFormat color,
			std::size_t width, std::size_t height
		);
	};

}

#endif		// TEXTURE_UTILS_H
