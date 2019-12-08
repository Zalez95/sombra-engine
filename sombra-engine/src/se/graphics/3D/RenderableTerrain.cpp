#include "se/graphics/3D/RenderableTerrain.h"
#include "se/graphics/3D/Camera.h"

namespace se::graphics {

	void RenderableTerrain::update(const Camera& camera)
	{
		mQuadTree.updateHighestLodLocation(camera.getPosition());
	}

}
