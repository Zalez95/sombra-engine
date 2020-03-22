#include <fstream>
#include "se/graphics/3D/ProgramPBR.h"
#include "se/graphics/3D/Material.h"
#include "se/graphics/core/Shader.h"
#include "se/graphics/core/Program.h"
#include "../core/GLWrapper.h"

namespace se::graphics {

	void ProgramPBR::setMaterial(const Material& material)
	{
		// Set the material alphaMode
		if (material.alphaMode == AlphaMode::Blend) {
			GL_WRAP( glEnable(GL_BLEND) );
			GL_WRAP( glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) );
			GL_WRAP( glDisable(GL_DEPTH_TEST) );
		}

		// Unset face culling
		if (material.doubleSided) {
			GL_WRAP( glDisable(GL_CULL_FACE) );
		}

		// Set uniforms
		mProgram->setUniform("uMaterial.pbrMetallicRoughness.baseColorFactor", material.pbrMetallicRoughness.baseColorFactor);

		bool useBaseColorTexture = (material.pbrMetallicRoughness.baseColorTexture != nullptr);
		mProgram->setUniform("uMaterial.pbrMetallicRoughness.useBaseColorTexture", useBaseColorTexture);
		if (useBaseColorTexture) {
			mProgram->setUniform("uMaterial.pbrMetallicRoughness.baseColorTexture", TextureUnits::kBaseColor);
			material.pbrMetallicRoughness.baseColorTexture->bind(TextureUnits::kBaseColor);
		}

		mProgram->setUniform("uMaterial.pbrMetallicRoughness.metallicFactor", material.pbrMetallicRoughness.metallicFactor);
		mProgram->setUniform("uMaterial.pbrMetallicRoughness.roughnessFactor", material.pbrMetallicRoughness.roughnessFactor);

		bool useMetallicRoughnessTexture = (material.pbrMetallicRoughness.metallicRoughnessTexture != nullptr);
		mProgram->setUniform("uMaterial.pbrMetallicRoughness.useMetallicRoughnessTexture", useMetallicRoughnessTexture);
		if (useMetallicRoughnessTexture) {
			mProgram->setUniform("uMaterial.pbrMetallicRoughness.metallicRoughnessTexture", TextureUnits::kMetallicRoughness);
			material.pbrMetallicRoughness.metallicRoughnessTexture->bind(TextureUnits::kMetallicRoughness);
		}

		bool useNormalTexture = (material.normalTexture != nullptr);
		mProgram->setUniform("uMaterial.useNormalTexture", useNormalTexture);
		if (useNormalTexture) {
			mProgram->setUniform("uMaterial.normalTexture", TextureUnits::kNormal);
			material.normalTexture->bind(TextureUnits::kNormal);
			mProgram->setUniform("uMaterial.normalScale", material.normalScale);
		}

		bool useOcclusionTexture = (material.occlusionTexture != nullptr);
		mProgram->setUniform("uMaterial.useOcclusionTexture", useOcclusionTexture);
		if (useOcclusionTexture) {
			mProgram->setUniform("uMaterial.occlusionTexture", TextureUnits::kOcclusion);
			material.occlusionTexture->bind(TextureUnits::kOcclusion);
			mProgram->setUniform("uMaterial.occlusionStrength", material.occlusionStrength);
		}

		bool useEmissiveTexture = (material.emissiveTexture != nullptr);
		mProgram->setUniform("uMaterial.useEmissiveTexture", useEmissiveTexture);
		if (useEmissiveTexture) {
			mProgram->setUniform("uMaterial.emissiveTexture", TextureUnits::kEmissive);
			material.emissiveTexture->bind(TextureUnits::kEmissive);
		}

		mProgram->setUniform("uMaterial.emissiveFactor", material.emissiveFactor);

		bool checkAlphaCutoff = (material.alphaMode == AlphaMode::Mask);
		mProgram->setUniform("uMaterial.checkAlphaCutoff", checkAlphaCutoff);
		if (checkAlphaCutoff) {
			mProgram->setUniform("uMaterial.alphaCutoff", material.alphaCutoff);
		}
	}


	void ProgramPBR::unsetMaterial(const Material& material)
	{
		// Set face culling
		if (material.doubleSided) {
			GL_WRAP( glEnable(GL_CULL_FACE) );
		}

		// Set the material alphaMode
		if (material.alphaMode == AlphaMode::Blend) {
			GL_WRAP( glEnable(GL_DEPTH_TEST) );
			GL_WRAP( glDisable(GL_BLEND) );
		}
	}

// Private functions
	bool ProgramPBR::createProgram()
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


	bool ProgramPBR::addUniforms()
	{
		bool ret = ProgramLight::addUniforms();

		ret &= mProgram->addUniform("uMaterial.pbrMetallicRoughness.baseColorFactor");
		ret &= mProgram->addUniform("uMaterial.pbrMetallicRoughness.useBaseColorTexture");
		ret &= mProgram->addUniform("uMaterial.pbrMetallicRoughness.baseColorTexture");
		ret &= mProgram->addUniform("uMaterial.pbrMetallicRoughness.metallicFactor");
		ret &= mProgram->addUniform("uMaterial.pbrMetallicRoughness.roughnessFactor");
		ret &= mProgram->addUniform("uMaterial.pbrMetallicRoughness.useMetallicRoughnessTexture");
		ret &= mProgram->addUniform("uMaterial.pbrMetallicRoughness.metallicRoughnessTexture");
		ret &= mProgram->addUniform("uMaterial.useNormalTexture");
		ret &= mProgram->addUniform("uMaterial.normalTexture");
		ret &= mProgram->addUniform("uMaterial.normalScale");
		ret &= mProgram->addUniform("uMaterial.useOcclusionTexture");
		ret &= mProgram->addUniform("uMaterial.occlusionTexture");
		ret &= mProgram->addUniform("uMaterial.occlusionStrength");
		ret &= mProgram->addUniform("uMaterial.useEmissiveTexture");
		ret &= mProgram->addUniform("uMaterial.emissiveTexture");
		ret &= mProgram->addUniform("uMaterial.emissiveFactor");
		ret &= mProgram->addUniform("uMaterial.checkAlphaCutoff");
		ret &= mProgram->addUniform("uMaterial.alphaCutoff");

		return ret;
	}

}
