#include "se/graphics/3D/RendererSky.h"
#include "se/graphics/3D/Renderable3D.h"
#include "se/graphics/3D/Camera.h"
#include "se/graphics/3D/Mesh.h"
#include "../core/GLWrapper.h"

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

		GL_WRAP( glDisable(GL_CULL_FACE) );

		// Bind the program data
		mProgram.enable();
		mProgram.setViewMatrix(viewMatrix);
		mProgram.setProjectionMatrix(projectionMatrix);

		// Draw
		mesh->bind();
		const IndexBuffer& ibo = mesh->getIBO();
		GL_WRAP( glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(ibo.getIndexCount()), toGLType(ibo.getIndexType()), nullptr) );

		GL_WRAP( glEnable(GL_CULL_FACE) );
	}

}
