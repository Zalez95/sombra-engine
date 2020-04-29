#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "se/app/loaders/TechniqueLoader.h"
#include "se/utils/Log.h"
#include "se/utils/FixedVector.h"
#include "se/graphics/core/Shader.h"
#include "se/graphics/core/UniformVariable.h"
#include "se/graphics/core/GraphicsOperations.h"

namespace se::app {

	void TechniqueLoader::addMaterialBindables(StepSPtr step, const Material& material, const ProgramSPtr program)
	{
		// Set the material alphaMode
		step->addBindable(std::make_shared<graphics::BlendingOperation>(material.alphaMode == graphics::AlphaMode::Blend))
			.addBindable(std::make_shared<graphics::DepthTestOperation>(material.alphaMode != graphics::AlphaMode::Blend));

		// Unset face culling
		step->addBindable(std::make_shared<graphics::CullingOperation>(!material.doubleSided));

		// Set uniforms
		step->addBindable(std::make_shared<graphics::UniformVariableValue<glm::vec4>>(
			"uMaterial.pbrMetallicRoughness.baseColorFactor", *program, material.pbrMetallicRoughness.baseColorFactor
		));

		bool useBaseColorTexture = material.pbrMetallicRoughness.baseColorTexture != nullptr;
		step->addBindable(std::make_shared<graphics::UniformVariableValue<bool>>(
			"uMaterial.pbrMetallicRoughness.useBaseColorTexture", *program, useBaseColorTexture
		));
		if (useBaseColorTexture) {
			material.pbrMetallicRoughness.baseColorTexture->setTextureUnit(Material::TextureUnits::kBaseColor);
			step->addBindable(material.pbrMetallicRoughness.baseColorTexture)
				.addBindable(std::make_shared<graphics::UniformVariableValue<int>>(
					"uMaterial.pbrMetallicRoughness.baseColorTexture", *program, Material::TextureUnits::kBaseColor
				));
		}

		step->addBindable(std::make_shared<graphics::UniformVariableValue<float>>(
				"uMaterial.pbrMetallicRoughness.metallicFactor", *program, material.pbrMetallicRoughness.metallicFactor
			))
			.addBindable(std::make_shared<graphics::UniformVariableValue<float>>(
				"uMaterial.pbrMetallicRoughness.roughnessFactor", *program, material.pbrMetallicRoughness.roughnessFactor
			));

		bool useMetallicRoughnessTexture = material.pbrMetallicRoughness.metallicRoughnessTexture != nullptr;
		step->addBindable(std::make_shared<graphics::UniformVariableValue<bool>>(
			"uMaterial.pbrMetallicRoughness.useMetallicRoughnessTexture", *program, useMetallicRoughnessTexture
		));
		if (useMetallicRoughnessTexture) {
			material.pbrMetallicRoughness.metallicRoughnessTexture->setTextureUnit(Material::TextureUnits::kMetallicRoughness);
			step->addBindable(material.pbrMetallicRoughness.metallicRoughnessTexture)
				.addBindable(std::make_shared<graphics::UniformVariableValue<int>>(
					"uMaterial.pbrMetallicRoughness.metallicRoughnessTexture", *program, Material::TextureUnits::kMetallicRoughness
				));
		}

		bool useNormalTexture = material.normalTexture != nullptr;
		step->addBindable(std::make_shared<graphics::UniformVariableValue<bool>>(
			"uMaterial.useNormalTexture", *program, useNormalTexture
		));
		if (useNormalTexture) {
			material.normalTexture->setTextureUnit(Material::TextureUnits::kNormal);
			step->addBindable(material.normalTexture)
				.addBindable(std::make_shared<graphics::UniformVariableValue<int>>(
					"uMaterial.normalTexture", *program, Material::TextureUnits::kNormal
				))
				.addBindable(std::make_shared<graphics::UniformVariableValue<float>>(
					"uMaterial.normalScale", *program, material.normalScale
				));
		}

		bool useOcclusionTexture = material.occlusionTexture != nullptr;
		step->addBindable(std::make_shared<graphics::UniformVariableValue<bool>>(
			"uMaterial.useOcclusionTexture", *program, useOcclusionTexture
		));
		if (useOcclusionTexture) {
			material.occlusionTexture->setTextureUnit(Material::TextureUnits::kOcclusion);
			step->addBindable(material.occlusionTexture)
				.addBindable(std::make_shared<graphics::UniformVariableValue<int>>(
					"uMaterial.occlusionTexture", *program, Material::TextureUnits::kOcclusion
				))
				.addBindable(std::make_shared<graphics::UniformVariableValue<float>>(
					"uMaterial.occlusionStrength", *program, material.occlusionStrength
				));
		}

		bool useEmissiveTexture = material.emissiveTexture != nullptr;
		step->addBindable(std::make_shared<graphics::UniformVariableValue<bool>>(
			"uMaterial.useEmissiveTexture", *program, useEmissiveTexture
		));
		if (useEmissiveTexture) {
			material.emissiveTexture->setTextureUnit(Material::TextureUnits::kEmissive);
			step->addBindable(material.emissiveTexture)
				.addBindable(std::make_shared<graphics::UniformVariableValue<int>>(
					"uMaterial.emissiveTexture", *program, Material::TextureUnits::kEmissive
				));
		}

		step->addBindable(std::make_shared<graphics::UniformVariableValue<glm::vec3>>(
			"uMaterial.emissiveFactor", *program, material.emissiveFactor
		));

		bool checkAlphaCutoff = (material.alphaMode == graphics::AlphaMode::Mask);
		step->addBindable(std::make_shared<graphics::UniformVariableValue<bool>>(
			"uMaterial.checkAlphaCutoff", *program, checkAlphaCutoff
		));
		if (checkAlphaCutoff) {
			step->addBindable(std::make_shared<graphics::UniformVariableValue<float>>(
				"uMaterial.alphaCutoff", *program, material.alphaCutoff
			));
		}
	}


