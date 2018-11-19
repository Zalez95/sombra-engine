#ifndef LAYER_3D_H
#define LAYER_3D_H

#include <vector>
#include <glm/glm.hpp>
#include "../ILayer.h"
#include "Renderer3D.h"

namespace se::graphics {

	class Renderable3D;
	class Camera;
	class PointLight;


	/**
	 * Class Layer3D, it's a class that holds all the 3D elements that the
	 * Renderer3D must render
	 */
	class Layer3D : public ILayer
	{
	private:	// Attributes
		static constexpr unsigned int kWidth	= 640;
		static constexpr unsigned int kHeight	= 480;
		static constexpr float kFOV				= glm::radians(60.0f);
		static constexpr float kZNear			= 1.0f;
		static constexpr float kZFar			= 250.0f;

		/** The Renderer used by the layer to render the Scene */
		Renderer3D mRenderer3D;

		/** The Camera used by the Layer */
		Camera* mCamera;

		/** The 3D renderables that the layer must render */
		std::vector<const Renderable3D*> mRenderable3Ds;

		/** The lights that illuminates the scene */
		std::vector<const PointLight*> mPointLights;

	public:		// Functions
		/** Creates a new Layer 3D */
		Layer3D();

		/** Class destructor */
		~Layer3D() {};

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

		/** Adds the given PointLight to the Layer so it will be rendered
		 *
		 * @param	pointLight PointLight a pointer to the PointLight to
		 *			add */
		void addPointLight(const PointLight* pointLight);

		/** Removes the given PointLight from the Layer so it will no longer
		 * be rendered
		 *
		 * @param	pointLight pointLight a pointer to the PointLight to
		 *			remove */
		void removePointLight(const PointLight* pointLight);

		/** Sets the given camera as the one used to render the Scene */
		void setCamera(Camera* camera) { mCamera = camera; };

		/** Draws the scene */
		void render() override;
	};

}

#endif		// LAYER_3D_H
