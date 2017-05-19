#include "Renderer2D.h"
#include <string>
#include <sstream>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include "../Shader.h"
#include "Renderable2D.h"

namespace graphics {

// Static variables definition
	const GLfloat Renderer2D::Quad2D::mPositions[] = { -1,1, -1,-1, 1,1, 1,-1 };

// Public functions
	Renderer2D::Renderer2D()
	{
		// 1. Read the shader text from the shader files
		std::ifstream reader;

		std::string vertexShaderText;
		std::stringstream vertexShaderStream;
		reader.open("res/shaders/2D.vert");
		vertexShaderStream << reader.rdbuf();
		vertexShaderText = vertexShaderStream.str();
		reader.close();

		std::string fragmentShaderText;
		std::stringstream fragmentShaderStream;
		reader.open("res/shaders/2D.frag");
		fragmentShaderStream << reader.rdbuf();
		fragmentShaderText = fragmentShaderStream.str();
		reader.close();

		// 2. Create the Program
		Shader vertexShader(vertexShaderText.c_str(), GL_VERTEX_SHADER);
		Shader fragmentShader(fragmentShaderText.c_str(), GL_FRAGMENT_SHADER);

		std::vector<const Shader*> shaders = { &vertexShader, &fragmentShader };
		mProgram = new Program(shaders);

		// 3. Get the uniform locations
		mUniformLocations.u_ModelMatrix = mProgram->getUniformLocation("u_ModelMatrix");
	}


	Renderer2D::~Renderer2D()
	{
		delete mProgram;
	}


	void Renderer2D::render()
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_DEPTH_TEST);

		mProgram->enable();
		mQuad.bindVAO();

		while (!mRenderable2Ds.empty()) {
			const Renderable2D* renderable2D = mRenderable2Ds.front();
			mRenderable2Ds.pop();

			glm::mat4 transforms = glm::translate(glm::mat4(), glm::vec3(renderable2D->getPosition(), 0));
			transforms *= glm::scale(glm::mat4(), glm::vec3(renderable2D->getScale(), 1));
			mProgram->setUniform(mUniformLocations.u_ModelMatrix, transforms);

			glActiveTexture(GL_TEXTURE0);
			renderable2D->getTexture()->bind();
	
			glDrawArrays(GL_TRIANGLE_STRIP, 0, mQuad.getNumVertices());
		}
		glBindTexture(GL_TEXTURE_2D, 0);

		glBindVertexArray(0);
		mProgram->disable();
		
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
	}

}
