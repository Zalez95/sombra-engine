#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "se/utils/Log.h"
#include "se/utils/FixedVector.h"
#include "se/graphics/core/Shader.h"
#include "se/graphics/core/UniformVariable.h"
#include "se/graphics/core/GraphicsOperations.h"
#include "se/app/io/ShaderLoader.h"
#include "se/app/graphics/TextureUtils.h"

namespace se::app {

	void ShaderLoader::addMaterialBindables(PassSPtr pass, const Material& material, const ProgramSPtr program)
	{
		// Set the material alphaMode
		pass->addBindable(std::make_shared<graphics::SetOperation>(graphics::Operation::Blending, material.alphaMode == graphics::AlphaMode::Blend))
			.addBindable(std::make_shared<graphics::SetOperation>(graphics::Operation::DepthTest, material.alphaMode != graphics::AlphaMode::Blend));

		// Unset face culling
		pass->addBindable(std::make_shared<graphics::SetOperation>(graphics::Operation::Culling, !material.doubleSided));

		// Set uniforms
		pass->addBindable(std::make_shared<graphics::UniformVariableValue<glm::vec4>>(
			"uMaterial.pbrMetallicRoughness.baseColorFactor", *program, material.pbrMetallicRoughness.baseColorFactor
		));

		bool useBaseColorTexture = material.pbrMetallicRoughness.baseColorTexture != nullptr;
		pass->addBindable(std::make_shared<graphics::UniformVariableValue<bool>>(
				"uMaterial.pbrMetallicRoughness.useBaseColorTexture", *program, useBaseColorTexture
			))
			.addBindable(std::make_shared<graphics::UniformVariableValue<int>>(
				"uMaterial.pbrMetallicRoughness.baseColorTexture", *program, Material::TextureUnits::kBaseColor
			));
		if (useBaseColorTexture) {
			material.pbrMetallicRoughness.baseColorTexture->setTextureUnit(Material::TextureUnits::kBaseColor);
			pass->addBindable(material.pbrMetallicRoughness.baseColorTexture);
		}

		pass->addBindable(std::make_shared<graphics::UniformVariableValue<float>>(
				"uMaterial.pbrMetallicRoughness.metallicFactor", *program, material.pbrMetallicRoughness.metallicFactor
			))
			.addBindable(std::make_shared<graphics::UniformVariableValue<float>>(
				"uMaterial.pbrMetallicRoughness.roughnessFactor", *program, material.pbrMetallicRoughness.roughnessFactor
			));

		bool useMetallicRoughnessTexture = material.pbrMetallicRoughness.metallicRoughnessTexture != nullptr;
		pass->addBindable(std::make_shared<graphics::UniformVariableValue<bool>>(
				"uMaterial.pbrMetallicRoughness.useMetallicRoughnessTexture", *program, useMetallicRoughnessTexture
			))
			.addBindable(std::make_shared<graphics::UniformVariableValue<int>>(
				"uMaterial.pbrMetallicRoughness.metallicRoughnessTexture", *program, Material::TextureUnits::kMetallicRoughness
			));
		if (useMetallicRoughnessTexture) {
			material.pbrMetallicRoughness.metallicRoughnessTexture->setTextureUnit(Material::TextureUnits::kMetallicRoughness);
			pass->addBindable(material.pbrMetallicRoughness.metallicRoughnessTexture);
		}

		bool useNormalTexture = material.normalTexture != nullptr;
		pass->addBindable(std::make_shared<graphics::UniformVariableValue<bool>>(
				"uMaterial.useNormalTexture", *program, useNormalTexture
			))
			.addBindable(std::make_shared<graphics::UniformVariableValue<int>>(
				"uMaterial.normalTexture", *program, Material::TextureUnits::kNormal
			))
			.addBindable(std::make_shared<graphics::UniformVariableValue<float>>(
				"uMaterial.normalScale", *program, material.normalScale
			));
		if (useNormalTexture) {
			material.normalTexture->setTextureUnit(Material::TextureUnits::kNormal);
			pass->addBindable(material.normalTexture);
		}

		bool useOcclusionTexture = material.occlusionTexture != nullptr;
		pass->addBindable(std::make_shared<graphics::UniformVariableValue<bool>>(
				"uMaterial.useOcclusionTexture", *program, useOcclusionTexture
			))
			.addBindable(std::make_shared<graphics::UniformVariableValue<int>>(
				"uMaterial.occlusionTexture", *program, Material::TextureUnits::kOcclusion
			))
			.addBindable(std::make_shared<graphics::UniformVariableValue<float>>(
				"uMaterial.occlusionStrength", *program, material.occlusionStrength
			));
		if (useOcclusionTexture) {
			material.occlusionTexture->setTextureUnit(Material::TextureUnits::kOcclusion);
			pass->addBindable(material.occlusionTexture);
		}

		bool useEmissiveTexture = material.emissiveTexture != nullptr;
		pass->addBindable(std::make_shared<graphics::UniformVariableValue<bool>>(
				"uMaterial.useEmissiveTexture", *program, useEmissiveTexture
			))
			.addBindable(std::make_shared<graphics::UniformVariableValue<int>>(
				"uMaterial.emissiveTexture", *program, Material::TextureUnits::kEmissive
			))
			.addBindable(std::make_shared<graphics::UniformVariableValue<glm::vec3>>(
				"uMaterial.emissiveFactor", *program, material.emissiveFactor
			));
		if (useEmissiveTexture) {
			material.emissiveTexture->setTextureUnit(Material::TextureUnits::kEmissive);
			pass->addBindable(material.emissiveTexture);
		}

		bool checkAlphaCutoff = (material.alphaMode == graphics::AlphaMode::Mask);
		pass->addBindable(std::make_shared<graphics::UniformVariableValue<bool>>(
			"uMaterial.checkAlphaCutoff", *program, checkAlphaCutoff
		));
		if (checkAlphaCutoff) {
			pass->addBindable(std::make_shared<graphics::UniformVariableValue<float>>(
				"uMaterial.alphaCutoff", *program, material.alphaCutoff
			));
		}
	}


