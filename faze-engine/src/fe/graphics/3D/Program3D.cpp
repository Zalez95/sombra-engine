#include <string>
#include <sstream>
#include <fstream>
#include <GL/glew.h>
#include "fe/graphics/3D/Program3D.h"
#include "fe/graphics/Shader.h"
#include "fe/graphics/Program.h"
#include "fe/graphics/3D/Lights.h"
#include "fe/graphics/3D/Material.h"

namespace fe { namespace graphics {

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


	void Program3D::setColorTexture(int unit)
	{
		mProgram->setUniform(mUniformLocations.colorTexture, unit);
	}


	void Program3D::setMaterial(const Material* material)
	{
		mProgram->setUniform(mUniformLocations.material.ambientColor, material->getAmbientColor());
		mProgram->setUniform(mUniformLocations.material.diffuseColor, material->getDiffuseColor());
		mProgram->setUniform(mUniformLocations.material.specularColor, material->getSpecularColor());
		mProgram->setUniform(mUniformLocations.material.shininess, material->getShininess());
	}


	void Program3D::setLights(const std::vector<const PointLight*>& pointLights)
	{
		int numPointLights = (static_cast<int>(pointLights.size()) > kMaxPointLights)? kMaxPointLights : static_cast<int>(pointLights.size());
		mProgram->setUniform(mUniformLocations.numPointLights, numPointLights);

		for (int i = 0; i < numPointLights; ++i) {
			BaseLight base		= pointLights[i]->getBaseLight();
			glm::vec3 position	= pointLights[i]->getPosition();
			Attenuation att		= pointLights[i]->getAttenuation();

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
		reader.open("res/shaders/3D.vert");
		vertexShaderStream << reader.rdbuf();
		vertexShaderText = vertexShaderStream.str();
		reader.close();

		std::string fragmentShaderText;
		std::stringstream fragmentShaderStream;
		reader.open("res/shaders/3D.frag");
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

		mUniformLocations.colorTexture				= mProgram->getUniformLocation("uColorTexture");

		mUniformLocations.material.ambientColor	= mProgram->getUniformLocation("uMaterial.ambientColor");
		mUniformLocations.material.diffuseColor	= mProgram->getUniformLocation("uMaterial.diffuseColor");
		mUniformLocations.material.specularColor	= mProgram->getUniformLocation("uMaterial.specularColor");
		mUniformLocations.material.shininess		= mProgram->getUniformLocation("uMaterial.shininess");

		mUniformLocations.numPointLights			= mProgram->getUniformLocation("uNumPointLights");
		for (size_t i = 0; i < kMaxPointLights; ++i) {
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

}}
