#ifndef LIGHT_PROBE_COMPONENT_H
#define LIGHT_PROBE_COMPONENT_H

#include "../graphics/core/Texture.h"
#include "Repository.h"

namespace se::app {

	/**
	 * Struct LightProbeComponent, it holds all the environment lighting
	 * textures
	 */
	struct LightProbeComponent
	{
		/** The irradiance texture */
		Repository::ResourceRef<graphics::Texture> irradianceMap;

		/** The prefiltered environment map texture */
		Repository::ResourceRef<graphics::Texture> prefilterMap;
	};

}

#endif		// LIGHT_PROBE_COMPONENT_H
