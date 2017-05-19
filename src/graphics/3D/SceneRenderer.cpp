#include "SceneRenderer.h"
#include "../Texture.h"
#include "Renderable3D.h"
#include "Mesh.h"
#include "Camera.h"

namespace graphics {

	void SceneRenderer::render(const Camera* camera, const std::vector<const PointLight*>& pointLights)
	{
		if (!camera) return;

		glm::mat4 viewMatrix = camera->getViewMatrix();

		mProgram.enable();
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
				glm::mat4 modelViewMatrix = viewMatrix * modelMatrix;
				mProgram.setModelViewMatrix(modelViewMatrix);
				
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
