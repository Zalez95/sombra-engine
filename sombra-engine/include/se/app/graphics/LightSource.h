#ifndef LIGHT_SOURCE_H
#define LIGHT_SOURCE_H

#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

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

		/** The name of the LightSource */
		std::string name;

		/** The type of the LightSource */
		Type type;

		/** The LightSource color */
		glm::vec3 color = glm::vec3(1.0f);

		/** The brightness of the LightSource */
		float intensity = 1.0f;

		/** The inverse of the distance where the LightSource intensity is
		 * considered to be zero (only for PointLights and SporLights) */
		float inverseRange = 0.0f;

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

}

#endif		// LIGHT_SOURCE_H
