#include "RendererText.h"
#include <string>
#include <sstream>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include "../Shader.h"
#include "../Texture.h"
#include "RenderableText.h"

namespace graphics {

// Public functions
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
		Shader vertexShader(vertexShaderText.c_str(), GL_VERTEX_SHADER);
		Shader fragmentShader(fragmentShaderText.c_str(), GL_FRAGMENT_SHADER);

		std::vector<const Shader*> shaders = { &vertexShader, &fragmentShader };
		mProgram = new Program(shaders);

		// 3. Get the uniform locations
//		mUniformLocations.u_ModelMatrix = mProgram->getUniformLocation("u_ModelMatrix");
	}


	RendererText::~RendererText()
	{
		delete mProgram;
	}


	void RendererText::render()
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_DEPTH_TEST);

		mProgram->enable();

		while (!mRenderableTexts.empty()) {
			const RenderableText* renderableText = mRenderableTexts.front();
			mRenderableTexts.pop();

			glActiveTexture(GL_TEXTURE0);
			renderableText->getFont()->getTextureAtlas()->bind();

			for (char c : renderableText->getText()) {

//				glDrawArrays(GL_TRIANGLE_STRIP, 0, mQuad.getNumVertices());
			}
		}
		glBindTexture(GL_TEXTURE_2D, 0);

		mProgram->disable();
		
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
	}

}
