#ifndef LIGHTS_H
#define LIGHTS_H

#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace se::graphics {

	/**
	 * Struct BaseLight, it holds the common properties for all the Lights
	 */
	struct ILight
	{
		/** The light name */
		std::string name;

		/** The light color */
		glm::vec3 color = glm::vec3(1.0f);

		/** The brightness of the light */
		float intensity = 1.0f;

		/** Class destructor */
		virtual ~ILight() = default;
	};


	/**
	 * Struct DirectionalLight, it's a Light source that shines in one
	 * direction without fading
	 */
	struct DirectionalLight : public ILight
	{
		/** The direction where the light points to */
		glm::vec3 direction = glm::vec3(0.0f, 0.0f, 1.0f);
	};


	/**
	 * Struct PointLight, it's a light source that has a position and shines
	 * in all directions with a fading effect
	 */
	struct PointLight : public ILight
	{
		/** The position of the light */
		glm::vec3 position = glm::vec3(0.0f);

		/** The inverse of the distance where the light intensity is considered
		 * to be zero */
		float inverseRange = 0.0f;
	};


	/**
	 * Struct SpotLight, it's a Light source that has a position and
	 * shines in one direction with a fading effect
	 *
	 * The effect of a SpotLight lights only within a limited cone that
	 * grows wider as light moves further away from the light source
	 */
	struct SpotLight : public ILight
	{
		/** The direction where the light points to */
		glm::vec3 direction = glm::vec3(0.0f, 0.0f, 1.0f);

		/** The position of the light */
		glm::vec3 position = glm::vec3(0.0f);

		/** The inverse of the distance where the light intensity is considered
		 * to be zero */
		float inverseRange = 0.0f;

		/** Angle in radians from the center of the SpotLight where the falloff
		 * begins */
		float innerConeAngle = 0.0f;

		/** Angle in radians from the center of the SpotLight where the falloff
		 * ends */
		float outerConeAngle = glm::quarter_pi<float>();
	};

}

#endif		// LIGHTS_H
