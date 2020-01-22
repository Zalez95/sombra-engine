#include <fstream>
#include <algorithm>
#include "se/graphics/3D/ProgramPBRSkinning.h"
#include "se/graphics/core/Shader.h"
#include "se/graphics/core/Program.h"
#include "se/utils/Log.h"

namespace se::graphics {

	void ProgramPBRSkinning::setJointMatrices(const std::vector<glm::mat4>& jointMatrices)
	{
		std::size_t numJoints = std::min(jointMatrices.size(), static_cast<std::size_t>(kMaxJoints));
		mProgram->setUniformV("uJointMatrices", numJoints, jointMatrices.data());
	}

// Private functions
	bool ProgramPBRSkinning::createProgram()
	{
		// 1. Read the shader text from the shader files
		std::string vertexShaderText;
		if (std::ifstream ifs("res/shaders/vertexLightSkinning.glsl"); ifs.good()) {
			vertexShaderText.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
		}
		else {
			return false;
		}

		std::string fragmentShaderText;
		if (std::ifstream ifs("res/shaders/fragmentPBR.glsl"); ifs.good()) {
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


	bool ProgramPBRSkinning::addUniforms()
	{
		bool ret = ProgramPBR::addUniforms();

		ret &= mProgram->addUniform("uJointMatrices");

		return ret;
	}

}
