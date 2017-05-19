#ifndef GRAPHICS_SYSTEM_H
#define GRAPHICS_SYSTEM_H

#include <vector>
#include "2D/Renderer2D.h"
#include "3D/SceneRenderer.h"

namespace graphics {

	class Renderable3D;
	class Renderable2D;
	class Camera;
	class PointLight;


	/**
	 * Class GraphicsSystem
	 */
	class GraphicsSystem
	{
	private:	// Attributes
		static const unsigned int WIDTH = 640;
		static const unsigned int HEIGHT = 480;
		static const float FOV;
		static const float Z_NEAR;
		static const float Z_FAR;

		glm::mat4 mProjectionMatrix;

		Renderer2D mRenderer2D;

		SceneRenderer mSceneRenderer;

	public:		// Functions
		/** Creates a new Graphics System */
		GraphicsSystem();

		/** Class destructor */
		~GraphicsSystem() {};

		/** Draws the scene */
		void render(
			const Camera* camera,
			const std::vector<const Renderable3D*>& renderable3Ds,
			const std::vector<const Renderable2D*>& renderable2Ds,
			const std::vector<const PointLight*>& pointLights
		);
	};

}

#endif		// GRAPHICS_SYSTEM_H