	void TechniqueLoader::addSplatmapMaterialBindables(StepSPtr step, const SplatmapMaterial& material, const ProgramSPtr program)
	{
		// Set the material alphaMode
		step->addBindable(std::make_shared<graphics::BlendingOperation>(false))
			.addBindable(std::make_shared<graphics::DepthTestOperation>(true));

		// Set face culling
		step->addBindable(std::make_shared<graphics::CullingOperation>(true));

		// Set uniforms
		int numMaterials = static_cast<int>(material.materials.size());
		step->addBindable(std::make_shared<graphics::UniformVariableValue<int>>(
			"uSMaterial.numMaterials", *program, numMaterials
		));

		for (int i = 0; i < numMaterials; ++i) {
			utils::ArrayStreambuf<char, 128> aStreambuf;
			std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].pbrMetallicRoughness.baseColorFactor";
			step->addBindable(std::make_shared<graphics::UniformVariableValue<glm::vec4>>(
				aStreambuf.data(), *program, material.materials[i].pbrMetallicRoughness.baseColorFactor
			));

			aStreambuf = {};
			std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].pbrMetallicRoughness.useBaseColorTexture";
			bool useBaseColorTexture = material.materials[i].pbrMetallicRoughness.baseColorTexture != nullptr;
			step->addBindable(std::make_shared<graphics::UniformVariableValue<bool>>(
				aStreambuf.data(), *program, useBaseColorTexture
			));
			if (useBaseColorTexture) {
				aStreambuf = {};
				std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].pbrMetallicRoughness.baseColorTexture";

