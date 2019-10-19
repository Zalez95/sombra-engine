#include "se/graphics/GLWrapper.h"
#include "se/graphics/Texture.h"
#include "se/graphics/3D/Mesh.h"
#include "se/graphics/3D/Camera.h"
#include "se/graphics/3D/Material.h"
#include "se/graphics/3D/Renderer3D.h"
#include "se/graphics/3D/Renderable3D.h"

namespace se::graphics {

	Renderer3D::Renderer3D()
	{
		mProgram.init();
		mSkinningProgram.init();
	}


	Renderer3D::~Renderer3D()
	{
		mProgram.end();
		mSkinningProgram.end();
	}


	void Renderer3D::submit(const Renderable3D* renderable3D)
	{
		if (renderable3D) {
			if (renderable3D->hasSkeleton()) {
				mSkinnedRenderable3Ds.push(renderable3D);
			}
			else {
				mRenderable3Ds.push(renderable3D);
			}
		}
	}


	void Renderer3D::render(const Camera* camera, const std::vector<const PointLight*>& pointLights)
	{
		glm::mat4 viewMatrix(1.0f), projectionMatrix(1.0f);
		if (camera) {
			viewMatrix = camera->getViewMatrix();
			projectionMatrix = camera->getProjectionMatrix();
		}

		// Render the renderable3Ds without skinning
		mProgram.enable();
		mProgram.setViewMatrix(viewMatrix);
		mProgram.setProjectionMatrix(projectionMatrix);
		mProgram.setLights(pointLights);
		while (!mRenderable3Ds.empty()) {
			const Renderable3D* renderable3D = mRenderable3Ds.front();
			mRenderable3Ds.pop();

			auto mesh = renderable3D->getMesh();
			auto material = renderable3D->getMaterial();

			if (!mesh) { continue; }

			// Bind the program data
			mProgram.setModelMatrix(renderable3D->getModelMatrix());
			if (material) { startMaterial(*material, mProgram); }

			// Draw
			mesh->bind();
			const IndexBuffer& ibo = mesh->getIBO();
			GL_WRAP( glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(ibo.getIndexCount()), toGLType(ibo.getIndexType()), nullptr) );
			mesh->unbind();

			// Unbind the program data
			if (material) { endMaterial(*material); }
		}
		mProgram.disable();

		// Render the renderable3Ds with skinning
		mSkinningProgram.enable();
		mSkinningProgram.setViewMatrix(viewMatrix);
		mSkinningProgram.setProjectionMatrix(projectionMatrix);
		mSkinningProgram.setLights(pointLights);
		while (!mSkinnedRenderable3Ds.empty()) {
			const Renderable3D* renderable3D = mSkinnedRenderable3Ds.front();
			mSkinnedRenderable3Ds.pop();

			auto mesh = renderable3D->getMesh();
			auto material = renderable3D->getMaterial();

			if (!mesh) { continue; }

			// Bind the program data
			mSkinningProgram.setModelMatrix(renderable3D->getModelMatrix());
			mSkinningProgram.setJointMatrices(renderable3D->getJointMatrices());
			if (material) { startMaterial(*material, mSkinningProgram); }

			// Draw
			mesh->bind();
			const IndexBuffer& ibo = mesh->getIBO();
			GL_WRAP( glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(ibo.getIndexCount()), toGLType(ibo.getIndexType()), nullptr) );
			mesh->unbind();

			// Unbind the program data
			if (material) { endMaterial(*material); }
		}
		mSkinningProgram.disable();
	}

// Private functions
	void Renderer3D::startMaterial(const Material& material, Program3D& program)
	{
		program.setMaterial(material);

		// Set the material alphaMode
		if (material.alphaMode == AlphaMode::Blend) {
			GL_WRAP( glEnable(GL_BLEND) );
			GL_WRAP( glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) );
			GL_WRAP( glDisable(GL_DEPTH_TEST) );
		}

		// Unset face culling
		if (material.doubleSided) {
			GL_WRAP( glDisable(GL_CULL_FACE) );
		}
	}


	void Renderer3D::endMaterial(const Material& material)
	{
		// Set face culling
		if (material.doubleSided) {
			GL_WRAP( glEnable(GL_CULL_FACE) );
		}

		// Set the material alphaMode
		if (material.alphaMode == AlphaMode::Blend) {
			GL_WRAP( glEnable(GL_DEPTH_TEST) );
			GL_WRAP( glDisable(GL_BLEND) );
		}
	}

}
