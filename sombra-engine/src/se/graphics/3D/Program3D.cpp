#include <string>
#include <sstream>
#include <fstream>
#include <GL/glew.h>
#include "se/graphics/3D/Program3D.h"
#include "se/graphics/Shader.h"
#include "se/graphics/Program.h"
#include "se/graphics/3D/Lights.h"
#include "se/graphics/3D/Material.h"

namespace se::graphics {

	Program3D::Program3D()
	{
		initShaders();
		initUniformLocations();
	}


	Program3D::~Program3D()
	{
		delete mProgram;
	}


	void Program3D::enable() const
	{
		mProgram->enable();
	}


	void Program3D::disable() const
	{
		mProgram->disable();
	}


	void Program3D::setModelMatrix(const glm::mat4& modelMatrix)
	{
		mProgram->setUniform(mUniformLocations.modelMatrix, modelMatrix);
	}


	void Program3D::setViewMatrix(const glm::mat4& viewMatrix)
	{
		mProgram->setUniform(mUniformLocations.viewMatrix, viewMatrix);
	}


	void Program3D::setProjectionMatrix(const glm::mat4& projectionMatrix)
	{
		mProgram->setUniform(mUniformLocations.projectionMatrix, projectionMatrix);
	}


	void Program3D::setMaterial(const Material& material)
	{
		mProgram->setUniform(mUniformLocations.material.pbrMetallicRoughness.baseColorFactor, material.pbrMetallicRoughness.baseColorFactor);
		if (material.pbrMetallicRoughness.baseColorTexture) {
			mProgram->setUniform(mUniformLocations.material.pbrMetallicRoughness.baseColorTexture, TextureUnits::kBaseColor);
			material.pbrMetallicRoughness.baseColorTexture->bind(TextureUnits::kBaseColor);
		}
		mProgram->setUniform(mUniformLocations.material.pbrMetallicRoughness.metallicFactor, material.pbrMetallicRoughness.metallicFactor);
		mProgram->setUniform(mUniformLocations.material.pbrMetallicRoughness.roughnessFactor, material.pbrMetallicRoughness.roughnessFactor);
		if (material.pbrMetallicRoughness.metallicRoughnessTexture) {
			mProgram->setUniform(mUniformLocations.material.pbrMetallicRoughness.metallicRoughnessTexture, TextureUnits::kMetallicRoughness);
			material.pbrMetallicRoughness.metallicRoughnessTexture->bind(TextureUnits::kMetallicRoughness);
		}
		if (material.normalTexture) {
			mProgram->setUniform(mUniformLocations.material.normalTexture, TextureUnits::kNormal);
			material.normalTexture->bind(TextureUnits::kNormal);
		}
		if (material.occlusionTexture) {
			mProgram->setUniform(mUniformLocations.material.occlusionTexture, TextureUnits::kOcclusion);
			material.occlusionTexture->bind(TextureUnits::kOcclusion);
		}
		if (material.emissiveTexture) {
			mProgram->setUniform(mUniformLocations.material.emissiveTexture, TextureUnits::kEmissive);
			material.occlusionTexture->bind(TextureUnits::kEmissive);
		}
		mProgram->setUniform(mUniformLocations.material.emissiveFactor, material.emissiveFactor);
	}


