#include "se/graphics/3D/Step3D.h"
#include "se/graphics/3D/Renderer3D.h"
#include "se/graphics/3D/Renderable3D.h"

namespace se::graphics {

	void Step3D::submit(Renderable& renderable)
	{
		auto renderable3D = dynamic_cast<Renderable3D*>(&renderable);
		if (renderable3D) {
			mRenderer.submit(*renderable3D, *this);
		}
	}

}
