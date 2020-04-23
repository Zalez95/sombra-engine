#ifndef GRAPHICS_ENGINE_H
#define GRAPHICS_ENGINE_H

#include <vector>
#include <memory>
#include "ILayer.h"
#include "core/Texture.h"
#include "2D/Font.h"
#include "3D/Material.h"

namespace se::graphics {

	/** Struct GraphicsData, holds all the Configuration parameters of the
	 * GraphicsEngine */
	struct GraphicsData
	{
		/** The size of the layers viewports */
		glm::uvec2 viewportSize = glm::uvec2(0);

		/** The maximum number of Textures */
		std::size_t maxTextures = 0;

		/** The maximum number of Fonts */
		std::size_t maxFonts = 0;

		/** The maximum number of Materials */
		std::size_t maxMaterials = 0;

		/** The maximum number of SplatmapMaterials */
		std::size_t maxSplatmapMaterials = 0;
	};


	/**
	 * Class GraphicsEngine. It prepares all the OpenGL data and holds all the
	 * Layers to render
	 */
	class GraphicsEngine
	{
	private:	// Attributes
		/** The Layers that the GraphicsEngine will render */
		std::vector<ILayer*> mLayers;

		/** The size of the viewport */
		glm::uvec2 mViewportSize;

		/** The Texture repository of the GraphicsEngine */
		std::unique_ptr<Texture::Repository> mTextureRepo;

		/** The Font repository of the GraphicsEngine */
		std::unique_ptr<Font::Repository> mFontRepo;

		/** The Material repository of the GraphicsEngine */
		std::unique_ptr<Material::Repository> mMaterialRepo;

		/** The Splatmap Material repository of the GraphicsEngine */
		std::unique_ptr<SplatmapMaterial::Repository> mSplatmapMaterialRepo;

	public:		// Functions
		/** Creates a new Graphics System
		 *
		 * @param	config the configuration parameters of the GraphicsEngine
		 * @throw	runtime_error if failed to initialize GLEW */
		GraphicsEngine(const GraphicsData& config);

		/** @return	the Graphics API version info and other limits */
		std::string getGLInfo() const;

		/** Sets the viewport resolution
		 *
		 * @param	viewportSize the new size of the viewport */
		void setViewportSize(const glm::uvec2& viewportSize);

		/** @return	the viewport resolution */
		const glm::uvec2& getViewportSize() { return mViewportSize; };

		/** @return	the Texture Repository of the GraphicsEngine */
		Texture::Repository& getTextureRepository()
		{ return *mTextureRepo; };

		/** @return	the Font Repository of the GraphicsEngine */
		Font::Repository& getFontRepository()
		{ return *mFontRepo; };

		/** @return	the Material Repository of the GraphicsEngine */
		Material::Repository& getMaterialRepository()
		{ return *mMaterialRepo; };

		/** @return	the SplatmapMaterial Repository of the GraphicsEngine */
		SplatmapMaterial::Repository& getSplatmapMaterialRepository()
		{ return *mSplatmapMaterialRepo; };

		/** Adds the given ILayer to the GraphicsEngine so it will
		 * be rendered in each render call
		 *
		 * @param	layer a pointer to the ILayer that we want
		 *			to add to the GraphicsEngine */
		void addLayer(ILayer* layer);

		/** Removes the given ILayer from the GraphicsEngine
		 *
		 * @param	layer a pointer to the ILayer that we want to remove from
		 *			the GraphicsEngine
		 * @note	you must call this function before deleting the
		 *			Layer */
		void removeLayer(ILayer* layer);

		/** Draws the scene */
		void render();
	};

}

#endif		// GRAPHICS_ENGINE_H
