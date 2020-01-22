#include <fstream>
#include "se/graphics/3D/ProgramSplatmap.h"
#include "se/graphics/3D/Material.h"
#include "se/graphics/core/Shader.h"
#include "se/graphics/core/Program.h"
#include "se/utils/Log.h"

namespace se::graphics {

	void ProgramSplatmap::setMaterial(const SplatmapMaterial& material)
	{
		unsigned int numMaterials = static_cast<unsigned int>(material.materials.size());
		mProgram->setUniform("uSMaterial.numMaterials", numMaterials);

		for (int i = 0; i < static_cast<int>(numMaterials); ++i) {
			mProgram->setUniform(("uSMaterial.materials[" + std::to_string(i) + "].pbrMetallicRoughness.baseColorFactor").c_str(), material.materials[i].pbrMetallicRoughness.baseColorFactor);

			int useBaseColorTexture = (material.materials[i].pbrMetallicRoughness.baseColorTexture != nullptr);
			mProgram->setUniform(("uSMaterial.materials[" + std::to_string(i) + "].pbrMetallicRoughness.useBaseColorTexture").c_str(), useBaseColorTexture);
			if (useBaseColorTexture) {
				mProgram->setUniform(("uSMaterial.materials[" + std::to_string(i) + "].pbrMetallicRoughness.baseColorTexture").c_str(), TextureUnits::kBaseColor0);
				material.materials[i].pbrMetallicRoughness.baseColorTexture->bind(TextureUnits::kBaseColor0);
			}

			mProgram->setUniform(("uSMaterial.materials[" + std::to_string(i) + "].pbrMetallicRoughness.metallicFactor").c_str(), material.materials[i].pbrMetallicRoughness.metallicFactor);
			mProgram->setUniform(("uSMaterial.materials[" + std::to_string(i) + "].pbrMetallicRoughness.roughnessFactor").c_str(), material.materials[i].pbrMetallicRoughness.roughnessFactor);

			int useMetallicRoughnessTexture = (material.materials[i].pbrMetallicRoughness.metallicRoughnessTexture != nullptr);
			mProgram->setUniform(("uSMaterial.materials[" + std::to_string(i) + "].pbrMetallicRoughness.useMetallicRoughnessTexture").c_str(), useMetallicRoughnessTexture);
			if (useMetallicRoughnessTexture) {
				mProgram->setUniform(("uSMaterial.materials[" + std::to_string(i) + "].pbrMetallicRoughness.metallicRoughnessTexture").c_str(), TextureUnits::kMetallicRoughness0);
				material.materials[i].pbrMetallicRoughness.metallicRoughnessTexture->bind(TextureUnits::kMetallicRoughness0);
			}

			int useNormalTexture = (material.materials[i].normalTexture != nullptr);
			mProgram->setUniform(("uSMaterial.materials[" + std::to_string(i) + "].useNormalTexture").c_str(), useNormalTexture);
			if (useNormalTexture) {
				mProgram->setUniform(("uSMaterial.materials[" + std::to_string(i) + "].normalTexture").c_str(), TextureUnits::kNormal0);
				material.materials[i].normalTexture->bind(TextureUnits::kNormal0);
				mProgram->setUniform(("uSMaterial.materials[" + std::to_string(i) + "].normalScale").c_str(), material.materials[i].normalScale);
			}
		}

		mProgram->setUniform("uSMaterial.splatmapTexture", TextureUnits::kSplatmap);
		material.splatmapTexture->bind(TextureUnits::kSplatmap);
	}

// Private functions
	bool ProgramSplatmap::createProgram()
	{
		// 1. Read the shader text from the shader files
		std::string vertexShaderText;
		if (std::ifstream ifs("res/shaders/vertexLight.glsl"); ifs.good()) {
			vertexShaderText.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
		}
		else {
			return false;
		}

		std::string fragmentShaderText;
		if (std::ifstream ifs("res/shaders/fragmentSplatmap.glsl"); ifs.good()) {
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


	bool ProgramSplatmap::addUniforms()
	{
		bool ret = ProgramLight::addUniforms();

		ret &= mProgram->addUniform("uSMaterial.numMaterials");
		for (int i = 0; i < 4; ++i) {
			ret &= mProgram->addUniform(("uSMaterial.materials[" + std::to_string(i) + "].pbrMetallicRoughness.baseColorFactor").c_str());
			ret &= mProgram->addUniform(("uSMaterial.materials[" + std::to_string(i) + "].pbrMetallicRoughness.useBaseColorTexture").c_str());
			ret &= mProgram->addUniform(("uSMaterial.materials[" + std::to_string(i) + "].pbrMetallicRoughness.baseColorTexture").c_str());
			ret &= mProgram->addUniform(("uSMaterial.materials[" + std::to_string(i) + "].pbrMetallicRoughness.metallicFactor").c_str());
			ret &= mProgram->addUniform(("uSMaterial.materials[" + std::to_string(i) + "].pbrMetallicRoughness.roughnessFactor").c_str());
			ret &= mProgram->addUniform(("uSMaterial.materials[" + std::to_string(i) + "].pbrMetallicRoughness.useMetallicRoughnessTexture").c_str());
			ret &= mProgram->addUniform(("uSMaterial.materials[" + std::to_string(i) + "].pbrMetallicRoughness.metallicRoughnessTexture").c_str());
			ret &= mProgram->addUniform(("uSMaterial.materials[" + std::to_string(i) + "].useNormalTexture").c_str());
			ret &= mProgram->addUniform(("uSMaterial.materials[" + std::to_string(i) + "].normalTexture").c_str());
			ret &= mProgram->addUniform(("uSMaterial.materials[" + std::to_string(i) + "].normalScale").c_str());
		}
		ret &= mProgram->addUniform("uSMaterial.splatmapTexture");

		return ret;
	}

}
