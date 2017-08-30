#include "Program2D.h"
#include <string>
#include <sstream>
#include <fstream>
#include "../Shader.h"
#include "../Program.h"

namespace graphics {

	Program2D::Program2D()
	{
		initShaders();
		initUniformLocations();
	}


	Program2D::~Program2D()
	{
		delete mProgram;
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
		mProgram->setUniform(mUniformLocations.mModelMatrix, modelMatrix);
	}

// Private functions
	void Program2D::initShaders()
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

		Shader vertexShader(vertexShaderText.c_str(), GL_VERTEX_SHADER);
		Shader fragmentShader(fragmentShaderText.c_str(), GL_FRAGMENT_SHADER);

		// 2. Create the Program
		std::vector<const Shader*> shaders = { &vertexShader, &fragmentShader };
		mProgram = new Program(shaders);
	}


	void Program2D::initUniformLocations()
	{
		mUniformLocations.mModelMatrix = mProgram->getUniformLocation("u_ModelMatrix");
	}

}
