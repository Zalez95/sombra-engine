#include "se/graphics/2D/Step2D.h"
#include "se/graphics/2D/Renderer2D.h"
#include "se/graphics/2D/Renderable2D.h"

namespace se::graphics {

	void Step2D::submit(Renderable& renderable)
	{
		auto renderable2D = dynamic_cast<Renderable2D*>(&renderable);
		if (renderable2D) {
			mRenderer.submit(*renderable2D, *this);
		}
	}

}
