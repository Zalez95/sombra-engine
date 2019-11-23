#ifndef MATERIAL_H
#define MATERIAL_H

#include <memory>
#include <string>
#include <glm/glm.hpp>
#include "../Texture.h"
#include "../Constants.h"

namespace se::graphics {

	using TextureSPtr = std::shared_ptr<Texture>;


	/**
	 * Struct PBRMetallicRoughness, It holds all the parameters related to the
	 * metallic-roughness of a material
	 */
	struct PBRMetallicRoughness
	{
		/** The base color factor */
		glm::vec4 baseColorFactor;

		/** The base color texture */
		TextureSPtr baseColorTexture;

		/** The metalness of the material */
		float metallicFactor;

		/** The roughness of the material */
		float roughnessFactor;

		/** The metallic-roughness texture */
		TextureSPtr metallicRoughnessTexture;
	};


	/**
	 * Struct Material, it holds the surface material properties of a
	 * Renderable3D, and is used for the PBR materials of the shaders
	 */
	struct Material
	{
		/** The name of the Material */
		std::string name;

		/** The PBRMetallicRoughness data of the Material */
		PBRMetallicRoughness pbrMetallicRoughness;

		/** The normal map texture */
		TextureSPtr normalTexture;

		/** The scale applied to the normal map texture */
		float normalScale;

		/** The occlusion map texture */
		TextureSPtr occlusionTexture;

		/** The amount of occlusion applied */
		float occlusionStrength;

		/** The emissive map texture */
		TextureSPtr emissiveTexture;

		/** The RGB components of the emissive color of the material */
		glm::vec3 emissiveFactor;

		/** Indicates how the renderer should interpret the alpha value of the
		 * baseColorFactor property of the PBRMetallicRoughness */
		enum AlphaMode alphaMode;

		/** When the AlphaMode is set to Mask specifies the cutoff threshold */
		float alphaCutoff;

		/** Indicates if each face of the Mesh should be rendered by both
		 * sides */
		bool doubleSided;
	};

}

#endif		// MATERIAL_H