				material.materials[i].pbrMetallicRoughness.baseColorTexture->setTextureUnit(
					SplatmapMaterial::TextureUnits::kBaseColor0 + BasicMaterial::kMaxTextures * i
				);
				step->addBindable(material.materials[i].pbrMetallicRoughness.baseColorTexture)
					.addBindable(std::make_shared<graphics::UniformVariableValue<int>>(
						aStreambuf.data(), *program, SplatmapMaterial::TextureUnits::kBaseColor0 + BasicMaterial::kMaxTextures * i
					));
			}

			aStreambuf = {};
			std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].pbrMetallicRoughness.metallicFactor";
			step->addBindable(std::make_shared<graphics::UniformVariableValue<float>>(
				aStreambuf.data(), *program, material.materials[i].pbrMetallicRoughness.metallicFactor
			));

			aStreambuf = {};
			std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].pbrMetallicRoughness.roughnessFactor";
			step->addBindable(std::make_shared<graphics::UniformVariableValue<float>>(
				aStreambuf.data(), *program, material.materials[i].pbrMetallicRoughness.roughnessFactor
			));

			aStreambuf = {};
			std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].pbrMetallicRoughness.useMetallicRoughnessTexture";
			bool useMetallicRoughnessTexture = material.materials[i].pbrMetallicRoughness.metallicRoughnessTexture != nullptr;
			step->addBindable(std::make_shared<graphics::UniformVariableValue<bool>>(
				aStreambuf.data(), *program, useMetallicRoughnessTexture
			));
			if (useMetallicRoughnessTexture) {
				aStreambuf = {};
				std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].pbrMetallicRoughness.metallicRoughnessTexture";

				material.materials[i].pbrMetallicRoughness.metallicRoughnessTexture->setTextureUnit(
					SplatmapMaterial::TextureUnits::kMetallicRoughness0 + BasicMaterial::kMaxTextures * i
				);
				step->addBindable(material.materials[i].pbrMetallicRoughness.metallicRoughnessTexture)
					.addBindable(std::make_shared<graphics::UniformVariableValue<int>>(
						aStreambuf.data(), *program, SplatmapMaterial::TextureUnits::kMetallicRoughness0 + BasicMaterial::kMaxTextures * i
					));
			}

			aStreambuf = {};
			std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].useNormalTexture";
			bool useNormalTexture = material.materials[i].normalTexture != nullptr;
			step->addBindable(std::make_shared<graphics::UniformVariableValue<bool>>(
				aStreambuf.data(), *program, useNormalTexture
			));
			if (useNormalTexture) {
				aStreambuf = {};
				std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].normalTexture";

				material.materials[i].normalTexture->setTextureUnit(
					SplatmapMaterial::TextureUnits::kNormal0 + BasicMaterial::kMaxTextures * i
				);
				step->addBindable(material.materials[i].normalTexture)
					.addBindable(std::make_shared<graphics::UniformVariableValue<int>>(
						aStreambuf.data(), *program, SplatmapMaterial::TextureUnits::kNormal0 + BasicMaterial::kMaxTextures * i
					));

				aStreambuf = {};
				std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].normalScale";
				step->addBindable(std::make_shared<graphics::UniformVariableValue<float>>(
					aStreambuf.data(), *program, material.materials[i].normalScale
				));
			}
		}

		material.splatmapTexture->setTextureUnit(SplatmapMaterial::TextureUnits::kSplatmap);
		step->addBindable(material.splatmapTexture)
			.addBindable(std::make_shared<graphics::UniformVariableValue<int>>(
				"uSMaterial.splatmapTexture", *program, SplatmapMaterial::TextureUnits::kSplatmap
			));
	}


	TechniqueLoader::ProgramUPtr TechniqueLoader::createProgram(
		const char* vertexShaderPath,
		const char* geometryShaderPath,
		const char* fragmentShaderPath
	) {
		// 1. Read the shader text from the shader files
		std::string vertexShaderText;
		if (vertexShaderPath) {
			if (std::ifstream ifs(vertexShaderPath); ifs.good()) {
				vertexShaderText.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
			}
			else {
				SOMBRA_ERROR_LOG << "Vertex shader not found";
				return nullptr;
			}
		}

		std::string geometryShaderText;
		if (geometryShaderPath) {
			if (std::ifstream ifs(geometryShaderPath); ifs.good()) {
				geometryShaderText.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
			}
			else {
				SOMBRA_ERROR_LOG << "Geometry shader not found";
				return nullptr;
			}
		}

		std::string fragmentShaderText;
		if (fragmentShaderPath) {
			if (std::ifstream ifs(fragmentShaderPath); ifs.good()) {
				fragmentShaderText.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
			}
			else {
				SOMBRA_ERROR_LOG << "Fragment shader not found";
				return nullptr;
			}
		}

		// 2. Create the Program
		try {
			utils::FixedVector<graphics::Shader, 3> shaders;
			if (vertexShaderPath) { shaders.emplace_back(vertexShaderText.c_str(), graphics::ShaderType::Vertex); }
			if (geometryShaderPath) { shaders.emplace_back(geometryShaderText.c_str(), graphics::ShaderType::Geometry); }
			if (fragmentShaderPath) { shaders.emplace_back(fragmentShaderText.c_str(), graphics::ShaderType::Fragment); }

			utils::FixedVector<graphics::Shader*, 3> shaderPtrs;
			for (auto& shader : shaders) {
				shaderPtrs.push_back(&shader);
			}

			return std::make_unique<graphics::Program>(shaderPtrs.data(), shaderPtrs.size());
		}
		catch (std::exception& e) {
			SOMBRA_ERROR_LOG << e.what();
			return nullptr;
		}
	}

}
