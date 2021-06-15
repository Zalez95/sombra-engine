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

	void ShaderLoader::addMaterialBindables(StepRef step, const Material& material, const ProgramRef& program)
	{
		// Set the program
		bool hasProgram = false;
		step->processPrograms([&](const ProgramRef& program2) {
			hasProgram |= (program == program2);
		});
		if (!hasProgram) {
			step->addResource(program);
		}

		// Set the material alphaMode
		step->addBindable(std::make_shared<graphics::SetOperation>(graphics::Operation::Blending, material.alphaMode == graphics::AlphaMode::Blend))
			.addBindable(std::make_shared<graphics::SetOperation>(graphics::Operation::DepthTest, material.alphaMode != graphics::AlphaMode::Blend));

		// Unset face culling
		step->addBindable(std::make_shared<graphics::SetOperation>(graphics::Operation::Culling, !material.doubleSided));

		// Set uniforms
		step->addBindable(std::make_shared<graphics::UniformVariableValue<glm::vec4>>(
			"uMaterial.pbrMetallicRoughness.baseColorFactor", program.get(), material.pbrMetallicRoughness.baseColorFactor
		));

		bool useBaseColorTexture = material.pbrMetallicRoughness.baseColorTexture;
		step->addBindable(std::make_shared<graphics::UniformVariableValue<int>>(
				"uMaterial.pbrMetallicRoughness.useBaseColorTexture", program.get(), useBaseColorTexture
			))
			.addBindable(std::make_shared<graphics::UniformVariableValue<int>>(
				"uMaterial.pbrMetallicRoughness.baseColorTexture", program.get(), Material::TextureUnits::kBaseColor
			));
		if (useBaseColorTexture) {
			TextureRef texture = material.pbrMetallicRoughness.baseColorTexture;
			texture->setTextureUnit(Material::TextureUnits::kBaseColor);
			step->addResource(std::move(texture));
		}

		step->addBindable(std::make_shared<graphics::UniformVariableValue<float>>(
				"uMaterial.pbrMetallicRoughness.metallicFactor", program.get(), material.pbrMetallicRoughness.metallicFactor
			))
			.addBindable(std::make_shared<graphics::UniformVariableValue<float>>(
				"uMaterial.pbrMetallicRoughness.roughnessFactor", program.get(), material.pbrMetallicRoughness.roughnessFactor
			));

		bool useMetallicRoughnessTexture = material.pbrMetallicRoughness.metallicRoughnessTexture;
		step->addBindable(std::make_shared<graphics::UniformVariableValue<int>>(
				"uMaterial.pbrMetallicRoughness.useMetallicRoughnessTexture", program.get(), useMetallicRoughnessTexture
			))
			.addBindable(std::make_shared<graphics::UniformVariableValue<int>>(
				"uMaterial.pbrMetallicRoughness.metallicRoughnessTexture", program.get(), Material::TextureUnits::kMetallicRoughness
			));
		if (useMetallicRoughnessTexture) {
			TextureRef texture = material.pbrMetallicRoughness.metallicRoughnessTexture;
			texture->setTextureUnit(Material::TextureUnits::kMetallicRoughness);
			step->addResource(std::move(texture));
		}

		bool useNormalTexture = material.normalTexture;
		step->addBindable(std::make_shared<graphics::UniformVariableValue<int>>(
				"uMaterial.useNormalTexture", program.get(), useNormalTexture
			))
			.addBindable(std::make_shared<graphics::UniformVariableValue<int>>(
				"uMaterial.normalTexture", program.get(), Material::TextureUnits::kNormal
			))
			.addBindable(std::make_shared<graphics::UniformVariableValue<float>>(
				"uMaterial.normalScale", program.get(), material.normalScale
			));
		if (useNormalTexture) {
			TextureRef texture = material.normalTexture;
			texture->setTextureUnit(Material::TextureUnits::kNormal);
			step->addResource(std::move(texture));
		}

		bool useOcclusionTexture = material.occlusionTexture;
		step->addBindable(std::make_shared<graphics::UniformVariableValue<int>>(
				"uMaterial.useOcclusionTexture", program.get(), useOcclusionTexture
			))
			.addBindable(std::make_shared<graphics::UniformVariableValue<int>>(
				"uMaterial.occlusionTexture", program.get(), Material::TextureUnits::kOcclusion
			))
			.addBindable(std::make_shared<graphics::UniformVariableValue<float>>(
				"uMaterial.occlusionStrength", program.get(), material.occlusionStrength
			));
		if (useOcclusionTexture) {
			TextureRef texture = material.occlusionTexture;
			texture->setTextureUnit(Material::TextureUnits::kOcclusion);
			step->addResource(std::move(texture));
		}

		bool useEmissiveTexture = material.emissiveTexture;
		step->addBindable(std::make_shared<graphics::UniformVariableValue<int>>(
				"uMaterial.useEmissiveTexture", program.get(), useEmissiveTexture
			))
			.addBindable(std::make_shared<graphics::UniformVariableValue<int>>(
				"uMaterial.emissiveTexture", program.get(), Material::TextureUnits::kEmissive
			))
			.addBindable(std::make_shared<graphics::UniformVariableValue<glm::vec3>>(
				"uMaterial.emissiveFactor", program.get(), material.emissiveFactor
			));
		if (useEmissiveTexture) {
			TextureRef texture = material.emissiveTexture;
			texture->setTextureUnit(Material::TextureUnits::kEmissive);
			step->addResource(std::move(texture));
		}

		bool checkAlphaCutoff = (material.alphaMode == graphics::AlphaMode::Mask);
		step->addBindable(std::make_shared<graphics::UniformVariableValue<int>>(
			"uMaterial.checkAlphaCutoff", program.get(), checkAlphaCutoff
		));
		if (checkAlphaCutoff) {
			step->addBindable(std::make_shared<graphics::UniformVariableValue<float>>(
				"uMaterial.alphaCutoff", program.get(), material.alphaCutoff
			));
		}
	}


	void ShaderLoader::readMaterialBindables(const StepRef& step, Material& material)
	{
		step->processTextures([&](const TextureRef& texture) {
			switch (texture->getTextureUnit()) {
				case Material::TextureUnits::kBaseColor:
					material.pbrMetallicRoughness.baseColorTexture = texture;
					break;
				case Material::TextureUnits::kMetallicRoughness:
					material.pbrMetallicRoughness.metallicRoughnessTexture = texture;
					break;
				case Material::TextureUnits::kNormal:
					material.normalTexture = texture;
					break;
				case Material::TextureUnits::kOcclusion:
					material.occlusionTexture = texture;
					break;
				case Material::TextureUnits::kEmissive:
					material.emissiveTexture = texture;
					break;
				default:
					break;
			}
		});
		step->processBindables([&](const std::shared_ptr<graphics::Bindable>& bindable) {
			if (auto operation = std::dynamic_pointer_cast<graphics::SetOperation>(bindable); operation) {
				switch (operation->getOperation()) {
					case graphics::Operation::Blending:
						material.alphaMode = operation->isEnabled()? graphics::AlphaMode::Blend : graphics::AlphaMode::Opaque;
						break;
					case graphics::Operation::DepthTest:
						material.alphaMode = operation->isEnabled()? graphics::AlphaMode::Opaque : graphics::AlphaMode::Blend;
						break;
					case graphics::Operation::Culling:
						material.doubleSided = !operation->isEnabled();
						break;
					default:
						break;
				}
			}
			else if (auto uniform = std::dynamic_pointer_cast<graphics::IUniformVariable>(bindable); uniform) {
				if (uniform->getName() == "uMaterial.pbrMetallicRoughness.baseColorFactor") {
					material.pbrMetallicRoughness.baseColorFactor = std::dynamic_pointer_cast<graphics::UniformVariableValue<glm::vec4>>(uniform)->getValue();
				}
				else if (uniform->getName() == "uMaterial.pbrMetallicRoughness.metallicFactor") {
					material.pbrMetallicRoughness.metallicFactor = std::dynamic_pointer_cast<graphics::UniformVariableValue<float>>(uniform)->getValue();
				}
				else if (uniform->getName() == "uMaterial.pbrMetallicRoughness.roughnessFactor") {
					material.pbrMetallicRoughness.roughnessFactor = std::dynamic_pointer_cast<graphics::UniformVariableValue<float>>(uniform)->getValue();
				}
				else if (uniform->getName() == "uMaterial.normalScale") {
					material.normalScale = std::dynamic_pointer_cast<graphics::UniformVariableValue<float>>(uniform)->getValue();
				}
				else if (uniform->getName() == "uMaterial.occlusionStrength") {
					material.occlusionStrength = std::dynamic_pointer_cast<graphics::UniformVariableValue<float>>(uniform)->getValue();
				}
				else if (uniform->getName() == "uMaterial.emissiveFactor") {
					material.emissiveFactor = std::dynamic_pointer_cast<graphics::UniformVariableValue<glm::vec3>>(uniform)->getValue();
				}
				else if (uniform->getName() == "uMaterial.checkAlphaCutoff") {
					if (std::dynamic_pointer_cast<graphics::UniformVariableValue<int>>(uniform)->getValue()) {
						material.alphaMode = graphics::AlphaMode::Mask;
					}
				}
				else if (uniform->getName() == "uMaterial.alphaCutoff") {
					material.alphaCutoff = std::dynamic_pointer_cast<graphics::UniformVariableValue<float>>(uniform)->getValue();
				}
			}
		});
	}


	void ShaderLoader::addSplatmapMaterialBindables(StepRef step, const SplatmapMaterial& material, const ProgramRef& program)
	{
		// Set the program
		bool hasProgram = false;
		step->processPrograms([&](const ProgramRef& program2) {
			hasProgram |= (program == program2);
		});
		if (!hasProgram) {
			step->addResource(program);
		}

		// Set the material alphaMode
		step->addBindable(std::make_shared<graphics::SetOperation>(graphics::Operation::Blending, false))
			.addBindable(std::make_shared<graphics::SetOperation>(graphics::Operation::DepthTest, true));

		// Set face culling
		step->addBindable(std::make_shared<graphics::SetOperation>(graphics::Operation::Culling, true));

		// Set uniforms
		int numMaterials = static_cast<int>(material.materials.size());
		step->addBindable(std::make_shared<graphics::UniformVariableValue<int>>(
			"uSMaterial.numMaterials", program.get(), numMaterials
		));

		for (int i = 0; i < numMaterials; ++i) {
			utils::ArrayStreambuf<char, 128> aStreambuf;
			std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].pbrMetallicRoughness.baseColorFactor";
			step->addBindable(std::make_shared<graphics::UniformVariableValue<glm::vec4>>(
				aStreambuf.data(), program.get(), material.materials[i].pbrMetallicRoughness.baseColorFactor
			));

			aStreambuf = {};
			std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].pbrMetallicRoughness.useBaseColorTexture";
			bool useBaseColorTexture = material.materials[i].pbrMetallicRoughness.baseColorTexture;
			step->addBindable(std::make_shared<graphics::UniformVariableValue<int>>(
				aStreambuf.data(), program.get(), useBaseColorTexture
			));

			aStreambuf = {};
			std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].pbrMetallicRoughness.baseColorTexture";
			step->addBindable(std::make_shared<graphics::UniformVariableValue<int>>(
				aStreambuf.data(), program.get(), SplatmapMaterial::TextureUnits::kBaseColor0 + BasicMaterial::kMaxTextures * i
			));

			if (useBaseColorTexture) {
				TextureRef texture = material.materials[i].pbrMetallicRoughness.baseColorTexture;
				texture->setTextureUnit(SplatmapMaterial::TextureUnits::kBaseColor0 + BasicMaterial::kMaxTextures * i);
				step->addResource(std::move(texture));
			}

			aStreambuf = {};
			std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].pbrMetallicRoughness.metallicFactor";
			step->addBindable(std::make_shared<graphics::UniformVariableValue<float>>(
				aStreambuf.data(), program.get(), material.materials[i].pbrMetallicRoughness.metallicFactor
			));

			aStreambuf = {};
			std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].pbrMetallicRoughness.roughnessFactor";
			step->addBindable(std::make_shared<graphics::UniformVariableValue<float>>(
				aStreambuf.data(), program.get(), material.materials[i].pbrMetallicRoughness.roughnessFactor
			));

			aStreambuf = {};
			std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].pbrMetallicRoughness.useMetallicRoughnessTexture";
			bool useMetallicRoughnessTexture = material.materials[i].pbrMetallicRoughness.metallicRoughnessTexture;
			step->addBindable(std::make_shared<graphics::UniformVariableValue<int>>(
				aStreambuf.data(), program.get(), useMetallicRoughnessTexture
			));

			aStreambuf = {};
			std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].pbrMetallicRoughness.metallicRoughnessTexture";
			step->addBindable(std::make_shared<graphics::UniformVariableValue<int>>(
				aStreambuf.data(), program.get(), SplatmapMaterial::TextureUnits::kMetallicRoughness0 + BasicMaterial::kMaxTextures * i
			));

			if (useMetallicRoughnessTexture) {
				TextureRef texture = material.materials[i].pbrMetallicRoughness.metallicRoughnessTexture;
				texture->setTextureUnit(SplatmapMaterial::TextureUnits::kMetallicRoughness0 + BasicMaterial::kMaxTextures * i);
				step->addResource(std::move(texture));
			}

			aStreambuf = {};
			std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].useNormalTexture";
			bool useNormalTexture = material.materials[i].normalTexture;
			step->addBindable(std::make_shared<graphics::UniformVariableValue<int>>(
				aStreambuf.data(), program.get(), useNormalTexture
			));

			aStreambuf = {};
			std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].normalTexture";
			step->addBindable(std::make_shared<graphics::UniformVariableValue<int>>(
				aStreambuf.data(), program.get(), SplatmapMaterial::TextureUnits::kNormal0 + BasicMaterial::kMaxTextures * i
			));

			aStreambuf = {};
			std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].normalScale";
			step->addBindable(std::make_shared<graphics::UniformVariableValue<float>>(
				aStreambuf.data(), program.get(), material.materials[i].normalScale
			));

			if (useNormalTexture) {
				TextureRef texture = material.materials[i].normalTexture;
				texture->setTextureUnit(SplatmapMaterial::TextureUnits::kNormal0 + BasicMaterial::kMaxTextures * i);
				step->addResource(std::move(texture));
			}
		}

		TextureRef texture = material.splatmapTexture;
		texture->setTextureUnit(SplatmapMaterial::TextureUnits::kSplatmap);
		step->addResource(std::move(texture))
			.addBindable(std::make_shared<graphics::UniformVariableValue<int>>(
				"uSMaterial.splatmapTexture", program.get(), SplatmapMaterial::TextureUnits::kSplatmap
			));
	}


	void ShaderLoader::readSplatmapMaterialBindables(const StepRef& step, SplatmapMaterial& material)
	{
		step->processBindables([&](const std::shared_ptr<graphics::Bindable>& bindable) {
			if (auto uniform = std::dynamic_pointer_cast<graphics::IUniformVariable>(bindable); uniform) {
				if (uniform->getName() == "uSMaterial.numMaterials") {
					material.materials.resize(std::dynamic_pointer_cast<graphics::UniformVariableValue<int>>(uniform)->getValue());
				}
			}
		});

		step->processTextures([&](const TextureRef& texture) {
			if (texture->getTextureUnit() == SplatmapMaterial::TextureUnits::kSplatmap) {
				material.splatmapTexture = texture;
			}
			else {
				for (int i = 0; i < static_cast<int>(material.materials.size()); ++i) {
					if (texture->getTextureUnit() == SplatmapMaterial::TextureUnits::kBaseColor0 + BasicMaterial::kMaxTextures * i) {
						material.materials[i].pbrMetallicRoughness.baseColorTexture = texture;
					}
					else if (texture->getTextureUnit() == SplatmapMaterial::TextureUnits::kMetallicRoughness0 + BasicMaterial::kMaxTextures * i) {
						material.materials[i].pbrMetallicRoughness.metallicRoughnessTexture = texture;
					}
					else if (texture->getTextureUnit() == SplatmapMaterial::TextureUnits::kNormal0 + BasicMaterial::kMaxTextures * i) {
						material.materials[i].normalTexture = texture;
					}
				}
			}
		});

		step->processBindables([&](const std::shared_ptr<graphics::Bindable>& bindable) {
			if (auto uniform = std::dynamic_pointer_cast<graphics::IUniformVariable>(bindable); uniform) {
				for (int i = 0; i < static_cast<int>(material.materials.size()); ++i) {
					utils::ArrayStreambuf<char, 128> aStreambuf;
					std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].pbrMetallicRoughness.baseColorFactor";
					if (uniform->getName() == aStreambuf.data()) {
						material.materials[i].pbrMetallicRoughness.baseColorFactor = std::dynamic_pointer_cast<graphics::UniformVariableValue<glm::vec4>>(uniform)->getValue();
						continue;
					}

					aStreambuf = {};
					std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].pbrMetallicRoughness.metallicFactor";
					if (uniform->getName() == aStreambuf.data()) {
						material.materials[i].pbrMetallicRoughness.metallicFactor = std::dynamic_pointer_cast<graphics::UniformVariableValue<float>>(uniform)->getValue();
						continue;
					}

					aStreambuf = {};
					std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].pbrMetallicRoughness.roughnessFactor";
					if (uniform->getName() == aStreambuf.data()) {
						material.materials[i].pbrMetallicRoughness.roughnessFactor = std::dynamic_pointer_cast<graphics::UniformVariableValue<float>>(uniform)->getValue();
						continue;
					}

					aStreambuf = {};
					std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].normalScale";
					if (uniform->getName() == aStreambuf.data()) {
						material.materials[i].normalScale = std::dynamic_pointer_cast<graphics::UniformVariableValue<float>>(uniform)->getValue();
						continue;
					}
				}
			}
		});
	}


	void ShaderLoader::addHeightMapBindables(
		StepRef step, const TextureRef& heightMap, float size, float maxHeight, const ProgramRef& program
	) {
		bool hasProgram = false;
		step->processPrograms([&](const ProgramRef& program2) {
			hasProgram |= (program == program2);
		});
		if (!hasProgram) {
			step->addResource(program);
		}

		auto normalMapTexture = TextureUtils::heightmapToNormalMapLocal(heightMap.get(), heightMap->getWidth(), heightMap->getHeight());
		normalMapTexture->setTextureUnit(SplatmapMaterial::TextureUnits::kNormalMap);

		step->addResource(std::move(heightMap))
			.addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uHeightMap", program.get(), SplatmapMaterial::TextureUnits::kHeightMap))
			.addBindable(std::move(normalMapTexture))
			.addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uNormalMap", program.get(), SplatmapMaterial::TextureUnits::kNormalMap))
			.addBindable(std::make_shared<graphics::UniformVariableValue<float>>("uXZSize", program.get(), size))
			.addBindable(std::make_shared<graphics::UniformVariableValue<float>>("uMaxHeight", program.get(), maxHeight));
	}


	Result ShaderLoader::createProgram(
		const char* vertexShaderPath, const char* geometryShaderPath, const char* fragmentShaderPath,
		std::shared_ptr<graphics::Program>& program
	) {
		// 1. Read the shader text from the shader files
		std::string vertexShaderText;
		if (vertexShaderPath) {
			if (std::ifstream ifs(vertexShaderPath); ifs.good()) {
				vertexShaderText.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
			}
			else {
				return Result(false, "Vertex shader not found");
			}
		}

		std::string geometryShaderText;
		if (geometryShaderPath) {
			if (std::ifstream ifs(geometryShaderPath); ifs.good()) {
				geometryShaderText.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
			}
			else {
				return Result(false, "Geometry shader not found");
			}
		}

		std::string fragmentShaderText;
		if (fragmentShaderPath) {
			if (std::ifstream ifs(fragmentShaderPath); ifs.good()) {
				fragmentShaderText.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
			}
			else {
				return Result(false, "Fragment shader not found");
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

			program = std::make_shared<graphics::Program>(shaderPtrs.data(), shaderPtrs.size());
		}
		catch (std::exception& e) {
			return Result(false, "Exception: " + std::string(e.what()));
		}

		return Result();
	}

}
