#ifndef LIGHT_COMPONENT_H
#define LIGHT_COMPONENT_H

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include "../graphics/core/Texture.h"
#include "../graphics/3D/RenderableMesh.h"
#include "events/EventManager.h"
#include "Entity.h"
#include "Repository.h"

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
		LightSource(Type type = Type::Directional) : type(type) {};
	};


	/**
	 * Struct ShadowData, it holds the configuration used for renfering the
	 * Shadows
	 */
	struct ShadowData
	{
		/** The maximum number of shadows casted by a light */
		static const std::size_t kMaxShadowsPerLight = 6;

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

		/** The number of Cascades for the Shadow (only if it isn't a
		 * PointLight). Maximum 6 per light. */
		std::size_t numCascades = 1;
	};


	/**
	 * Struct LightComponent, holds a pointer to the LightSource that is
	 * going to be used by the Entity
	 */
	class LightComponent
	{
	private:	// Attributes
		/** The EventManager used for notifying the LightComponent changes */
		EventManager* mEventManager = nullptr;

		/** The Entity that owns the LightComponent */
		Entity mEntity = kNullEntity;

		/** The RenderableMesh used for rendering the LightComponent */
		graphics::RenderableMesh mRMesh;

		/** A pointer to the LightSource that is going to be used */
		Repository::ResourceRef<LightSource> mSource;

		/** The data used for rendering the shadows */
		std::unique_ptr<ShadowData> mShadowData;

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

		/** Sets the MeshComponent attributes
		 *
		 * @param	eventManager the new EventManager of the MeshComponent
		 * @param	entity the new Entity of the MeshComponent */
		void setup(EventManager* eventManager, Entity entity);

		/** @return	the RenderableMesh used for rendering the LightComponent */
		graphics::RenderableMesh& getLightRenderable() { return mRMesh; };

		/** @return	a reference to the LightSource of the LightComponent */
		const Repository::ResourceRef<LightSource>& getSource() const
		{ return mSource; };

		/** Sets the LightSource of the LightComponent
		 *
		 * @param	source the new LightSource of the Component
		 * @note	Changing the Source of the Component will remove the
		 *			LightComponent Shadows */
		void setSource(const Repository::ResourceRef<LightSource>& source);

		/** @return	true if the LightComponent casts Shadows or not */
		bool hasShadows() const { return mShadowData.get(); };

		/** @return	the configuration of the Shadows */
		const ShadowData* getShadowData() const { return mShadowData.get(); };

		/** Sets the ShadowData of the LightComponent, enabling the Shadow
		 * casting from the LightComponent if it didn't casted shadows yet.
		 * If data is nullptr it disables casting shadows.
		 *
		 * @param	data the new ShadowData of the LightComponent */
		void setShadowData(std::unique_ptr<ShadowData> data);
	};


	/**
	 * Struct LightProbe, it holds all the environment lighting textures
	 */
	struct LightProbe
	{
		/** The irradiance texture */
		Repository::ResourceRef<graphics::Texture> irradianceMap;

		/** The prefiltered environment map texture */
		Repository::ResourceRef<graphics::Texture> prefilterMap;
	};

}

#endif		// LIGHT_COMPONENT_H