	void ShaderLoader::addSplatmapMaterialBindables(PassSPtr pass, const SplatmapMaterial& material, const ProgramSPtr program)
	{
		// Set the material alphaMode
		pass->addBindable(std::make_shared<graphics::SetOperation>(graphics::Operation::Blending, false))
			.addBindable(std::make_shared<graphics::SetOperation>(graphics::Operation::DepthTest, true));

		// Set face culling
		pass->addBindable(std::make_shared<graphics::SetOperation>(graphics::Operation::Culling, true));

		// Set uniforms
		int numMaterials = static_cast<int>(material.materials.size());
		pass->addBindable(std::make_shared<graphics::UniformVariableValue<int>>(
			"uSMaterial.numMaterials", *program, numMaterials
		));

		for (int i = 0; i < numMaterials; ++i) {
			utils::ArrayStreambuf<char, 128> aStreambuf;
			std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].pbrMetallicRoughness.baseColorFactor";
			pass->addBindable(std::make_shared<graphics::UniformVariableValue<glm::vec4>>(
				aStreambuf.data(), *program, material.materials[i].pbrMetallicRoughness.baseColorFactor
			));

			aStreambuf = {};
			std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].pbrMetallicRoughness.useBaseColorTexture";
			bool useBaseColorTexture = material.materials[i].pbrMetallicRoughness.baseColorTexture != nullptr;
			pass->addBindable(std::make_shared<graphics::UniformVariableValue<bool>>(
				aStreambuf.data(), *program, useBaseColorTexture
			));

