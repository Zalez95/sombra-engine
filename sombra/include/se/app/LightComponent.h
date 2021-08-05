#ifndef LIGHT_COMPONENT_H
#define LIGHT_COMPONENT_H

#include <glm/gtc/constants.hpp>
#include "graphics/RenderableLight.h"
#include "graphics/RenderableShader.h"
#include "Entity.h"

namespace se::app {

	/**
	 * Class LightSource, it holds the common properties for all the light
	 * sources
	 */
	class LightSource :
		public std::enable_shared_from_this<LightSource>
	{
	public:		// Nested types
		/** The different types that a LightSource can have */
		enum class Type : unsigned int
		{ Directional = 0, Point, Spot };

	private:	// Attributes
		/** The EventManager used for notifying of LightSource updates */
		EventManager& mEventManager;

		/** The type of the LightSource */
		Type mType;

		/** The LightSource color */
		glm::vec3 mColor = glm::vec3(1.0f);

		/** The brightness of the LightSource */
		float mIntensity = 1.0f;

		/** The distance from the LightSource where its intensity is considered
		 * to be zero */
		float mRange = 50.0f;

		/** Angle in radians from the center of the LightSource where the
		 * falloff begins (Only for SpotLights) */
		float mInnerConeAngle = 0.0f;

		/** Angle in radians from the center of the LightSource where the
		 * falloff ends (Only for SpotLights) */
		float mOuterConeAngle = glm::quarter_pi<float>();

		/** If the LightSource will cast shadows or not */
		bool mCastShadows = false;

		/** The resolution of each side of the Texture where the Shadows
		 * are going to be rendered */
		std::size_t mShadowResolution = 1024;

		/** The length of the square that is going to be covered by the
		 * shadows */
		float mShadowSize = 50.0f;

		/** The distance to the near plane of the camera used for rendering
		 * the shadows */
		float mShadowZNear = 0.1f;

		/** The distance to the far plane of the camera used for rendering
		 * the shadows */
		float mShadowZFar = 10.0f;

		/** The number of Cascades for the Shadow mapping (only for
		 * DirectionalLights). Maximum 6 per light. */
		std::size_t mNumCascades = 1;

	public:		// Functions
		/** Creates a new LightSource
		 *
		 * @param	eventManager the EventManager used for notifying of
		 *			RenderableShader updates
		 * @param	type the Type of the LightSource */
		LightSource(EventManager& eventManager, Type type = Type::Directional) :
			mEventManager(eventManager), mType(type) {};

		/** @return	the Type of the LightSource */
		Type getType() const { return mType; };

		/** Sets the Type of the LightSource */
		void setType(Type type);

		/** @return	the color of the LightSource */
		const glm::vec3& getColor() const { return mColor; };

		/** Sets the Color of the LightSource
		 *
		 * @param	color the new Color of the LightSource */
		void setColor(const glm::vec3& color) { mColor = color; };

		/** @return	the intensity of the LightSource */
		float getIntensity() const { return mIntensity; };

		/** Sets the intensity of the LightSource
		 *
		 * @param	intensity the new intensity of the LightSource */
		void setIntensity(float intensity) { mIntensity = intensity; };

		/** @return	the range of the LightSource */
		float getRange() const { return mRange; };

		/** Sets the range of the LightSource
		 *
		 * @param	range the new Range of the LightSource */
		void setRange(float range) { mRange = range; };

		/** Returns the range of the LightSource
		 *
		 * @param	innerConeAngle return parameter for the angle where the
		 *			falloff begins
		 * @param	outerConeAngle return parameter for the angle where the
		 *			falloff ends */
		void getSpotLightRange(
			float& innerConeAngle, float& outerConeAngle
		) const
		{ innerConeAngle = mInnerConeAngle; outerConeAngle = mOuterConeAngle; };

		/** Sets the range of the LightSource
		 *
		 * @param	innerConeAngle the angle in radians from the center of the
		 *			LightSource where the falloff begins
		 * @param	outerConeAngle the angle in radians from the center of the
		 *			LightSource where the falloff ends
		 * @note	it only affects SpotLights */
		void setSpotLightRange(float innerConeAngle, float outerConeAngle)
		{ mInnerConeAngle = innerConeAngle; mOuterConeAngle = outerConeAngle; };

		/** @return	true if the LightSource casts Shadows, false otherwise */
		bool castsShadows() const { return mCastShadows; };

		/** Returns the Shadow configuration of the LightSource
		 *
		 * @param	shadowResolution the resolution of the Shadow maps
		 * @param	shadowZNear the Z-near value of the shadow cameras
		 * @param	shadowZFar the Z-far value of the shadow cameras
		 * @param	shadowSize the length of the square covered by the shadows
		 *			(only for directional lights)
		 * @param	numCascades the number of cascades for CSM (only for
		 *			directional and spot lights) */
		void getShadows(
			std::size_t& shadowResolution,
			float& shadowZNear, float& shadowZFar,
			float& shadowSize, std::size_t& numCascades
		) const;

		/** Sets the Shadow configuration of the LightSource
		 *
		 * @param	shadowResolution the resolution of the Shadow maps
		 * @param	shadowZNear the Z-near value of the shadow cameras
		 * @param	shadowZFar the Z-far value of the shadow cameras
		 * @param	shadowSize the length of the square covered by the shadows
		 *			(only for directional lights)
		 * @param	numCascades the number of cascades for CSM (only for
		 *			DirectionalLights) */
		void setShadows(
			std::size_t shadowResolution = 1024,
			float shadowZNear = 0.1f, float shadowZFar = 10.0f,
			float shadowSize = 50.0f, std::size_t numCascades = 1
		);

		/** Disables the Shadows */
		void disableShadows();
	};


