#ifndef SHADOW_RENDERER_3D_H
#define SHADOW_RENDERER_3D_H

#include "FrustumRenderer3D.h"

namespace se::app {

	/**
	 * Class ShadowRenderer. It's a FrustumRenderer3D used for rendering shadows
	 * to a Shadow map. It has a "shadow" input and output where the shadow map
	 * can be appended and retrieved.
	 * The Shadow Map must be float depth texture attached to the "target" input
	 * and output FrameBuffer
	 */
	class ShadowRenderer3D : public FrustumRenderer3D
	{
	private:	// Attributes
		/** The index of the Shadow map texture bindable */
		std::size_t mIShadowTexBindable;

		/** The resolution of the Shadow maps */
		std::size_t mShadowResolution;

	public:		// Functions
		/** Creates a new ShadowRenderer3D
		 *
		 * @param	name the name of the RenderNode */
		ShadowRenderer3D(const std::string& name);

		/** Class destructor */
		virtual ~ShadowRenderer3D() = default;

		/** Sets the resolution of the Shadow maps
		 *
		 * @param	shadowResolution the new resolution of the Shadow maps
		 * @return	the current ShadowRenderer3D
		 * @note	the appended Shadow map will be changed */
		ShadowRenderer3D& setShadowResolution(std::size_t shadowResolution);

		/** @copydoc FrustumRenderer3D::render() */
		virtual void render() override;
	};

}

#endif		// SHADOW_RENDERER_3D_H