			aStreambuf = {};
			std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].pbrMetallicRoughness.baseColorTexture";
			pass->addBindable(std::make_shared<graphics::UniformVariableValue<int>>(
				aStreambuf.data(), *program, SplatmapMaterial::TextureUnits::kBaseColor0 + BasicMaterial::kMaxTextures * i
			));

			if (useBaseColorTexture) {
				material.materials[i].pbrMetallicRoughness.baseColorTexture->setTextureUnit(
					SplatmapMaterial::TextureUnits::kBaseColor0 + BasicMaterial::kMaxTextures * i
				);
				pass->addBindable(material.materials[i].pbrMetallicRoughness.baseColorTexture);
			}

			aStreambuf = {};
			std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].pbrMetallicRoughness.metallicFactor";
			pass->addBindable(std::make_shared<graphics::UniformVariableValue<float>>(
				aStreambuf.data(), *program, material.materials[i].pbrMetallicRoughness.metallicFactor
			));

			aStreambuf = {};
			std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].pbrMetallicRoughness.roughnessFactor";
			pass->addBindable(std::make_shared<graphics::UniformVariableValue<float>>(
				aStreambuf.data(), *program, material.materials[i].pbrMetallicRoughness.roughnessFactor
			));

			aStreambuf = {};
			std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].pbrMetallicRoughness.useMetallicRoughnessTexture";
			bool useMetallicRoughnessTexture = material.materials[i].pbrMetallicRoughness.metallicRoughnessTexture != nullptr;
			pass->addBindable(std::make_shared<graphics::UniformVariableValue<bool>>(
				aStreambuf.data(), *program, useMetallicRoughnessTexture
			));

			aStreambuf = {};
			std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].pbrMetallicRoughness.metallicRoughnessTexture";
			pass->addBindable(std::make_shared<graphics::UniformVariableValue<int>>(
				aStreambuf.data(), *program, SplatmapMaterial::TextureUnits::kMetallicRoughness0 + BasicMaterial::kMaxTextures * i
			));

			if (useMetallicRoughnessTexture) {
				material.materials[i].pbrMetallicRoughness.metallicRoughnessTexture->setTextureUnit(
					SplatmapMaterial::TextureUnits::kMetallicRoughness0 + BasicMaterial::kMaxTextures * i
				);
				pass->addBindable(material.materials[i].pbrMetallicRoughness.metallicRoughnessTexture);
			}

			aStreambuf = {};
			std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].useNormalTexture";
			bool useNormalTexture = material.materials[i].normalTexture != nullptr;
			pass->addBindable(std::make_shared<graphics::UniformVariableValue<bool>>(
				aStreambuf.data(), *program, useNormalTexture
			));

			aStreambuf = {};
			std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].normalTexture";
			pass->addBindable(std::make_shared<graphics::UniformVariableValue<int>>(
				aStreambuf.data(), *program, SplatmapMaterial::TextureUnits::kNormal0 + BasicMaterial::kMaxTextures * i
			));

			aStreambuf = {};
			std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].normalScale";
			pass->addBindable(std::make_shared<graphics::UniformVariableValue<float>>(
				aStreambuf.data(), *program, material.materials[i].normalScale
			));

			if (useNormalTexture) {
				material.materials[i].normalTexture->setTextureUnit(
					SplatmapMaterial::TextureUnits::kNormal0 + BasicMaterial::kMaxTextures * i
				);
				pass->addBindable(material.materials[i].normalTexture);
			}
		}

		material.splatmapTexture->setTextureUnit(SplatmapMaterial::TextureUnits::kSplatmap);
		pass->addBindable(material.splatmapTexture)
			.addBindable(std::make_shared<graphics::UniformVariableValue<int>>(
				"uSMaterial.splatmapTexture", *program, SplatmapMaterial::TextureUnits::kSplatmap
			));
	}


	void ShaderLoader::addHeightMapBindables(
		PassSPtr pass, const Image<unsigned char>& heightMap, float size, float maxHeight, const ProgramSPtr program
	) {
		auto heightMapTexture = std::make_shared<graphics::Texture>(graphics::TextureTarget::Texture2D);
		heightMapTexture->setTextureUnit(SplatmapMaterial::TextureUnits::kHeightMap)
			.setFiltering(graphics::TextureFilter::Linear, graphics::TextureFilter::Linear)
			.setWrapping(se::graphics::TextureWrap::ClampToEdge, se::graphics::TextureWrap::ClampToEdge)
			.setImage(
				heightMap.pixels.get(), graphics::TypeId::UnsignedByte, graphics::ColorFormat::Red, graphics::ColorFormat::Red,
				heightMap.width, heightMap.height
			);

		auto normalMapTexture = TextureUtils::heightmapToNormalMapLocal(heightMapTexture, heightMap.width, heightMap.height);
		normalMapTexture->setTextureUnit(SplatmapMaterial::TextureUnits::kNormalMap);

		pass->addBindable(std::move(heightMapTexture))
			.addBindable(std::make_shared<graphics::UniformVariableValue<int>>(
				"uHeightMap", *program, SplatmapMaterial::TextureUnits::kHeightMap
			))
			.addBindable(std::move(normalMapTexture))
			.addBindable(std::make_shared<graphics::UniformVariableValue<int>>(
				"uNormalMap", *program, SplatmapMaterial::TextureUnits::kNormalMap
			))
			.addBindable(std::make_shared<graphics::UniformVariableValue<float>>(
				"uXZSize", *program, size
			))
			.addBindable(std::make_shared<graphics::UniformVariableValue<float>>(
				"uMaxHeight", *program, maxHeight
			));
	}


	ShaderLoader::ProgramUPtr ShaderLoader::createProgram(
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
