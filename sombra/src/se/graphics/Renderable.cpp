#include <algorithm>
#include "se/graphics/Renderable.h"
#include "se/graphics/Technique.h"

namespace se::graphics {

	Renderable& Renderable::addTechnique(TechniqueSPtr technique)
	{
		if (technique) {
			mTechniques.push_back(technique);
		}

		return *this;
	}


	Renderable& Renderable::removeTechnique(TechniqueSPtr technique)
	{
		mTechniques.erase(
			std::remove(mTechniques.begin(), mTechniques.end(), technique),
			mTechniques.end()
		);

		return *this;
	}


	void Renderable::submit()
	{
		for (auto& technique : mTechniques) {
			technique->submit(*this);
		}
	}

}
