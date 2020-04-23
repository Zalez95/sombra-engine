#include "se/graphics/3D/RendererSky.h"
#include "se/graphics/3D/Renderable3D.h"
#include "se/graphics/3D/Camera.h"
#include "se/graphics/3D/Mesh.h"
#include "se/graphics/core/Graphics.h"
#include "se/utils/Log.h"

namespace se::graphics {

	RendererSky::RendererSky()
	{
		if (!mProgram.init()) {
			SOMBRA_ERROR_LOG << "Failed to create the Program";
		}
	}


	RendererSky::~RendererSky()
	{
		mProgram.end();
	}


	void RendererSky::render(const Camera* camera, const Renderable3D& sky)
	{
		glm::mat4 viewMatrix(1.0f), projectionMatrix(1.0f);
		if (camera) {
			viewMatrix = camera->getViewMatrix();
			projectionMatrix = camera->getProjectionMatrix();
		}

		auto mesh = sky.getMesh();

		Graphics::setCulling(false);

		// Bind the program data
		mProgram.bind();
		mProgram.setViewMatrix(viewMatrix);
		mProgram.setProjectionMatrix(projectionMatrix);

		// Draw
		mesh->bind();
		Graphics::drawIndexed(PrimitiveType::Triangle, mesh->getIBO().getIndexCount(), mesh->getIBO().getIndexType());

		Graphics::setCulling(true);
	}

}
