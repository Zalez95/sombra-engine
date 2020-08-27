#ifndef LIGHT_COMPONENT_H
#define LIGHT_COMPONENT_H

#include <memory>
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

		/** The type of the LightSource */
		Type type;

		/** The LightSource color */
		glm::vec3 color = glm::vec3(1.0f);

		/** The brightness of the LightSource */
		float intensity = 1.0f;

		/** The distance from the LightSource where its intensity is considered
		 * to be zero (only for PointLights and SpotLights) */
		float range = std::numeric_limits<float>::max();

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

}

#endif		// LIGHT_COMPONENT_H
