#include <array>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <GL/glew.h>
#include "se/graphics/3D/Program3D.h"
#include "se/graphics/Shader.h"
#include "se/graphics/Program.h"
#include "se/graphics/3D/Lights.h"
#include "se/graphics/3D/Material.h"

namespace se::graphics {

	bool Program3D::init()
	{
		if (!initShaders("res/shaders/vertex3D.glsl", "res/shaders/fragment3D.glsl")) {
			return false;
		}

		initUniformLocations();

		return true;
	}


	void Program3D::end()
	{
		if (mProgram) {
			delete mProgram;
		}
	}


	void Program3D::enable() const
	{
		mProgram->enable();
	}


	void Program3D::disable() const
	{
		mProgram->disable();
	}


	void Program3D::setModelMatrix(const glm::mat4& modelMatrix) const
	{
		mProgram->setUniform(mUniformLocations.modelMatrix, modelMatrix);
	}


	void Program3D::setViewMatrix(const glm::mat4& viewMatrix) const
	{
		mProgram->setUniform(mUniformLocations.viewMatrix, viewMatrix);
	}


	void Program3D::setProjectionMatrix(const glm::mat4& projectionMatrix) const
	{
		mProgram->setUniform(mUniformLocations.projectionMatrix, projectionMatrix);
	}


	void Program3D::setMaterial(const Material& material) const
	{
		mProgram->setUniform(mUniformLocations.material.pbrMetallicRoughness.baseColorFactor, material.pbrMetallicRoughness.baseColorFactor);

		int useBaseColorTexture = (material.pbrMetallicRoughness.baseColorTexture != nullptr);
		mProgram->setUniform(mUniformLocations.material.pbrMetallicRoughness.useBaseColorTexture, useBaseColorTexture);
		if (useBaseColorTexture) {
			mProgram->setUniform(mUniformLocations.material.pbrMetallicRoughness.baseColorTexture, TextureUnits::kBaseColor);
			material.pbrMetallicRoughness.baseColorTexture->bind(TextureUnits::kBaseColor);
		}

		mProgram->setUniform(mUniformLocations.material.pbrMetallicRoughness.metallicFactor, material.pbrMetallicRoughness.metallicFactor);
		mProgram->setUniform(mUniformLocations.material.pbrMetallicRoughness.roughnessFactor, material.pbrMetallicRoughness.roughnessFactor);

		int useMetallicRoughnessTexture = (material.pbrMetallicRoughness.metallicRoughnessTexture != nullptr);
		mProgram->setUniform(mUniformLocations.material.pbrMetallicRoughness.useMetallicRoughnessTexture, useMetallicRoughnessTexture);
		if (useMetallicRoughnessTexture) {
			mProgram->setUniform(mUniformLocations.material.pbrMetallicRoughness.metallicRoughnessTexture, TextureUnits::kMetallicRoughness);
			material.pbrMetallicRoughness.metallicRoughnessTexture->bind(TextureUnits::kMetallicRoughness);
		}

		int useNormalTexture = (material.normalTexture != nullptr);
		mProgram->setUniform(mUniformLocations.material.useNormalTexture, useNormalTexture);
		if (useNormalTexture) {
			mProgram->setUniform(mUniformLocations.material.normalTexture, TextureUnits::kNormal);
			material.normalTexture->bind(TextureUnits::kNormal);
		}

		int useOcclusionTexture = (material.occlusionTexture != nullptr);
		mProgram->setUniform(mUniformLocations.material.useOcclusionTexture, useOcclusionTexture);
		if (useOcclusionTexture) {
			mProgram->setUniform(mUniformLocations.material.occlusionTexture, TextureUnits::kOcclusion);
			material.occlusionTexture->bind(TextureUnits::kOcclusion);
		}

		int useEmissiveTexture = (material.emissiveTexture != nullptr);
		mProgram->setUniform(mUniformLocations.material.useEmissiveTexture, useEmissiveTexture);
		if (useEmissiveTexture) {
			mProgram->setUniform(mUniformLocations.material.emissiveTexture, TextureUnits::kEmissive);
			material.emissiveTexture->bind(TextureUnits::kEmissive);
		}

		mProgram->setUniform(mUniformLocations.material.emissiveFactor, material.emissiveFactor);

		int checkAlphaCutoff = (material.alphaMode == AlphaMode::Mask);
		mProgram->setUniform(mUniformLocations.material.checkAlphaCutoff, checkAlphaCutoff);
		if (checkAlphaCutoff) {
			mProgram->setUniform(mUniformLocations.material.alphaCutoff, material.alphaCutoff);
		}
	}


