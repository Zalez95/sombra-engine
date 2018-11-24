#include <string>
#include <sstream>
#include <fstream>
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include "se/graphics/Texture.h"
#include "se/graphics/2D/Renderer2D.h"
#include "se/graphics/2D/Renderable2D.h"

namespace se::graphics {

	constexpr float Renderer2D::Quad2D::kPositions[];


	Renderer2D::Quad2D::Quad2D() :
		mPositionsBuffer(kPositions, kNumVertices * kNumComponentsPerVertex, kNumComponentsPerVertex)
	{
		mVAO.addBuffer(mPositionsBuffer, 0);
	}


	void Renderer2D::submit(const Renderable2D* renderable2D)
	{
		if (renderable2D) {
			mRenderable2Ds.push(renderable2D);
		}
	}


	void Renderer2D::render()
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_DEPTH_TEST);

		mProgram.enable();
		mQuad.bindVAO();

		while (!mRenderable2Ds.empty()) {
			const Renderable2D* renderable2D = mRenderable2Ds.front();
			mRenderable2Ds.pop();

			glm::mat4 transforms = glm::translate(glm::mat4(1.0f), glm::vec3(renderable2D->getPosition(), 0));
			transforms *= glm::scale(glm::mat4(1.0f), glm::vec3(renderable2D->getScale(), 1));
			auto texture = renderable2D->getTexture();

			mProgram.setModelMatrix(transforms);
			mProgram.setTextureSampler(0);

			texture->bind(0);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, mQuad.getNumVertices());
			texture->unbind();
		}

		glBindVertexArray(0);
		mProgram.disable();

		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
	}

}
