#include "Renderer3D.h"
#include "../Texture.h"
#include "Renderable3D.h"
#include "Mesh.h"
#include "Camera.h"

namespace graphics {

	void Renderer3D::submit(const Renderable3D* renderable3D)
	{
		if (renderable3D) {
			mRenderable3Ds.push(renderable3D);
		}
	}


	void Renderer3D::render(const Camera& camera, const std::vector<const PointLight*>& pointLights)
	{
		glm::mat4 viewMatrix = camera.getViewMatrix();

		mProgram.enable();
		mProgram.setViewMatrix(viewMatrix);
		mProgram.setProjectionMatrix(mProjectionMatrix);
		mProgram.setLights(pointLights);

		while (!mRenderable3Ds.empty()) {
			const Renderable3D* renderable3D = mRenderable3Ds.front();
			mRenderable3Ds.pop();

			auto mesh				= renderable3D->getMesh();
			auto material			= renderable3D->getMaterial();
			auto texture			= renderable3D->getTexture();
			glm::mat4 modelMatrix	= renderable3D->getModelMatrix();

			if (mesh) {
				mProgram.setModelMatrix(modelMatrix);
				
				if (material) {
					mProgram.setMaterial(material.get());
				}
				if (texture) {
					glActiveTexture(GL_TEXTURE0);
					texture->bind();
				}

				// Draw
				mesh->bindVAO();
				glDrawElements(GL_TRIANGLES, mesh->getIndexCount(), GL_UNSIGNED_SHORT, nullptr);
				glBindVertexArray(0);

				if (texture) {
					texture->unbind();
				}
			}
		}

		mProgram.disable();
	}

}
