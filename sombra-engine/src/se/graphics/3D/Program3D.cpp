#include <fstream>
#include "se/graphics/3D/Program3D.h"
#include "se/graphics/core/Shader.h"
#include "se/graphics/core/Program.h"
#include "se/utils/Log.h"

namespace se::graphics {

	void Program3D::setViewMatrix(const glm::mat4& viewMatrix) const
	{
		mProgram->setUniform("uViewMatrix", viewMatrix);
	}


	void Program3D::setProjectionMatrix(const glm::mat4& projectionMatrix) const
	{
		mProgram->setUniform("uProjectionMatrix", projectionMatrix);
	}

// Private functions
	bool Program3D::createProgram()
	{
		// 1. Read the shader text from the shader files
		std::string vertexShaderText;
		if (std::ifstream ifs("res/shaders/vertex3D.glsl"); ifs.good()) {
			vertexShaderText.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
		}
		else {
			return false;
		}

		std::string fragmentShaderText;
		if (std::ifstream ifs("res/shaders/fragment3D.glsl"); ifs.good()) {
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


	bool Program3D::addUniforms()
	{
		bool ret = true;

		ret &= mProgram->addUniform("uViewMatrix");
		ret &= mProgram->addUniform("uProjectionMatrix");

		return ret;
	}

}
