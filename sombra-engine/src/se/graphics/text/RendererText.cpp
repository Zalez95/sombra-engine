#include <string>
#include <sstream>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include "se/graphics/core/Shader.h"
#include "se/graphics/core/Program.h"
#include "se/graphics/core/Texture.h"
#include "se/graphics/text/RendererText.h"
#include "se/graphics/text/RenderableText.h"
#include "../core/GLWrapper.h"

namespace se::graphics {

	RendererText::RendererText()
	{
		// 1. Read the shader text from the shader files
		std::ifstream reader;

		std::string vertexShaderText;
		std::stringstream vertexShaderStream;
		reader.open("res/shaders/text.vert");
		vertexShaderStream << reader.rdbuf();
		vertexShaderText = vertexShaderStream.str();
		reader.close();

		std::string fragmentShaderText;
		std::stringstream fragmentShaderStream;
		reader.open("res/shaders/text.frag");
		fragmentShaderStream << reader.rdbuf();
		fragmentShaderText = fragmentShaderStream.str();
		reader.close();

		// 2. Create the Program
		Shader vertexShader(vertexShaderText.c_str(), ShaderType::Vertex);
		Shader fragmentShader(fragmentShaderText.c_str(), ShaderType::Fragment);

		const Shader* shaders[] = { &vertexShader, &fragmentShader };
		mProgram = new Program(shaders, 2);

		// 3. Get the uniform locations
//		mUniformLocations.u_ModelMatrix = mProgram->getUniformLocation("u_ModelMatrix");
	}


	RendererText::~RendererText()
	{
		delete mProgram;
	}


	void RendererText::render()
	{
		GL_WRAP( glEnable(GL_BLEND) );
		GL_WRAP( glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) );
		GL_WRAP( glDisable(GL_DEPTH_TEST) );

		mProgram->enable();

		while (!mRenderableTexts.empty()) {
			const RenderableText* renderableText = mRenderableTexts.front();
			mRenderableTexts.pop();

			auto textureAtlas = renderableText->getFont()->getTextureAtlas();

			textureAtlas->bind();
//			for (char c : renderableText->getText()) {
//				GL_WRAP( glDrawArrays(GL_TRIANGLE_STRIP, 0, mQuad.getNumVertices()) );
//			}
			textureAtlas->unbind();
		}

		mProgram->disable();

		GL_WRAP( glEnable(GL_DEPTH_TEST) );
		GL_WRAP( glDisable(GL_BLEND) );
	}

}