	/**
	 * Struct LightComponent, it's a Component that holds the RenderableLight
	 * of an Entity and it's realted LightSource.
	 */
	class LightComponent
	{
	private:	// Nested types
		using RenderableShaderRef = Repository::ResourceRef<RenderableShader>;

	private:	// Attributes
		/** The EventManager used for notifying the MeshComponent changes */
		EventManager* mEventManager = nullptr;

		/** The Entity that owns the MeshComponent */
		Entity mEntity = kNullEntity;

		/** The Renderable that is going to be used for drawing the ligth */
		RenderableLight mRenderable;

		/** A pointer to the LightSource that is going to be used */
		Repository::ResourceRef<LightSource> mSource;

		/** The shaders added to the LightComponent */
		std::vector<RenderableShaderRef> mShaders;

	public:		// Functions
		/** Creates a new LightComponent */
		LightComponent() = default;
		LightComponent(const LightComponent& other);
		LightComponent(LightComponent&& other) = default;

		/** Class destructor */
		~LightComponent() = default;

		/** Assignment operator */
		LightComponent& operator=(const LightComponent& other);
		LightComponent& operator=(LightComponent&& other) = default;

		/** Sets the LightComponent attributes
		 *
		 * @param	eventManager the new EventManager of the LightComponent
		 * @param	entity the new Entity of the LightComponent */
		void setup(EventManager* eventManager, Entity entity)
		{
			mEventManager = eventManager;
			mEntity = entity;
		};

		/** @return	the RenderableLight of the LightComponent */
		RenderableLight& getRenderable() { return mRenderable; };

		/** @return	the RenderableLight of the LightComponent */
		const RenderableLight& getRenderable() const { return mRenderable; };

		/** @return	a reference to the LightSource of the LightComponent */
		const Repository::ResourceRef<LightSource>& getSource() const
		{ return mSource; };

		/** Sets the LightSource of the LightComponent
		 *
		 * @param	source the new LightSource of the Component
		 * @note	Changing the Source of the Component will remove the
		 *			LightComponent Shadows */
		void setSource(const Repository::ResourceRef<LightSource>& source);

		/** Adds the given RenderableShader to the RenderableMesh of the
		 * RenderableLight of the LightComponent
		 *
		 * @param	shader a pointer to the shader to add */
		void addRenderableShader(const RenderableShaderRef& shader);

		/** Iterates through all the RenderableShaders of the RenderableMesh
		 * of the RenderableLight of the LightComponent calling the given
		 * callback function
		 *
		 * @param	callback the function to call for each RenderableShader */
		template <typename F>
		void processRenderableShaders(F&& callback) const;

		/** Removes the given RenderableShader from the RenderableMesh of the
		 * RenderableLight of the LightComponent
		 *
		 * @param	shader a pointer to the shader to remove */
		void removeRenderableShader(const RenderableShaderRef& shader);
	};


	template <typename F>
	void LightComponent::processRenderableShaders(F&& callback) const
	{
		for (auto& shader : mShaders) {
			callback(shader);
		}
	}

}

#endif		// LIGHT_COMPONENT_H
