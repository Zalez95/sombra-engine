#ifndef MATERIAL_H
#define MATERIAL_H

#include <memory>
#include <glm/glm.hpp>
#include "../../utils/FixedVector.h"
#include "../../graphics/core/Texture.h"

namespace se::app {

	/**
	 * Struct PBRMetallicRoughness, It holds all the parameters related to the
	 * metallic-roughness of a material
	 */
	struct PBRMetallicRoughness
	{
		/** The base color factor */
		glm::vec4 baseColorFactor = glm::vec4(1.0f);

		/** The base color texture */
		std::shared_ptr<graphics::Texture> baseColorTexture;

		/** The metalness of the material */
		float metallicFactor = 1.0f;

		/** The roughness of the material */
		float roughnessFactor = 1.0f;

		/** The metallic-roughness texture */
		std::shared_ptr<graphics::Texture> metallicRoughnessTexture;
	};


	/**
	 * Struct Material, it holds the surface material properties of a
	 * Renderable3D, and is used for the PBR materials of the shaders
	 */
	struct Material
	{
		/** The TextureUnits of the Materials */
		struct TextureUnits
		{
			static constexpr int kBaseColor			= 3;
			static constexpr int kMetallicRoughness	= 4;
			static constexpr int kNormal			= 5;
			static constexpr int kOcclusion			= 6;
			static constexpr int kEmissive			= 7;
		};

		/** The PBRMetallicRoughness data of the Material */
		PBRMetallicRoughness pbrMetallicRoughness;

		/** The normal map texture */
		std::shared_ptr<graphics::Texture> normalTexture;

		/** The scale applied to the normal map texture */
		float normalScale = 1.0f;

		/** The occlusion map texture */
		std::shared_ptr<graphics::Texture> occlusionTexture;

		/** The amount of occlusion applied */
		float occlusionStrength = 1.0f;

		/** The emissive map texture */
		std::shared_ptr<graphics::Texture> emissiveTexture;

		/** The RGB components of the emissive color of the material */
		glm::vec3 emissiveFactor = glm::vec3(0.0f);

		/** Indicates how the renderer should interpret the alpha value of the
		 * baseColorFactor property of the PBRMetallicRoughness */
		graphics::AlphaMode alphaMode = graphics::AlphaMode::Opaque;

		/** When the AlphaMode is set to Mask specifies the cutoff threshold */
		float alphaCutoff = 0.5f;

		/** Indicates if each face of the Mesh should be rendered by both
		 * sides */
		bool doubleSided = false;
	};


	/**
	 * Struct BasicMaterial, it holds surface material properties.
	 * It's a simplified version of the Material so it can be used with
	 * splatmaps
	 */
	struct BasicMaterial
	{
		/** The number of textures in each BasicMaterial */
		static constexpr int kMaxTextures = 3;

		/** The PBRMetallicRoughness data of the Material */
		PBRMetallicRoughness pbrMetallicRoughness;

		/** The normal map texture */
		std::shared_ptr<graphics::Texture> normalTexture;

		/** The scale applied to the normal map texture */
		float normalScale;
	};


	/**
	 * Struct SplatmapMaterial, it holds up to 4 BasicMaterials that can be
	 * combined with its splatmap
	 */
	struct SplatmapMaterial
	{
		/** The TextureUnits of the Materials */
		struct TextureUnits {
			static constexpr int kHeightMap				= 0;
			static constexpr int kNormalMap				= 1;
			static constexpr int kSplatmap				= 2;
			static constexpr int kBaseColor0			= 3;
			static constexpr int kMetallicRoughness0	= 4;
			static constexpr int kNormal0				= 5;
		};

		/** The different BasicMaterials that can be combined */
		utils::FixedVector<BasicMaterial, 4> materials;

		/** The splatmap texture, its a RGBA textures that codifies in each of
		 * its channels which material should be used at each position and in
		 * what percentage */
		std::shared_ptr<graphics::Texture> splatmapTexture;
	};

}

#endif		// MATERIAL_H
