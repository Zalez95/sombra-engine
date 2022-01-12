#ifndef RENDERABLE_LIGHT_H
#define RENDERABLE_LIGHT_H

#include "../../utils/FixedVector.h"
#include "../../graphics/3D/RenderableMesh.h"
#include "TypeRefs.h"

namespace se::app {

	/**
	 * Class RenderableLight, its a Renderable3D used for submitting the light
	 * to a DeferredLightSubGraph, the real light volume will be rendererd
	 * using the RenderableMesh of RenderableLight. This means that the
	 * Techniques and Passes added to the RenderableLight won't be used for
	 * drawing the light, instead they will only be used for submitting it to
	 * the Renderer, the real Techniques used for drawing will be the ones
	 * added to its RenderableMesh.
	 */
	class RenderableLight : public graphics::Renderable3D
	{
	public:		// Attributes
		/** The maximum number of shadow maps per light */
		static constexpr std::size_t kMaxShadowMaps = 6;
	private:
		/** The RenderableMesh used for rendering the LightVolume */
		graphics::RenderableMesh mRenderableMesh;

		/** If the light is a PointLight or not */
		bool mIsPointLight = false;

		/** The resolution of the Shadow Maps */
		std::size_t mResolution;

		/** The Shadow Map texture. If @see mIsPointLight is true it will be
		 * a CubeMap, otherwise it will be a Texture 2D array */
		TextureRef mShadowMap;

		/** The view matrices used for rendering each shadow */
		utils::FixedVector<glm::mat4, kMaxShadowMaps> mViewMatrices;

		/** The projection matrices used for rendering each shadow */
		utils::FixedVector<glm::mat4, kMaxShadowMaps> mProjectionMatrices;

	public:		// Functions
		/** @copydoc graphics::Renderable3D::getBounds() */
		virtual std::pair<glm::vec3, glm::vec3> getBounds() const override
		{ return mRenderableMesh.getBounds(); };

		/** @return	the RenderableMesh of the RenderableLight */
		graphics::RenderableMesh& getRenderableMesh()
		{ return mRenderableMesh; };

		/** @return	true if the RenderableLight casts shadows, false
		 *			otherwise */
		bool castsShadows() const { return mShadowMap; };

		/** @return	the number of Shadow Maps of the RenderableLight */
		std::size_t getNumShadows() const { return mViewMatrices.size(); };

		/** @return	true if the RenderableLight is a PointLight, false
		 *			otherwise */
		bool isPointLight() const { return mIsPointLight; };

		/** @return	the resolution of the Shadow Maps */
		std::size_t getResolution() const { return mResolution; };

		/** Enables shadows if it hasnt' been enabled yet, or updates the
		 * shadow configuration
		 *
		 * @param	context the graphics Context used for creating the
		 *			ShadowMaps
		 * @param	resolution the Resolution of the Shadow Maps
		 * @param	isPointLight if the RenderableLight is a PointLight or
		 *			not
		 * @param	numShadows the number of shadow cascades if the
		 *			RenderableLight isn't a PointLight
		 * @return	a reference to the current RenderableLight object */
		RenderableLight& setShadows(
			graphics::Context& context, std::size_t resolution,
			bool isPointLight = false, std::size_t numShadows = 1
		);

		/** Disables shadow casting for the current RenderableLight
		 *
		 * @return	a reference to the current RenderableLight object */
		RenderableLight& disableShadows();

		/** Returns the shadow map of the Light
		 *
		 * @return	a pointer to the Shadow Map Texture, nullptr if it doesn't
		 *			exists */
		const TextureRef& getShadowMap() const { return mShadowMap; };

		/** Returns the requested view matrix
		 *
		 * @param	i the shadow index to retrieve its matrix
		 * @return	the view matrix */
		const glm::mat4& getShadowViewMatrix(std::size_t i) const
		{ return mViewMatrices[i]; };

		/** Sets the requested view matrix
		 *
		 * @param	i the shadow index to retrieve its matrix
		 * @param	viewMatrix the new view matrix */
		void setShadowViewMatrix(std::size_t i, const glm::mat4& viewMatrix)
		{ mViewMatrices[i] = viewMatrix; };

		/** Returns the requested projection matrix
		 *
		 * @param	i the shadow index to retrieve its matrix
		 * @return	the projection matrix */
		const glm::mat4& getShadowProjectionMatrix(std::size_t i) const
		{ return mProjectionMatrices[i]; };

		/** Sets the requested projection matrix
		 *
		 * @param	i the shadow index to retrieve its matrix
		 * @param	projectionMatrix the new projection matrix */
		void setShadowProjectionMatrix(
			std::size_t i, const glm::mat4& projectionMatrix
		) { mProjectionMatrices[i] = projectionMatrix; };
	};

}

#endif		// RENDERABLE_LIGHT_H
