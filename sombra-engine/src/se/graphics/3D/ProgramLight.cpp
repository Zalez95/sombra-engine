#include <array>
#include "se/graphics/3D/ProgramLight.h"
#include "se/graphics/3D/Lights.h"
#include "se/graphics/core/Program.h"

namespace se::graphics {

	struct ShaderPointLight
	{
		glm::vec3 color;
		float intensity;
		float inverseRange;
		glm::vec3 padding;
	};


	void ProgramLight::setModelMatrix(const glm::mat4& modelMatrix)
	{
		mProgram->setUniform("uModelMatrix", modelMatrix);
	}


	void ProgramLight::setLights(const std::vector<const ILight*>& lights)
	{
		unsigned int uNumPointLights = 0;
		std::array<ShaderPointLight, kMaxPointLights> uPointLights;
		std::array<glm::vec3, kMaxPointLights> uPointLightsPositions;

		for (const ILight* light : lights) {
			const PointLight* pLight = dynamic_cast<const PointLight*>(light);
			if (pLight && (uNumPointLights < kMaxPointLights)) {
				uPointLights[uNumPointLights].color = pLight->color;
				uPointLights[uNumPointLights].intensity = pLight->intensity;
				uPointLights[uNumPointLights].inverseRange = pLight->inverseRange;
				uPointLightsPositions[uNumPointLights] = pLight->position;
				uNumPointLights++;
			}
		}

		mProgram->setUniform("uNumPointLights", uNumPointLights);
		mPointLightsUBO.copy(uPointLights.data(), uPointLights.size());
		mPointLightsUBO.bind();
		mProgram->setUniformBlock("LightsBlock", UniformBlockIndices::kPointLights);
		mProgram->setUniformV("uPointLightsPositions", uNumPointLights, uPointLightsPositions.data());
	}

// Private functions
	bool ProgramLight::addUniforms()
	{
		bool ret = Program3D::addUniforms();

		ret &= mProgram->addUniform("uModelMatrix");

		ret &= mProgram->addUniform("uNumPointLights");
		ret &= mProgram->addUniformBlock("LightsBlock");
		ret &= mProgram->addUniform("uPointLightsPositions");

		return ret;
	}

}
