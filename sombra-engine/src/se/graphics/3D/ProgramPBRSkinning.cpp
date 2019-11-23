#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <GL/glew.h>
#include "se/graphics/3D/ProgramPBRSkinning.h"
#include "se/graphics/Shader.h"
#include "se/graphics/Program.h"
#include "se/graphics/3D/Lights.h"
#include "se/graphics/3D/Material.h"

namespace se::graphics {

	bool ProgramPBRSkinning::init()
	{
		return createProgram("res/shaders/vertexPBRSkinning.glsl", "res/shaders/fragmentPBR.glsl")
			&& addUniforms();
	}


	void ProgramPBRSkinning::setJointMatrices(const std::vector<glm::mat4>& jointMatrices) const
	{
		std::size_t numJoints = std::min(jointMatrices.size(), static_cast<std::size_t>(kMaxJoints));
		mProgram->setUniformV("uJointMatrices", numJoints, jointMatrices.data());
	}

// Private functions
	bool ProgramPBRSkinning::addUniforms()
	{
		return ProgramPBR::addUniforms()
			&& mProgram->addUniform("uJointMatrices");
	}

}
