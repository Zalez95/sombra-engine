#include "GraphicsSystem.h"
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include "3D/Camera.h"
#include "3D/Renderable3D.h"
#include "text/RenderableText.h"

namespace graphics {

// Static attributes
	const float GraphicsSystem::FOV		= glm::radians(50.0f);
	const float GraphicsSystem::Z_NEAR	= 1.0f;
	const float GraphicsSystem::Z_FAR	= 100.0f;

// Public functions
	GraphicsSystem::GraphicsSystem() :
		mRenderer2D(glm::ortho(0.0f, (float)WIDTH, (float)HEIGHT, 0.0f, Z_NEAR, Z_FAR)),
		mRenderer3D(glm::perspective(FOV, (float)WIDTH / (float)HEIGHT, Z_NEAR, Z_FAR))//,
//		mRendererText()
	{
		// Enable depth-testing
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LEQUAL);		// Write if depth <= depth buffer
		glDepthRange(0.0f, 1.0f);	// The Z coordinate range is [0,1]

		// Enable face culling - tells OpenGL to not draw the faces that
		// cannot be seen
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);		// Render only the counter-clockwise faces

		// The Clear Color of the window
		glClearColor(1.0f, 0.95f, 1.0f, 1.0f);
	}


	void GraphicsSystem::render(
		const Camera* camera,
		const std::vector<const Renderable2D*>& renderable2Ds,
		const std::vector<const Renderable3D*>& renderable3Ds,
		const std::vector<const RenderableText*>& renderableTexts,
		const std::vector<const PointLight*>& pointLights
	) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		for (const Renderable2D* renderable2D : renderable2Ds) {
			mRenderer2D.submit(renderable2D);
		}
		mRenderer2D.render();

		for (const Renderable3D* renderable3D : renderable3Ds) {
			mRenderer3D.submit(renderable3D);
		}
		mRenderer3D.render(camera, pointLights);

//		for (const RenderableText* renderableText : renderableTexts) {
//			mRendererText.submit(renderableText);
//		}
//		mRendererText.render();
	}

}
