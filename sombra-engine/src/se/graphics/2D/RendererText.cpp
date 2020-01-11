#include <string>
#include <sstream>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include "se/graphics/core/Shader.h"
#include "se/graphics/core/Program.h"
#include "se/graphics/core/Texture.h"
#include "se/graphics/2D/RendererText.h"
#include "se/graphics/2D/RenderableText.h"
#include "../core/GLWrapper.h"

namespace se::graphics {

	RendererText::Quad2D::Quad2D()
	{
		mPositionsBuffer.setData(kPositions, kNumVertices * kNumComponentsPerVertex);

		mVAO.bind();
		mPositionsBuffer.bind();
		mVAO.setVertexAttribute(0, TypeId::Float, false, kNumComponentsPerVertex, 0);
		mVAO.unbind();
	}


	RendererText::RendererText()
	{
		if (!mProgram.init()) {
			SOMBRA_ERROR_LOG << "Failed to create the Program2D";
		}
	}


	RendererText::~RendererText()
	{
		mProgram.end();
	}


	void RendererText::submit(const RenderableText* renderableText)
	{
		if (renderableText) {
			mRenderableTexts.push(renderableText);
		}
	}


	void RendererText::render(const glm::mat4& projectionMatrix)
	{
		GL_WRAP( glEnable(GL_BLEND) );
		GL_WRAP( glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) );
		GL_WRAP( glDisable(GL_DEPTH_TEST) );

		mProgram.enable();
		mProgram.setProjectionMatrix(projectionMatrix);

		mQuad.bind();

		while (!mRenderableTexts.empty()) {
			const RenderableText* renderableText = mRenderableTexts.front();
			mRenderableTexts.pop();

			float advance = 0.0f;
			auto font = renderableText->getFont();
			for (char c : renderableText->getText()) {
				auto itCharacter = font->characters.find(c);
				if (itCharacter != font->characters.end()) {
					const Character& character = itCharacter->second;

					glm::vec2 scaledOffset(advance, 0.0f);
					scaledOffset += (renderableText->getScale() * static_cast<glm::vec2>(character.offset)) / static_cast<glm::vec2>(character.size);

					glm::mat4 translation = glm::translate(glm::mat4(1.0f), { renderableText->getPosition() + scaledOffset, 0.0f });
					glm::mat4 scale = glm::scale(glm::mat4(1.0f), { renderableText->getScale(), 1.0f });

					mProgram.setModelViewMatrix(translation * scale);
					mProgram.setTextureSampler(0);

					if (character.texture) { character.texture->bind(0); }
					GL_WRAP( glDrawArrays(GL_TRIANGLE_STRIP, 0, mQuad.getNumVertices()) );
					if (character.texture) { character.texture->unbind(); }

					advance += (renderableText->getScale().x * character.advance) / static_cast<float>(character.size.x);
				}
			}
		}

		GL_WRAP( glEnable(GL_DEPTH_TEST) );
		GL_WRAP( glDisable(GL_BLEND) );
	}

}
