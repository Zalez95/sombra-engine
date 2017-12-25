#ifndef LIGHTS_H
#define LIGHTS_H

#include <glm/glm.hpp>
#include "../PrimitiveTypes.h"

namespace fe { namespace graphics {

	/**
	 * Class BaseLight, it's the common class for all the Lights
	 */
	class BaseLight
	{
	private:	// Attributes
		RGBColor	mDiffuseColor;
		RGBColor	mSpecularColor;

	public:		// Functions
		/** Creates a new BaseLight
		 *
		 * @param	diffuseColor the diffuse color that emits the light
		 * @param	specularColor the specular color that emits the light */
		BaseLight(
			const RGBColor& diffuseColor,
		   	const RGBColor& specularColor
		) : mDiffuseColor(diffuseColor), mSpecularColor(specularColor) {};

		/** Class destructor */
		~BaseLight() {};

		/** @return the specular color that emits the light */
		inline RGBColor getSpecularColor() const { return mSpecularColor; };

		/** @return the diffuse color that emits the light */
		inline RGBColor getDiffuseColor() const { return mDiffuseColor; };
	};


	/**
	 * Struct Attenuation, it holds the data for the fading effect of some of
	 * the Lights
	 */
	struct Attenuation
	{
		float	mConstant;
		float	mLinear;
		float	mExponential;
	};


	/**
	 * Class DirectionalLight, it's a Light source that shines in one
	 * direction without fading
	 */
	class DirectionalLight
	{
	private:	// Attributes
		BaseLight	mBase;
		glm::vec3	mDirection;

	public:		// Functions
		/** Creates a new DirectionalLight
		 *
		 * @param	baseLight the basis Light of the DirectionalLight
		 * @param	directional the direction of the DirectionalLight */
		DirectionalLight(
			const BaseLight& baseLight, const glm::vec3& direction
		) :mBase(baseLight), mDirection(direction) {};

		/** Class destructor */
		~DirectionalLight() {};
	};


	/**
	 * Class PointLight, it's a light source that has a position and shines
	 * in all directions with a fading effect called attenuation
	 */
	class PointLight
	{
	private:	// Attributes
		BaseLight	mBase;
		Attenuation mAttenuation;
		glm::vec3	mPosition;

	public:		// Functions
		/** Creates a new PointLight
		 *
		 * @param	baseLight the basis PointLight of the PointLight
		 * @param	position the position of the DirectionalLight */
		PointLight(
			const BaseLight& baseLight,
			const Attenuation& attenuation,
			const glm::vec3& position
		) : mBase(baseLight), mAttenuation(attenuation),
			mPosition(position) {};

		/** Class destructor */
		~PointLight() {};

		/** @return the base light of the Point Light */
		inline BaseLight getBaseLight() const { return mBase; };

		/** @return the attenuation of the light */
		inline Attenuation getAttenuation() const { return mAttenuation; };

		/** @return the position of the light */
		inline glm::vec3 getPosition() const { return mPosition; };

		/** Sets the position of the Light
		 *
		 * @param	position the new position of the Light */
		inline void setPosition(const glm::vec3& position)
		{ mPosition = position; };
	};


	/**
	 * Class SpotLight, it's a Light source that has a position and
	 * shines in one direction with a fading effect called attenuation.
	 * <br>The effect of a SpotLight lights only within a limited cone that
	 * grows wider as light moves further away from the light source
	 */
	class SpotLight
	{
	private:	// Attributes
		const PointLight mBase;
		glm::vec3 mDirection;
		float mCutoff;

	public:		// Functions
		/** Creates a new SpotLight
		 *
		 * @param	baseLight the basis PointLight of the SpotLight
		 * @param	direction the direction of the SpotLight */
		SpotLight(
			const PointLight& baseLight, const glm::vec3& direction
		) : mBase(baseLight), mDirection(direction) {};

		/** Class destructor */
		~SpotLight() {};
	};

}}

#endif		// LIGHTS_H
