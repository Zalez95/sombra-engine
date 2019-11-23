#include <string>
#include <sstream>
#include <fstream>
#include <GL/glew.h>
#include "se/graphics/2D/Program2D.h"
#include "se/graphics/Shader.h"
#include "se/graphics/Program.h"

namespace se::graphics {

	bool Program2D::init()
	{
		return createProgram("res/shaders/vertex2D.glsl", "res/shaders/fragment2D.glsl")
			&& addUniforms();
	}


	void Program2D::end()
	{
		if (mProgram) {
			delete mProgram;
		}
	}


	void Program2D::enable() const
	{
		mProgram->enable();
	}


	void Program2D::disable() const
	{
		mProgram->disable();
	}


	void Program2D::setModelMatrix(const glm::mat4& modelMatrix)
	{
		mProgram->setUniform("uModelMatrix", modelMatrix);
	}


	void Program2D::setTextureSampler(int unit)
	{
		mProgram->setUniform("uTextureSampler", unit);
	}


// Private functions
	bool Program2D::createProgram(const char* vertexShaderPath, const char* fragmentShaderPath)
	{
		// 1. Read the shader text from the shader files
		std::ifstream reader;

		std::string vertexShaderText;
		std::stringstream vertexShaderStream;
		reader.open(vertexShaderPath);
		if (reader.good()) {
			vertexShaderStream << reader.rdbuf();
			vertexShaderText = vertexShaderStream.str();
			reader.close();
		}
		else {
			reader.close();
			return false;
		}

		std::string fragmentShaderText;
		std::stringstream fragmentShaderStream;
		reader.open(fragmentShaderPath);
		if (reader.good()) {
			fragmentShaderStream << reader.rdbuf();
			fragmentShaderText = fragmentShaderStream.str();
			reader.close();
		}
		else {
			reader.close();
			return false;
		}

		Shader vertexShader(vertexShaderText.c_str(), ShaderType::Vertex);
		Shader fragmentShader(fragmentShaderText.c_str(), ShaderType::Fragment);

		// 2. Create the Program
		const Shader* shaders[] = { &vertexShader, &fragmentShader };
		mProgram = new Program(shaders, 2);

		return true;
	}


	bool Program2D::addUniforms()
	{
		return mProgram->addUniform("uModelMatrix")
			&& mProgram->addUniform("uTextureSampler");
	}

}
