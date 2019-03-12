#ifndef LIGHTS_H
#define LIGHTS_H

#include <glm/glm.hpp>

namespace se::graphics {

	/**
	 * Struct BaseLight, it holds the common properties for all the Lights
	 */
	struct BaseLight
	{
		glm::vec3 lightColor;
	};


	/**
	 * Struct Attenuation, it holds the data for the fading effect of some of
	 * the Lights
	 */
	struct Attenuation
	{
		float constant;
		float linear;
		float exponential;
	};


	/**
	 * Struct DirectionalLight, it's a Light source that shines in one
	 * direction without fading
	 */
	struct DirectionalLight
	{
		BaseLight	mBase;
		glm::vec3	mDirection;
	};


	/**
	 * Struct PointLight, it's a light source that has a position and shines
	 * in all directions with a fading effect called attenuation
	 */
	struct PointLight
	{
		BaseLight base;
		Attenuation attenuation;
		glm::vec3 position;
	};


	/**
	 * Struct SpotLight, it's a Light source that has a position and
	 * shines in one direction with a fading effect called attenuation.
	 *
	 * The effect of a SpotLight lights only within a limited cone that
	 * grows wider as light moves further away from the light source
	 */
	struct SpotLight
	{
		const PointLight base;
		glm::vec3 direction;
		float cutoff;
	};

}

#endif		// LIGHTS_H
