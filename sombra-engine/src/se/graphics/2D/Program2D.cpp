#include <fstream>
#include "se/graphics/2D/Program2D.h"
#include "se/graphics/core/Shader.h"
#include "se/graphics/core/Program.h"
#include "se/utils/Log.h"

namespace se::graphics {

	void Program2D::setModelMatrix(const glm::mat4& modelMatrix)
	{
		mProgram->setUniform("uModelMatrix", modelMatrix);
	}


	void Program2D::setTextureSampler(int unit)
	{
		mProgram->setUniform("uTextureSampler", unit);
	}


// Private functions
	bool Program2D::createProgram()
	{
		// 1. Read the shader text from the shader files
		std::string vertexShaderText;
		if (std::ifstream ifs("res/shaders/vertex2D.glsl"); ifs.good()) {
			vertexShaderText.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
		}
		else {
			return false;
		}

		std::string fragmentShaderText;
		if (std::ifstream ifs("res/shaders/fragment2D.glsl"); ifs.good()) {
			fragmentShaderText.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
		}
		else {
			return false;
		}

		// 2. Create the Program
		try {
			Shader vertexShader(vertexShaderText.c_str(), ShaderType::Vertex);
			Shader fragmentShader(fragmentShaderText.c_str(), ShaderType::Fragment);
			const Shader* shaders[] = { &vertexShader, &fragmentShader };
			mProgram = new Program(shaders, 2);
		}
		catch (std::exception& e) {
			SOMBRA_ERROR_LOG << e.what();

			if (mProgram) {
				delete mProgram;
			}

			return false;
		}

		return true;
	}


	bool Program2D::addUniforms()
	{
		bool ret = true;

		ret &= mProgram->addUniform("uModelMatrix");
		ret &= mProgram->addUniform("uTextureSampler");

		return ret;
	}

}
