#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "se/utils/Log.h"
#include "se/graphics/core/Shader.h"
#include "se/graphics/core/GraphicsOperations.h"
#include "se/app/io/ShaderLoader.h"
#include "se/app/graphics/TextureUtils.h"

namespace se::app {

	void ShaderLoader::addMaterialBindables(StepResource step, const Material& material, const ProgramResource& program)
	{
		graphics::Context& context = *program->getParent();

		// Set the program
		bool hasProgram = false;
		step->processPrograms([&](const ProgramResource& program2) {
			hasProgram |= (program == program2);
		});
		if (!hasProgram) {
			step->addResource(program);
		}

		// Set the material alphaMode
		step->addBindable(context.create<graphics::SetOperation>(graphics::Operation::Blending, material.alphaMode == graphics::AlphaMode::Blend))
			.addBindable(context.create<graphics::SetOperation>(graphics::Operation::DepthTest, material.alphaMode != graphics::AlphaMode::Blend));

		// Unset face culling
		step->addBindable(context.create<graphics::SetOperation>(graphics::Operation::Culling, !material.doubleSided));

		// Set uniforms
		auto programRef = *program;

		step->addBindable(
			context.create<graphics::UniformVariableValue<glm::vec4>>("uMaterial.pbrMetallicRoughness.baseColorFactor", material.pbrMetallicRoughness.baseColorFactor)
				.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(programRef)); })
		);

		bool useBaseColorTexture = material.pbrMetallicRoughness.baseColorTexture;
		step->addBindable(
				context.create<graphics::UniformVariableValue<int>>("uMaterial.pbrMetallicRoughness.useBaseColorTexture", useBaseColorTexture)
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(programRef)); })
			)
			.addBindable(
				context.create<graphics::UniformVariableValue<int>>("uMaterial.pbrMetallicRoughness.baseColorTexture", Material::TextureUnits::kBaseColor)
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(programRef)); })
			);
		if (useBaseColorTexture) {
			auto texture = material.pbrMetallicRoughness.baseColorTexture;
			texture->edit([](graphics::Texture& tex) { tex.setTextureUnit(Material::TextureUnits::kBaseColor); });
			step->addResource(std::move(texture));
		}

		step->addBindable(
				context.create<graphics::UniformVariableValue<float>>("uMaterial.pbrMetallicRoughness.metallicFactor", material.pbrMetallicRoughness.metallicFactor)
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(programRef)); })
			)
			.addBindable(
				context.create<graphics::UniformVariableValue<float>>("uMaterial.pbrMetallicRoughness.roughnessFactor", material.pbrMetallicRoughness.roughnessFactor)
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(programRef)); })
			);

		bool useMetallicRoughnessTexture = material.pbrMetallicRoughness.metallicRoughnessTexture;
		step->addBindable(
				context.create<graphics::UniformVariableValue<int>>("uMaterial.pbrMetallicRoughness.useMetallicRoughnessTexture", useMetallicRoughnessTexture)
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(programRef)); })
			)
			.addBindable(
				context.create<graphics::UniformVariableValue<int>>("uMaterial.pbrMetallicRoughness.metallicRoughnessTexture", Material::TextureUnits::kMetallicRoughness)
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(programRef)); })
			);
		if (useMetallicRoughnessTexture) {
			auto texture = material.pbrMetallicRoughness.metallicRoughnessTexture;
			texture->edit([](graphics::Texture& tex) { tex.setTextureUnit(Material::TextureUnits::kMetallicRoughness); });
			step->addResource(std::move(texture));
		}

		bool useNormalTexture = material.normalTexture;
		step->addBindable(
				context.create<graphics::UniformVariableValue<int>>("uMaterial.useNormalTexture", useNormalTexture)
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(programRef)); })
			)
			.addBindable(
				context.create<graphics::UniformVariableValue<int>>("uMaterial.normalTexture", Material::TextureUnits::kNormal)
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(programRef)); })
			)
			.addBindable(
				context.create<graphics::UniformVariableValue<float>>("uMaterial.normalScale", material.normalScale)
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(programRef)); })
			);
		if (useNormalTexture) {
			auto texture = material.normalTexture;
			texture->edit([](graphics::Texture& tex) { tex.setTextureUnit(Material::TextureUnits::kNormal); });
			step->addResource(std::move(texture));
		}

		bool useOcclusionTexture = material.occlusionTexture;
		step->addBindable(
				context.create<graphics::UniformVariableValue<int>>("uMaterial.useOcclusionTexture", useOcclusionTexture)
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(programRef)); })
			)
			.addBindable(
				context.create<graphics::UniformVariableValue<int>>("uMaterial.occlusionTexture", Material::TextureUnits::kOcclusion)
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(programRef)); })
			)
			.addBindable(
				context.create<graphics::UniformVariableValue<float>>("uMaterial.occlusionStrength", material.occlusionStrength)
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(programRef)); })
			);
		if (useOcclusionTexture) {
			auto texture = material.occlusionTexture;
			texture->edit([](graphics::Texture& tex) { tex.setTextureUnit(Material::TextureUnits::kOcclusion); });
			step->addResource(std::move(texture));
		}

		bool useEmissiveTexture = material.emissiveTexture;
		step->addBindable(
				context.create<graphics::UniformVariableValue<int>>("uMaterial.useEmissiveTexture", useEmissiveTexture)
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(programRef)); })
			)
			.addBindable(
				context.create<graphics::UniformVariableValue<int>>("uMaterial.emissiveTexture", Material::TextureUnits::kEmissive)
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(programRef)); })
			)
			.addBindable(
				context.create<graphics::UniformVariableValue<glm::vec3>>("uMaterial.emissiveFactor", material.emissiveFactor)
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(programRef)); })
			);
		if (useEmissiveTexture) {
			auto texture = material.emissiveTexture;
			texture->edit([](graphics::Texture& tex) { tex.setTextureUnit(Material::TextureUnits::kEmissive); });
			step->addResource(std::move(texture));
		}

		bool checkAlphaCutoff = (material.alphaMode == graphics::AlphaMode::Mask);
		step->addBindable(
			context.create<graphics::UniformVariableValue<int>>("uMaterial.checkAlphaCutoff", checkAlphaCutoff)
				.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(programRef)); })
			);
		if (checkAlphaCutoff) {
			step->addBindable(
				context.create<graphics::UniformVariableValue<float>>("uMaterial.alphaCutoff", material.alphaCutoff	)
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(programRef)); })
			);
		}
	}


	void ShaderLoader::readMaterialBindables(const StepResource& step, Material& material)
	{
		step->processTextures([&](const TextureResource& texture) {
			texture->edit([&](graphics::Texture& tex) {
				switch (tex.getTextureUnit()) {
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
		});

		step->processBindables([&](const graphics::Context::BindableRef& bindable) {
			if (auto operation = graphics::Context::TBindableRef<graphics::SetOperation>::from(bindable); operation) {
				operation.edit([&](graphics::SetOperation& op) {
					switch (op.getOperation()) {
						case graphics::Operation::Blending:
							material.alphaMode = op.getEnable()? graphics::AlphaMode::Blend : graphics::AlphaMode::Opaque;
							break;
						case graphics::Operation::DepthTest:
							material.alphaMode = op.getEnable()? graphics::AlphaMode::Opaque : graphics::AlphaMode::Blend;
							break;
						case graphics::Operation::Culling:
							material.doubleSided = !op.getEnable();
							break;
						default:
							break;
					}
				});
			}
			else if (auto uniform = graphics::Context::TBindableRef<graphics::IUniformVariable>::from(bindable); uniform) {
				uniform.edit([&](graphics::IUniformVariable& iuv) {
					if (iuv.getName() == "uMaterial.pbrMetallicRoughness.baseColorFactor") {
						material.pbrMetallicRoughness.baseColorFactor = dynamic_cast<graphics::UniformVariableValue<glm::vec4>*>(&iuv)->getValue();
					}
					else if (iuv.getName() == "uMaterial.pbrMetallicRoughness.metallicFactor") {
						material.pbrMetallicRoughness.metallicFactor = dynamic_cast<graphics::UniformVariableValue<float>*>(&iuv)->getValue();
					}
					else if (iuv.getName() == "uMaterial.pbrMetallicRoughness.roughnessFactor") {
						material.pbrMetallicRoughness.roughnessFactor = dynamic_cast<graphics::UniformVariableValue<float>*>(&iuv)->getValue();
					}
					else if (iuv.getName() == "uMaterial.normalScale") {
						material.normalScale = dynamic_cast<graphics::UniformVariableValue<float>*>(&iuv)->getValue();
					}
					else if (iuv.getName() == "uMaterial.occlusionStrength") {
						material.occlusionStrength = dynamic_cast<graphics::UniformVariableValue<float>*>(&iuv)->getValue();
					}
					else if (iuv.getName() == "uMaterial.emissiveFactor") {
						material.emissiveFactor = dynamic_cast<graphics::UniformVariableValue<glm::vec3>*>(&iuv)->getValue();
					}
					else if (iuv.getName() == "uMaterial.checkAlphaCutoff") {
						if (dynamic_cast<graphics::UniformVariableValue<int>*>(&iuv)->getValue()) {
							material.alphaMode = graphics::AlphaMode::Mask;
						}
					}
					else if (iuv.getName() == "uMaterial.alphaCutoff") {
						material.alphaCutoff = dynamic_cast<graphics::UniformVariableValue<float>*>(&iuv)->getValue();
					}
				});
			}
		});

		step->processPrograms([&](const ProgramResource& program) {
			program->getParent()->wait();
		});
	}


	void ShaderLoader::addSplatmapMaterialBindables(StepResource step, const SplatmapMaterial& material, const ProgramResource& program)
	{
		graphics::Context& context = *program->getParent();

		// Set the program
		bool hasProgram = false;
		step->processPrograms([&](const ProgramResource& program2) {
			hasProgram |= (program == program2);
		});
		if (!hasProgram) {
			step->addResource(program);
		}

		// Set the material alphaMode
		step->addBindable(context.create<graphics::SetOperation>(graphics::Operation::Blending, false))
			.addBindable(context.create<graphics::SetOperation>(graphics::Operation::DepthTest, true));

		// Set face culling
		step->addBindable(context.create<graphics::SetOperation>(graphics::Operation::Culling, true));

		// Set uniforms
		auto programRef = *program;

		int numMaterials = static_cast<int>(material.materials.size());
		step->addBindable(
			context.create<graphics::UniformVariableValue<int>>("uSMaterial.numMaterials", numMaterials)
				.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(programRef)); })
		);

		for (int i = 0; i < numMaterials; ++i) {
			step->addBindable(
				context.create<graphics::UniformVariableValue<glm::vec4>>("uSMaterial.materials[" + std::to_string(i) + "].pbrMetallicRoughness.baseColorFactor", material.materials[i].pbrMetallicRoughness.baseColorFactor)
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(programRef)); })
			);

			bool useBaseColorTexture = material.materials[i].pbrMetallicRoughness.baseColorTexture;
			step->addBindable(
				context.create<graphics::UniformVariableValue<int>>("uSMaterial.materials[" + std::to_string(i) + "].pbrMetallicRoughness.useBaseColorTexture", useBaseColorTexture)
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(programRef)); })
			);

			step->addBindable(
				context.create<graphics::UniformVariableValue<int>>("uSMaterial.materials[" + std::to_string(i) + "].pbrMetallicRoughness.baseColorTexture", SplatmapMaterial::TextureUnits::kBaseColor0 + BasicMaterial::kMaxTextures * i)
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(programRef)); })
			);

			if (useBaseColorTexture) {
				auto texture = material.materials[i].pbrMetallicRoughness.baseColorTexture;
				texture->edit([i](graphics::Texture& tex) { tex.setTextureUnit(SplatmapMaterial::TextureUnits::kBaseColor0 + BasicMaterial::kMaxTextures * i); });
				step->addResource(std::move(texture));
			}

			step->addBindable(
				context.create<graphics::UniformVariableValue<float>>("uSMaterial.materials[" + std::to_string(i) + "].pbrMetallicRoughness.metallicFactor", material.materials[i].pbrMetallicRoughness.metallicFactor)
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(programRef)); })
			);

			step->addBindable(
				context.create<graphics::UniformVariableValue<float>>("uSMaterial.materials[" + std::to_string(i) + "].pbrMetallicRoughness.roughnessFactor", material.materials[i].pbrMetallicRoughness.roughnessFactor)
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(programRef)); })
			);

			bool useMetallicRoughnessTexture = material.materials[i].pbrMetallicRoughness.metallicRoughnessTexture;
			step->addBindable(
				context.create<graphics::UniformVariableValue<int>>("uSMaterial.materials[" + std::to_string(i) + "].pbrMetallicRoughness.useMetallicRoughnessTexture", useMetallicRoughnessTexture)
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(programRef)); })
			);

			step->addBindable(
				context.create<graphics::UniformVariableValue<int>>("uSMaterial.materials[" + std::to_string(i) + "].pbrMetallicRoughness.metallicRoughnessTexture", SplatmapMaterial::TextureUnits::kMetallicRoughness0 + BasicMaterial::kMaxTextures * i)
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(programRef)); })
			);

			if (useMetallicRoughnessTexture) {
				auto texture = material.materials[i].pbrMetallicRoughness.metallicRoughnessTexture;
				texture->edit([i](graphics::Texture& tex) { tex.setTextureUnit(SplatmapMaterial::TextureUnits::kMetallicRoughness0 + BasicMaterial::kMaxTextures * i); });
				step->addResource(std::move(texture));
			}

			bool useNormalTexture = material.materials[i].normalTexture;
			step->addBindable(
				context.create<graphics::UniformVariableValue<int>>("uSMaterial.materials[" + std::to_string(i) + "].useNormalTexture", useNormalTexture)
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(programRef)); })
			);

			step->addBindable(
				context.create<graphics::UniformVariableValue<int>>("uSMaterial.materials[" + std::to_string(i) + "].normalTexture", SplatmapMaterial::TextureUnits::kNormal0 + BasicMaterial::kMaxTextures * i)
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(programRef)); })
			);

			step->addBindable(
				context.create<graphics::UniformVariableValue<float>>("uSMaterial.materials[" + std::to_string(i) + "].normalScale", material.materials[i].normalScale)
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(programRef)); })
			);

			if (useNormalTexture) {
				auto texture = material.materials[i].normalTexture;
				texture->edit([i](graphics::Texture& tex) { tex.setTextureUnit(SplatmapMaterial::TextureUnits::kNormal0 + BasicMaterial::kMaxTextures * i); });
				step->addResource(std::move(texture));
			}
		}

		auto texture = material.splatmapTexture;
		texture->edit([](graphics::Texture& tex) { tex.setTextureUnit(SplatmapMaterial::TextureUnits::kSplatmap); });
		step->addResource(std::move(texture))
			.addBindable(
				context.create<graphics::UniformVariableValue<int>>("uSMaterial.splatmapTexture", SplatmapMaterial::TextureUnits::kSplatmap)
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(programRef)); })
			);
	}


	void ShaderLoader::readSplatmapMaterialBindables(const StepResource& step, SplatmapMaterial& material)
	{
		step->processBindables([&](const graphics::Context::BindableRef& bindable) {
			if (auto uniform = graphics::Context::TBindableRef<graphics::IUniformVariable>::from(bindable); uniform) {
				uniform.edit([&](graphics::IUniformVariable& iuv) {
					if (iuv.getName() == "uSMaterial.numMaterials") {
						material.materials.resize(dynamic_cast<graphics::UniformVariableValue<int>*>(&iuv)->getValue());
					}
				});
			}
		});

		step->processTextures([&](const TextureResource& texture) {
			texture->edit([&](graphics::Texture& tex) {
				if (tex.getTextureUnit() == SplatmapMaterial::TextureUnits::kSplatmap) {
					material.splatmapTexture = texture;
				}
				else {
					for (int i = 0; i < static_cast<int>(material.materials.size()); ++i) {
						if (tex.getTextureUnit() == SplatmapMaterial::TextureUnits::kBaseColor0 + BasicMaterial::kMaxTextures * i) {
							material.materials[i].pbrMetallicRoughness.baseColorTexture = texture;
						}
						else if (tex.getTextureUnit() == SplatmapMaterial::TextureUnits::kMetallicRoughness0 + BasicMaterial::kMaxTextures * i) {
							material.materials[i].pbrMetallicRoughness.metallicRoughnessTexture = texture;
						}
						else if (tex.getTextureUnit() == SplatmapMaterial::TextureUnits::kNormal0 + BasicMaterial::kMaxTextures * i) {
							material.materials[i].normalTexture = texture;
						}
					}
				}
			});
		});

		step->processBindables([&](const graphics::Context::BindableRef& bindable) {
			if (auto uniform = graphics::Context::TBindableRef<graphics::IUniformVariable>::from(bindable); uniform) {
				uniform.edit([&](graphics::IUniformVariable& iuv) {
					for (int i = 0; i < static_cast<int>(material.materials.size()); ++i) {
						utils::ArrayStreambuf<char, 128> aStreambuf;
						std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].pbrMetallicRoughness.baseColorFactor";
						if (iuv.getName() == aStreambuf.data()) {
							material.materials[i].pbrMetallicRoughness.baseColorFactor = dynamic_cast<graphics::UniformVariableValue<glm::vec4>*>(&iuv)->getValue();
							continue;
						}

						aStreambuf = {};
						std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].pbrMetallicRoughness.metallicFactor";
						if (iuv.getName() == aStreambuf.data()) {
							material.materials[i].pbrMetallicRoughness.metallicFactor = dynamic_cast<graphics::UniformVariableValue<float>*>(&iuv)->getValue();
							continue;
						}

						aStreambuf = {};
						std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].pbrMetallicRoughness.roughnessFactor";
						if (iuv.getName() == aStreambuf.data()) {
							material.materials[i].pbrMetallicRoughness.roughnessFactor = dynamic_cast<graphics::UniformVariableValue<float>*>(&iuv)->getValue();
							continue;
						}

						aStreambuf = {};
						std::ostream(&aStreambuf) << "uSMaterial.materials[" << i << "].normalScale";
						if (iuv.getName() == aStreambuf.data()) {
							material.materials[i].normalScale = dynamic_cast<graphics::UniformVariableValue<float>*>(&iuv)->getValue();
							continue;
						}
					}
				});
			}
		});

		step->processPrograms([&](const ProgramResource& program) {
			program->getParent()->wait();
		});
	}


	void ShaderLoader::addHeightMapBindables(
		StepResource step, const TextureResource& heightMap, const TextureResource& normalMap,
		float size, float maxHeight, const ProgramResource& program
	) {
		graphics::Context& context = *program->getParent();

		bool hasProgram = false;
		step->processPrograms([&](const ProgramResource& program2) {
			hasProgram |= (program == program2);
		});
		if (!hasProgram) {
			step->addResource(program);
		}

		auto programRef = *program;
		step->addResource(std::move(heightMap))
			.addResource(std::move(normalMap))
			.addBindable(
				context.create<graphics::UniformVariableValue<int>>("uHeightMap", SplatmapMaterial::TextureUnits::kHeightMap)
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(programRef)); })
			)
			.addBindable(
				context.create<graphics::UniformVariableValue<int>>("uNormalMap", SplatmapMaterial::TextureUnits::kNormalMap)
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(programRef)); })
			)
			.addBindable(
				context.create<graphics::UniformVariableValue<float>>("uXZSize", size)
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(programRef)); })
			)
			.addBindable(
				context.create<graphics::UniformVariableValue<float>>("uMaxHeight", maxHeight)
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(programRef)); })
			);
	}


	Result ShaderLoader::createProgram(
		const char* vertexShaderPath, const char* geometryShaderPath, const char* fragmentShaderPath, graphics::Context& context,
		ProgramRef& program
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
		program = context.create<graphics::Program>()
			.edit([=](graphics::Program& program) {
				utils::FixedVector<graphics::Shader, 3> shaders;
				if (vertexShaderPath) {
					shaders.emplace_back(vertexShaderText, graphics::ShaderType::Vertex);
				}
				if (geometryShaderPath) {
					shaders.emplace_back(geometryShaderText, graphics::ShaderType::Geometry);
				}
				if (fragmentShaderPath) {
					shaders.emplace_back(fragmentShaderText, graphics::ShaderType::Fragment);
				}

				utils::FixedVector<const graphics::Shader*, 3> shaderPtrs;
				for (auto& shader : shaders) {
					shaderPtrs.push_back(&shader);
				}

				program.load(shaderPtrs.data(), shaderPtrs.size());
			});

		return Result();
	}

}