	void Program3D::setLights(const std::vector<const PointLight*>& pointLights)
	{
		int numPointLights = (static_cast<int>(pointLights.size()) > kMaxPointLights)? kMaxPointLights : static_cast<int>(pointLights.size());
		mProgram->setUniform(mUniformLocations.numPointLights, numPointLights);

		for (int i = 0; i < numPointLights; ++i) {
			const BaseLight& base		= pointLights[i]->getBaseLight();
			const Attenuation& att		= pointLights[i]->getAttenuation();
			const glm::vec3& position	= pointLights[i]->getPosition();

			mProgram->setUniform(mUniformLocations.pointLights[i].baseLight.diffuseColor, base.getDiffuseColor());
			mProgram->setUniform(mUniformLocations.pointLights[i].baseLight.specularColor, base.getSpecularColor());
			mProgram->setUniform(mUniformLocations.pointLights[i].attenuation.constant, att.constant);
			mProgram->setUniform(mUniformLocations.pointLights[i].attenuation.linear, att.linear);
			mProgram->setUniform(mUniformLocations.pointLights[i].attenuation.exponential, att.exponential);
			mProgram->setUniform(mUniformLocations.pointLightsPositions[i], position);
		}
	}

// Private functions
	void Program3D::initShaders()
	{
		// 1. Read the shader text from the shader files
		std::ifstream reader;

		std::string vertexShaderText;
		std::stringstream vertexShaderStream;
		reader.open("res/shaders/vertex3D.glsl");
		vertexShaderStream << reader.rdbuf();
		vertexShaderText = vertexShaderStream.str();
		reader.close();

		std::string fragmentShaderText;
		std::stringstream fragmentShaderStream;
		reader.open("res/shaders/fragment3D.glsl");
		fragmentShaderStream << reader.rdbuf();
		fragmentShaderText = fragmentShaderStream.str();
		reader.close();

		Shader vertexShader(vertexShaderText.c_str(), GL_VERTEX_SHADER);
		Shader fragmentShader(fragmentShaderText.c_str(), GL_FRAGMENT_SHADER);

		// 2. Create the Program
		std::vector<const Shader*> shaders = { &vertexShader, &fragmentShader };
		mProgram = new Program(shaders);
	}


	void Program3D::initUniformLocations()
	{
		mUniformLocations.modelMatrix				= mProgram->getUniformLocation("uModelMatrix");
		mUniformLocations.viewMatrix				= mProgram->getUniformLocation("uViewMatrix");
		mUniformLocations.projectionMatrix			= mProgram->getUniformLocation("uProjectionMatrix");

		mUniformLocations.material.pbrMetallicRoughness.baseColorFactor = mProgram->getUniformLocation("uMaterial.pbrMetallicRoughness.baseColorFactor");
		mUniformLocations.material.pbrMetallicRoughness.baseColorTexture = mProgram->getUniformLocation("uMaterial.pbrMetallicRoughness.baseColorTexture");
		mUniformLocations.material.pbrMetallicRoughness.metallicFactor = mProgram->getUniformLocation("uMaterial.pbrMetallicRoughness.metallicFactor");
		mUniformLocations.material.pbrMetallicRoughness.roughnessFactor = mProgram->getUniformLocation("uMaterial.pbrMetallicRoughness.roughnessFactor");
		mUniformLocations.material.pbrMetallicRoughness.metallicRoughnessTexture = mProgram->getUniformLocation("uMaterial.pbrMetallicRoughness.metallicRoughnessTexture");
		mUniformLocations.material.normalTexture	= mProgram->getUniformLocation("uMaterial.normalTexture");
		mUniformLocations.material.occlusionTexture	= mProgram->getUniformLocation("uMaterial.occlusionTexture");
		mUniformLocations.material.emissiveTexture	= mProgram->getUniformLocation("uMaterial.emissiveTexture");
		mUniformLocations.material.emissiveFactor	= mProgram->getUniformLocation("uMaterial.emissiveFactor");

		mUniformLocations.numPointLights			= mProgram->getUniformLocation("uNumPointLights");
		for (std::size_t i = 0; i < kMaxPointLights; ++i) {
			mUniformLocations.pointLights[i].baseLight.diffuseColor = mProgram->getUniformLocation(
				("uPointLights[" + std::to_string(i) + "].baseLight.diffuseColor").c_str()
			);
			mUniformLocations.pointLights[i].baseLight.specularColor = mProgram->getUniformLocation(
				("uPointLights[" + std::to_string(i) + "].baseLight.specularColor").c_str()
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
			mUniformLocations.pointLightsPositions[i] = mProgram->getUniformLocation(
				("uPointLightsPositions[" + std::to_string(i) + "]").c_str()
			);
		}
	}

}
