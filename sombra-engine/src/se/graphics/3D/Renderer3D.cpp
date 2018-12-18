#include "se/graphics/GLWrapper.h"
#include "se/graphics/Texture.h"
#include "se/graphics/3D/Mesh.h"
#include "se/graphics/3D/Camera.h"
#include "se/graphics/3D/Material.h"
#include "se/graphics/3D/Renderer3D.h"
#include "se/graphics/3D/Renderable3D.h"

namespace se::graphics {

	void Renderer3D::submit(const Renderable3D* renderable3D)
	{
		if (renderable3D) {
			mRenderable3Ds.push(renderable3D);
		}
	}


	void Renderer3D::render(const Camera* camera, const std::vector<const PointLight*>& pointLights)
	{
		glm::mat4 viewMatrix(1.0f), projectionMatrix(1.0f);
		if (camera) {
			viewMatrix = camera->getViewMatrix();
			projectionMatrix = camera->getProjectionMatrix();
		}

		mProgram.enable();
		mProgram.setViewMatrix(viewMatrix);
		mProgram.setProjectionMatrix(projectionMatrix);
		mProgram.setLights(pointLights);

		while (!mRenderable3Ds.empty()) {
			const Renderable3D* renderable3D = mRenderable3Ds.front();
			mRenderable3Ds.pop();

			auto mesh				= renderable3D->getMesh();
			auto material			= renderable3D->getMaterial();
			auto texture			= renderable3D->getTexture();
			glm::mat4 modelMatrix	= renderable3D->getModelMatrix();
			unsigned char flags		= renderable3D->getRenderFlags();

			if (!mesh) { continue; }

			// Bind the program data
			mProgram.setModelMatrix(modelMatrix);
			if (material) {
				mProgram.setMaterial(*material);
			}
			if (texture) {
				mProgram.setColorTexture(0);
				texture->bind(0);
			}

			// Set the mesh transparency
			if (flags & RenderFlags::DISABLE_DEPTH_TEST) {
				GL_WRAP( glEnable(GL_BLEND) );
				GL_WRAP( glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) );
				GL_WRAP( glDisable(GL_DEPTH_TEST) );
			}

			// Unset face culling
			if (flags & RenderFlags::DISABLE_FACE_CULLING) {
				GL_WRAP( glDisable(GL_CULL_FACE) );
			}

			// Set mesh wireframe
			if (flags & RenderFlags::WIREFRAME) {
				GL_WRAP( glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) );
			}

			// Draw
			mesh->bind();
			GL_WRAP( glDrawElements(GL_TRIANGLES, mesh->getIndexCount(), GL_UNSIGNED_SHORT, nullptr) );
			mesh->unbind();

			// Unset mesh wireframe
			if (flags & RenderFlags::WIREFRAME) {
				GL_WRAP( glPolygonMode(GL_FRONT_AND_BACK, GL_FILL) );
			}

			// Set face culling
			if (flags & RenderFlags::DISABLE_FACE_CULLING) {
				GL_WRAP( glEnable(GL_CULL_FACE) );
			}

			// Unset the mesh transparency
			if (flags & RenderFlags::DISABLE_DEPTH_TEST) {
				GL_WRAP( glEnable(GL_DEPTH_TEST) );
				GL_WRAP( glDisable(GL_BLEND) );
			}

			// Unbind the program data
			if (texture) {
				texture->unbind();
			}
		}

		mProgram.disable();
	}

}
