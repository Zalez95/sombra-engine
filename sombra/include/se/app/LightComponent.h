#ifndef LIGHT_COMPONENT_H
#define LIGHT_COMPONENT_H

#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include "../graphics/core/Texture.h"

namespace se::app {

	/**
	 * Struct LightSource, it holds the common properties for all the light
	 * sources
	 */
	struct LightSource
	{
		/** The different types that a LightSource can have */
		enum class Type : unsigned int
		{ Directional = 0, Point, Spot };

		/** The type of the LightSource */
		Type type;

		/** The LightSource color */
		glm::vec3 color = glm::vec3(1.0f);

		/** The brightness of the LightSource */
		float intensity = 1.0f;

		/** The distance from the LightSource where its intensity is considered
		 * to be zero (only for PointLights and SpotLights) */
		float range = 50.0f;

		/** Angle in radians from the center of the LightSource where the
		 * falloff begins (Only for SpotLights) */
		float innerConeAngle = 0.0f;

		/** Angle in radians from the center of the LightSource where the
		 * falloff ends (Only for SpotLights) */
		float outerConeAngle = glm::quarter_pi<float>();

		/** Creates a new LightSource
		 *
		 * @param	type the Type of the LightSource */
		LightSource(Type type) : type(type) {};
	};


	/** Struct LightComponent, holds a pointer to the LightSource that is
	 * going to be used by the Entity */
	struct LightComponent
	{
		/** A pointer to the LightSource that is going to be used */
		std::shared_ptr<LightSource> source;
	};


	/**
	 * Struct LightProbe, it holds all the environment lighting textures
	 */
	struct LightProbe
	{
		/** The irradiance texture */
		std::shared_ptr<graphics::Texture> irradianceMap;

		/** The prefiltered environment map texture */
		std::shared_ptr<graphics::Texture> prefilterMap;
	};


	/**
	 * Struct ShadowData, it holds the configuration used for renfering the
	 * Shadows
	 */
	struct ShadowData
	{
		/** The resolution of each side of the the Texture where the Shadows
		 * are going to be rendered */
		std::size_t resolution = 1024;

		/** The length of the square that is going to be covered by shadows */
		float size = 50.0f;

		/** The distance to the near plane of the camera used for rendering the
		 * shadows */
		float zNear = 0.1f;

		/** The distance to the far plane of the camera used for rendering the
		 * shadows */
		float zFar = 10.0f;
	};

}

#endif		// LIGHT_COMPONENT_H
