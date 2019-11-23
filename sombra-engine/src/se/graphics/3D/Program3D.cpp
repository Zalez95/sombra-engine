#include <sstream>
#include <fstream>
#include "se/graphics/3D/Program3D.h"
#include "se/graphics/Shader.h"
#include "se/graphics/Program.h"

namespace se::graphics {

	bool Program3D::init()
	{
		return createProgram("res/shaders/vertex3D.glsl", "res/shaders/fragment3D.glsl")
			&& addUniforms();
	}


	bool Program3D::end()
	{
		if (mProgram) {
			delete mProgram;
		}

		return true;
	}


	void Program3D::enable() const
	{
		mProgram->enable();
	}


	void Program3D::disable() const
	{
		mProgram->disable();
	}


	void Program3D::setModelMatrix(const glm::mat4& modelMatrix) const
	{
		mProgram->setUniform("uModelMatrix", modelMatrix);
	}


	void Program3D::setViewMatrix(const glm::mat4& viewMatrix) const
	{
		mProgram->setUniform("uViewMatrix", viewMatrix);
	}


	void Program3D::setProjectionMatrix(const glm::mat4& projectionMatrix) const
	{
		mProgram->setUniform("uProjectionMatrix", projectionMatrix);
	}

// Private functions
	bool Program3D::createProgram(const char* vertexShaderPath, const char* fragmentShaderPath)
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


	bool Program3D::addUniforms()
	{
		return mProgram->addUniform("uModelMatrix")
			&& mProgram->addUniform("uViewMatrix")
			&& mProgram->addUniform("uProjectionMatrix");
	}

}
