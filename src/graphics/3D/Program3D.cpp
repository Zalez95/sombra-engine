#include "Program3D.h"
#include <string>
#include <sstream>
#include <fstream>
#include "../Shader.h"
#include "../Program.h"
#include "Lights.h"
#include "Material.h"

namespace graphics {

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
		mProgram->setUniform(mUniformLocations.mModelMatrix, modelMatrix);
	}


	void Program3D::setViewMatrix(const glm::mat4& viewMatrix)
	{
		mProgram->setUniform(mUniformLocations.mViewMatrix, viewMatrix);
	}


	void Program3D::setProjectionMatrix(const glm::mat4& projectionMatrix)
	{
		mProgram->setUniform(mUniformLocations.mProjectionMatrix, projectionMatrix);
	}


	void Program3D::setColorTexture(int unit)
	{
		mProgram->setUniform(mUniformLocations.mColorTexture, unit);
	}


	void Program3D::setMaterial(const Material* material)
	{
		mProgram->setUniform(mUniformLocations.mMaterial.mAmbientColor, material->getAmbientColor());
		mProgram->setUniform(mUniformLocations.mMaterial.mDiffuseColor, material->getDiffuseColor());
		mProgram->setUniform(mUniformLocations.mMaterial.mSpecularColor, material->getSpecularColor());
		mProgram->setUniform(mUniformLocations.mMaterial.mShininess, material->getShininess());
	}


	void Program3D::setLights(const std::vector<const PointLight*>& pointLights)
	{
		int numPointLights = (pointLights.size() > MAX_POINT_LIGHTS) ? static_cast<int>(MAX_POINT_LIGHTS) : static_cast<int>(pointLights.size());
		mProgram->setUniform(mUniformLocations.mNumPointLights, numPointLights);

		for (int i = 0; i < numPointLights; ++i) {
			BaseLight base		= pointLights[i]->getBaseLight();
			glm::vec3 position	= pointLights[i]->getPosition();
			Attenuation att		= pointLights[i]->getAttenuation();

			mProgram->setUniform(mUniformLocations.mPointLights[i].mBaseLight.mDiffuseColor, base.getDiffuseColor());
			mProgram->setUniform(mUniformLocations.mPointLights[i].mBaseLight.mSpecularColor, base.getSpecularColor());
			mProgram->setUniform(mUniformLocations.mPointLights[i].mAttenuation.mConstant, att.mConstant);
			mProgram->setUniform(mUniformLocations.mPointLights[i].mAttenuation.mLinear, att.mLinear);
			mProgram->setUniform(mUniformLocations.mPointLights[i].mAttenuation.mExponential, att.mExponential);
			mProgram->setUniform(mUniformLocations.mPointLightsPositions[i], position);
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
		mUniformLocations.mModelMatrix				= mProgram->getUniformLocation("uModelMatrix");
		mUniformLocations.mViewMatrix				= mProgram->getUniformLocation("uViewMatrix");
		mUniformLocations.mProjectionMatrix			= mProgram->getUniformLocation("uProjectionMatrix");
		
		mUniformLocations.mColorTexture				= mProgram->getUniformLocation("uColorTexture");

		mUniformLocations.mMaterial.mAmbientColor	= mProgram->getUniformLocation("uMaterial.mAmbientColor");
		mUniformLocations.mMaterial.mDiffuseColor	= mProgram->getUniformLocation("uMaterial.mDiffuseColor");
		mUniformLocations.mMaterial.mSpecularColor	= mProgram->getUniformLocation("uMaterial.mSpecularColor");
		mUniformLocations.mMaterial.mShininess		= mProgram->getUniformLocation("uMaterial.mShininess");
		
		mUniformLocations.mNumPointLights			= mProgram->getUniformLocation("uNumPointLights");
		for (size_t i = 0; i < MAX_POINT_LIGHTS; ++i) {
			mUniformLocations.mPointLights[i].mBaseLight.mDiffuseColor = mProgram->getUniformLocation(
				("uPointLights[" + std::to_string(i) + "].mBaseLight.mDiffuseColor").c_str()
			);
			mUniformLocations.mPointLights[i].mBaseLight.mSpecularColor = mProgram->getUniformLocation(
				("uPointLights[" + std::to_string(i) + "].mBaseLight.mSpecularColor").c_str()
			);
			mUniformLocations.mPointLights[i].mAttenuation.mConstant = mProgram->getUniformLocation(
				("uPointLights[" + std::to_string(i) + "].mAttenuation.mConstant").c_str()
			);
			mUniformLocations.mPointLights[i].mAttenuation.mLinear = mProgram->getUniformLocation(
				("uPointLights[" + std::to_string(i) + "].mAttenuation.mLinear").c_str()
			);
			mUniformLocations.mPointLights[i].mAttenuation.mExponential = mProgram->getUniformLocation(
				("uPointLights[" + std::to_string(i) + "].mAttenuation.mExponential").c_str()
			);
			mUniformLocations.mPointLightsPositions[i] = mProgram->getUniformLocation(
				("uPointLightsPositions[" + std::to_string(i) + "]").c_str()
			);
		}
	}

}
