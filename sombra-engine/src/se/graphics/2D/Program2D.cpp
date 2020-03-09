#include <fstream>
#include <algorithm>
#include "se/graphics/2D/Program2D.h"
#include "se/graphics/core/Shader.h"
#include "se/graphics/core/Program.h"
#include "se/graphics/core/Texture.h"
#include "se/utils/Log.h"

namespace se::graphics {

	void Program2D::setProjectionMatrix(const glm::mat4& projectionMatrix)
	{
		mProgram->setUniform("uProjectionMatrix", projectionMatrix);
	}


	void Program2D::setTextures(const TextureSPtr* textures, std::size_t textureCount)
	{
		int numTextures = std::min(kMaxTextures, static_cast<int>(textureCount));
		for (int i = 0; i < numTextures; ++i) {
			utils::ArrayStreambuf<char, 64> aStreambuf;
			std::ostream(&aStreambuf) << "uTextures[" << i << "]";
			mProgram->setUniform(aStreambuf.data(), i);
			textures[i]->bind(i);
		}
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

		ret &= mProgram->addUniform("uProjectionMatrix");
		for (std::size_t i = 0; i < kMaxTextures; ++i) {
			utils::ArrayStreambuf<char, 64> aStreambuf;
			std::ostream(&aStreambuf) << "uTextures[" << i << "]";
			ret &= mProgram->addUniform(aStreambuf.data());
		}

		return ret;
	}

}
