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
			utils::ArrayStreambuf<char, 128> aStreambuf;
			std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].pbrMetallicRoughness.baseColorFactor";
			mProgram->setUniform(aStreambuf.data(), material.materials[i].pbrMetallicRoughness.baseColorFactor);

			int useBaseColorTexture = (material.materials[i].pbrMetallicRoughness.baseColorTexture != nullptr);
			aStreambuf = {};
			std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].pbrMetallicRoughness.useBaseColorTexture";
			mProgram->setUniform(aStreambuf.data(), useBaseColorTexture);
			if (useBaseColorTexture) {
				aStreambuf = {};
				std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].pbrMetallicRoughness.baseColorTexture";
				mProgram->setUniform(aStreambuf.data(), TextureUnits::kBaseColor0);
				material.materials[i].pbrMetallicRoughness.baseColorTexture->bind(TextureUnits::kBaseColor0);
			}

			aStreambuf = {};
			std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].pbrMetallicRoughness.metallicFactor";
			mProgram->setUniform(aStreambuf.data(), material.materials[i].pbrMetallicRoughness.metallicFactor);

			aStreambuf = {};
			std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].pbrMetallicRoughness.roughnessFactor";
			mProgram->setUniform(aStreambuf.data(), material.materials[i].pbrMetallicRoughness.roughnessFactor);

			int useMetallicRoughnessTexture = (material.materials[i].pbrMetallicRoughness.metallicRoughnessTexture != nullptr);
			aStreambuf = {};
			std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].pbrMetallicRoughness.useMetallicRoughnessTexture";
			mProgram->setUniform(aStreambuf.data(), useMetallicRoughnessTexture);
			if (useMetallicRoughnessTexture) {
				aStreambuf = {};
				std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].pbrMetallicRoughness.metallicRoughnessTexture";
				mProgram->setUniform(aStreambuf.data(), TextureUnits::kMetallicRoughness0);
				material.materials[i].pbrMetallicRoughness.metallicRoughnessTexture->bind(TextureUnits::kMetallicRoughness0);
			}

			int useNormalTexture = (material.materials[i].normalTexture != nullptr);
			aStreambuf = {};
			std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].useNormalTexture";
			mProgram->setUniform(aStreambuf.data(), useNormalTexture);
			if (useNormalTexture) {
				aStreambuf = {};
				std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].normalTexture";
				mProgram->setUniform(aStreambuf.data(), TextureUnits::kNormal0);

				material.materials[i].normalTexture->bind(TextureUnits::kNormal0);

				aStreambuf = {};
				std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].normalScale";
				mProgram->setUniform(aStreambuf.data(), material.materials[i].normalScale);
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
			utils::ArrayStreambuf<char, 128> aStreambuf;
			std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].pbrMetallicRoughness.baseColorFactor";
			ret &= mProgram->addUniform(aStreambuf.data());

			aStreambuf = {};
			std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].pbrMetallicRoughness.useBaseColorTexture";
			ret &= mProgram->addUniform(aStreambuf.data());

			aStreambuf = {};
			std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].pbrMetallicRoughness.baseColorTexture";
			ret &= mProgram->addUniform(aStreambuf.data());

			aStreambuf = {};
			std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].pbrMetallicRoughness.metallicFactor";
			ret &= mProgram->addUniform(aStreambuf.data());

			aStreambuf = {};
			std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].pbrMetallicRoughness.roughnessFactor";
			ret &= mProgram->addUniform(aStreambuf.data());

			aStreambuf = {};
			std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].pbrMetallicRoughness.useMetallicRoughnessTexture";
			ret &= mProgram->addUniform(aStreambuf.data());

			aStreambuf = {};
			std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].pbrMetallicRoughness.metallicRoughnessTexture";
			ret &= mProgram->addUniform(aStreambuf.data());

			aStreambuf = {};
			std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].useNormalTexture";
			ret &= mProgram->addUniform(aStreambuf.data());

			aStreambuf = {};
			std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].normalTexture";
			ret &= mProgram->addUniform(aStreambuf.data());

			aStreambuf = {};
			std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].normalScale";
			ret &= mProgram->addUniform(aStreambuf.data());
		}
		ret &= mProgram->addUniform("uSMaterial.splatmapTexture");

		return ret;
	}

}
