#include <algorithm>
#include "se/graphics/3D/Renderer3D.h"
#include "se/graphics/3D/Renderable3D.h"

namespace se::graphics {

	Renderer3D& Renderer3D::addFilter(FilterSPtr filter)
	{
		mFilters.emplace_back(std::move(filter));
		return *this;
	}


	Renderer3D& Renderer3D::removeFilter(FilterSPtr filter)
	{
		mFilters.erase(std::remove(mFilters.begin(), mFilters.end(), filter), mFilters.end());
		return *this;
	}


	void Renderer3D::submit(Renderable& renderable, Pass& pass)
	{
		auto renderable3D = dynamic_cast<Renderable3D*>(&renderable);
		if (renderable3D
			&& std::all_of(mFilters.begin(), mFilters.end(), [renderable3D](FilterSPtr filter) {
				return filter->shouldBeRendered(*renderable3D);
			})
		) {
			submitRenderable3D(*renderable3D, pass);
		}
	}

}
