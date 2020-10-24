#ifndef LIGHT_PROBE_H
#define LIGHT_PROBE_H

#include <memory>
#include "../graphics/core/Texture.h"

namespace se::app {

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

}

#endif		// LIGHT_PROBE_H
