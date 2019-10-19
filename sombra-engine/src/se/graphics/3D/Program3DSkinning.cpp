#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <GL/glew.h>
#include "se/graphics/3D/Program3DSkinning.h"
#include "se/graphics/Shader.h"
#include "se/graphics/Program.h"
#include "se/graphics/3D/Lights.h"
#include "se/graphics/3D/Material.h"

namespace se::graphics {

	bool Program3DSkinning::init()
	{
		if (!initShaders("res/shaders/vertex3DSkinning.glsl", "res/shaders/fragment3D.glsl")) {
			return false;
		}

		initUniformLocations();

		return true;
	}


	void Program3DSkinning::setJointMatrices(const std::vector<glm::mat4>& jointMatrices) const
	{
		std::size_t numJoints = std::min(jointMatrices.size(), static_cast<std::size_t>(kMaxJoints));
		mProgram->setUniformV(mUniformLocationsSkinning.jointMatrices, numJoints, jointMatrices.data());
	}

// Private functions
	void Program3DSkinning::initUniformLocations()
	{
		Program3D::initUniformLocations();

		mUniformLocationsSkinning.jointMatrices = mProgram->getUniformLocation("uJointMatrices");
	}

}
