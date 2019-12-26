#include "se/graphics/3D/RendererPBR.h"
#include "se/graphics/3D/Mesh.h"
#include "se/graphics/3D/Camera.h"
#include "../core/GLWrapper.h"

namespace se::graphics {

	RendererPBR::RendererPBR()
	{
		if (!mProgramPBR.init()) {
			SOMBRA_ERROR_LOG << "Failed to create the Program3D";
		}

		if (!mProgramPBRSkinning.init()) {
			SOMBRA_ERROR_LOG << "Failed to create the Program3DSkinning";
		}
	}


	RendererPBR::~RendererPBR()
	{
		mProgramPBRSkinning.end();
		mProgramPBR.end();
	}


	void RendererPBR::submit(const Renderable3D* renderable3D)
	{
		mRenderable3Ds.push(renderable3D);
	}


	void RendererPBR::submitSkinned(const Renderable3D* renderable3D)
	{
		mSkinnedRenderable3Ds.push(renderable3D);
	}


	void RendererPBR::render(const Camera* camera, const std::vector<const ILight*>& lights)
	{
		glm::mat4 viewMatrix(1.0f), projectionMatrix(1.0f);
		if (camera) {
			viewMatrix = camera->getViewMatrix();
			projectionMatrix = camera->getProjectionMatrix();
		}

		// Render the renderable3Ds without skinning
		mProgramPBR.enable();
		mProgramPBR.setViewMatrix(viewMatrix);
		mProgramPBR.setProjectionMatrix(projectionMatrix);
		mProgramPBR.setLights(lights);
		while (!mRenderable3Ds.empty()) {
			const Renderable3D* renderable3D = mRenderable3Ds.front();
			mRenderable3Ds.pop();

			auto mesh = renderable3D->getMesh();
			auto material = renderable3D->getMaterial();

			if (!mesh) { continue; }

			// Bind the program data
			mProgramPBR.setModelMatrix(renderable3D->getModelMatrix());
			if (material) { mProgramPBR.setMaterial(*material); }

			// Draw
			mesh->bind();
			const IndexBuffer& ibo = mesh->getIBO();
			GL_WRAP( glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(ibo.getIndexCount()), toGLType(ibo.getIndexType()), nullptr) );

			// Unbind the program data
			if (material) { mProgramPBR.unsetMaterial(*material); }
		}

		// Render the renderable3Ds with skinning
		mProgramPBRSkinning.enable();
		mProgramPBRSkinning.setViewMatrix(viewMatrix);
		mProgramPBRSkinning.setProjectionMatrix(projectionMatrix);
		mProgramPBRSkinning.setLights(lights);
		while (!mSkinnedRenderable3Ds.empty()) {
			const Renderable3D* renderable3D = mSkinnedRenderable3Ds.front();
			mSkinnedRenderable3Ds.pop();

			auto mesh = renderable3D->getMesh();
			auto material = renderable3D->getMaterial();

			if (!mesh) { continue; }

			// Bind the program data
			mProgramPBRSkinning.setModelMatrix(renderable3D->getModelMatrix());
			mProgramPBRSkinning.setJointMatrices(renderable3D->getJointMatrices());
			if (material) { mProgramPBRSkinning.setMaterial(*material); }

			// Draw
			mesh->bind();
			const IndexBuffer& ibo = mesh->getIBO();
			GL_WRAP( glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(ibo.getIndexCount()), toGLType(ibo.getIndexType()), nullptr) );

			// Unbind the program data
			if (material) { mProgramPBRSkinning.unsetMaterial(*material); }
		}
	}

}
