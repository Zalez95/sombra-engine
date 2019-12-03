#ifndef LAYER_3D_H
#define LAYER_3D_H

#include <vector>
#include "../ILayer.h"
#include "Renderer3D.h"

namespace se::graphics {

	class Renderable3D;
	class Camera;
	struct ILight;


	/**
	 * Class Layer3D, it's a class that holds all the 3D elements that the
	 * Renderer3D must render
	 */
	class Layer3D : public ILayer
	{
	private:	// Attributes
		/** The Renderer used by the layer to render the Scene */
		Renderer3D mRenderer3D;

		/** The 3D renderables that the layer must render */
		std::vector<const Renderable3D*> mRenderable3Ds;

		/** The lights that illuminates the scene */
		std::vector<const ILight*> mLights;

		/** The sky used by the Layer */
		const Renderable3D* mSky;

		/** The Camera used by the Layer */
		Camera* mCamera;

	public:		// Functions
		/** Creates a new Layer3D */
		Layer3D() : mSky(nullptr), mCamera(nullptr) {};

		/** Adds the given Renderable to the Layer so it will be rendered
		 *
		 * @param	renderable3D renderable3D a pointer to the Renderable3D to
		 *			add */
		void addRenderable3D(const Renderable3D* renderable3D);

		/** Removes the given Renderable from the Layer so it will no longer
		 * be rendered
		 *
		 * @param	renderable3D renderable3D a pointer to the Renderable3D to
		 *			remove */
		void removeRenderable3D(const Renderable3D* renderable3D);

		/** Adds the given ILight to the Layer so it will be rendered
		 *
		 * @param	light a pointer to the ILight to add */
		void addLight(const ILight* light);

		/** Removes the given ILight from the Layer so it will no longer be
		 * rendered
		 *
		 * @param	light a pointer to the ILight to remove */
		void removeLight(const ILight* light);

		/** Sets the given Renderable as the Layer's Sky
		 *
		 * @param	sky a pointer to the Renderable3D used as Sky */
		void setSky(const Renderable3D* sky) { mSky = sky; }

		/** Sets the given camera as the one used to render the Scene */
		void setCamera(Camera* camera) { mCamera = camera; };

		/** Draws the scene */
		void render() override;
	};

}

#endif		// LAYER_3D_H