	void Program3D::setLights(const std::vector<const PointLight*>& pointLights) const
	{
		int numPointLights = std::min(static_cast<int>(pointLights.size()), kMaxPointLights);

		mProgram->setUniform(mUniformLocations.numPointLights, numPointLights);

		std::array<glm::vec3, kMaxPointLights> positions;
		for (int i = 0; i < numPointLights; ++i) {
			const PointLight& pLight = *pointLights[i];
			mProgram->setUniform(mUniformLocations.pointLights[i].baseLight.lightColor, pLight.base.lightColor);
			mProgram->setUniform(mUniformLocations.pointLights[i].attenuation.constant, pLight.attenuation.constant);
			mProgram->setUniform(mUniformLocations.pointLights[i].attenuation.linear, pLight.attenuation.linear);
			mProgram->setUniform(mUniformLocations.pointLights[i].attenuation.exponential, pLight.attenuation.exponential);
			positions[i] = pLight.position;
		}

		mProgram->setUniformV(mUniformLocations.pointLightsPositions, numPointLights, positions.data());
	}

// Private functions
	bool Program3D::initShaders(const char* vertexShaderPath, const char* fragmentShaderPath)
	{
		// 1. Read the shader text from the shader files
		std::ifstream reader;

		std::string vertexShaderText;
		std::stringstream vertexShaderStream;
		reader.open(vertexShaderPath);
		if (reader.good()) {
			vertexShaderStream << reader.rdbuf();
			vertexShaderText = vertexShaderStream.str();
			reader.close();
		}
		else {
			reader.close();
			return false;
		}

		std::string fragmentShaderText;
		std::stringstream fragmentShaderStream;
		reader.open(fragmentShaderPath);
		if (reader.good()) {
			fragmentShaderStream << reader.rdbuf();
			fragmentShaderText = fragmentShaderStream.str();
			reader.close();
		}
		else {
			reader.close();
			return false;
		}

		Shader vertexShader(vertexShaderText.c_str(), ShaderType::Vertex);
		Shader fragmentShader(fragmentShaderText.c_str(), ShaderType::Fragment);

		// 2. Create the Program
		const Shader* shaders[] = { &vertexShader, &fragmentShader };
		mProgram = new Program(shaders, 2);

		return true;
	}


	void Program3D::initUniformLocations()
	{
		mUniformLocations.modelMatrix				= mProgram->getUniformLocation("uModelMatrix");
		mUniformLocations.viewMatrix				= mProgram->getUniformLocation("uViewMatrix");
		mUniformLocations.projectionMatrix			= mProgram->getUniformLocation("uProjectionMatrix");

		mUniformLocations.material.pbrMetallicRoughness.baseColorFactor				= mProgram->getUniformLocation("uMaterial.pbrMetallicRoughness.baseColorFactor");
		mUniformLocations.material.pbrMetallicRoughness.useBaseColorTexture			= mProgram->getUniformLocation("uMaterial.pbrMetallicRoughness.useBaseColorTexture");
		mUniformLocations.material.pbrMetallicRoughness.baseColorTexture			= mProgram->getUniformLocation("uMaterial.pbrMetallicRoughness.baseColorTexture");
		mUniformLocations.material.pbrMetallicRoughness.metallicFactor				= mProgram->getUniformLocation("uMaterial.pbrMetallicRoughness.metallicFactor");
		mUniformLocations.material.pbrMetallicRoughness.roughnessFactor				= mProgram->getUniformLocation("uMaterial.pbrMetallicRoughness.roughnessFactor");
		mUniformLocations.material.pbrMetallicRoughness.useMetallicRoughnessTexture	= mProgram->getUniformLocation("uMaterial.pbrMetallicRoughness.useMetallicRoughnessTexture");
		mUniformLocations.material.pbrMetallicRoughness.metallicRoughnessTexture	= mProgram->getUniformLocation("uMaterial.pbrMetallicRoughness.metallicRoughnessTexture");
		mUniformLocations.material.useNormalTexture		= mProgram->getUniformLocation("uMaterial.useNormalTexture");
		mUniformLocations.material.normalTexture		= mProgram->getUniformLocation("uMaterial.normalTexture");
		mUniformLocations.material.useOcclusionTexture	= mProgram->getUniformLocation("uMaterial.useOcclusionTexture");
		mUniformLocations.material.occlusionTexture		= mProgram->getUniformLocation("uMaterial.occlusionTexture");
		mUniformLocations.material.useEmissiveTexture	= mProgram->getUniformLocation("uMaterial.useEmissiveTexture");
		mUniformLocations.material.emissiveTexture		= mProgram->getUniformLocation("uMaterial.emissiveTexture");
		mUniformLocations.material.emissiveFactor		= mProgram->getUniformLocation("uMaterial.emissiveFactor");
		mUniformLocations.material.checkAlphaCutoff		= mProgram->getUniformLocation("uMaterial.checkAlphaCutoff");
		mUniformLocations.material.alphaCutoff			= mProgram->getUniformLocation("uMaterial.alphaCutoff");

		mUniformLocations.numPointLights = mProgram->getUniformLocation("uNumPointLights");
		for (int i = 0; i < kMaxPointLights; ++i) {
			mUniformLocations.pointLights[i].baseLight.lightColor = mProgram->getUniformLocation(
				("uPointLights[" + std::to_string(i) + "].baseLight.lightColor").c_str()
			);
			mUniformLocations.pointLights[i].attenuation.constant = mProgram->getUniformLocation(
				("uPointLights[" + std::to_string(i) + "].attenuation.constant").c_str()
			);
			mUniformLocations.pointLights[i].attenuation.linear = mProgram->getUniformLocation(
				("uPointLights[" + std::to_string(i) + "].attenuation.linear").c_str()
			);
			mUniformLocations.pointLights[i].attenuation.exponential = mProgram->getUniformLocation(
				("uPointLights[" + std::to_string(i) + "].attenuation.exponential").c_str()
			);
		}
		mUniformLocations.pointLightsPositions = mProgram->getUniformLocation("uPointLightsPositions");
	}

}
