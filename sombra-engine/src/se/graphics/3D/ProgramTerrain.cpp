#include <array>
#include <sstream>
#include <fstream>
#include "se/graphics/3D/ProgramTerrain.h"
#include "se/graphics/core/Shader.h"
#include "se/graphics/core/Program.h"
#include "se/graphics/core/Texture.h"
#include "se/utils/Log.h"

namespace se::graphics {

	void ProgramTerrain::setXZSize(float xzSize) const
	{
		mProgram->setUniform("uXZSize", xzSize);
	}


	void ProgramTerrain::setMaxHeight(float maxHeight) const
	{
		mProgram->setUniform("uMaxHeight", maxHeight);
	}


	void ProgramTerrain::setHeightMap(const Texture& heightMap) const
	{
		mProgram->setUniform("uHeightMap", TextureUnits::kHeightMap);
		heightMap.bind(TextureUnits::kHeightMap);
	}

// Private functions
	bool ProgramTerrain::createProgram()
	{
		// 1. Read the shader text from the shader files
		std::string vertexShaderText;
		if (std::ifstream ifs("res/shaders/vertexTerrain.glsl"); ifs.good()) {
			vertexShaderText.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
		}
		else {
			return false;
		}

		std::string geometryShaderText;
		if (std::ifstream ifs("res/shaders/geometryTerrain.glsl"); ifs.good()) {
			geometryShaderText.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
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
			Shader geometryShader(geometryShaderText.c_str(), ShaderType::Geometry);
			Shader fragmentShader(fragmentShaderText.c_str(), ShaderType::Fragment);
			const Shader* shaders[] = { &vertexShader, &geometryShader, &fragmentShader };
			mProgram = new Program(shaders, 3);
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


	bool ProgramTerrain::addUniforms()
	{
		bool ret = ProgramPBR::addUniforms();

		ret &= mProgram->addUniform("uXZSize");
		ret &= mProgram->addUniform("uMaxHeight");
		ret &= mProgram->addUniform("uHeightMap");

		return ret;
	}

}
