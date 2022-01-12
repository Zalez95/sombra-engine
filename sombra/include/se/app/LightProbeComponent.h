#ifndef LIGHT_PROBE_COMPONENT_H
#define LIGHT_PROBE_COMPONENT_H

#include "graphics/TypeRefs.h"
#include "Repository.h"

namespace se::app {

	/**
	 * Struct LightProbeComponent, it holds all the environment lighting
	 * textures
	 */
	struct LightProbeComponent
	{
		/** The irradiance texture */
		Repository::ResourceRef<TextureRef> irradianceMap;

		/** The prefiltered environment map texture */
		Repository::ResourceRef<TextureRef> prefilterMap;
	};

}

#endif		// LIGHT_PROBE_COMPONENT_H
