#include <algorithm>
#include "se/graphics/Renderable.h"
#include "se/graphics/Technique.h"

namespace se::graphics {

	void Renderable::addTechnique(TechniqueSPtr technique)
	{
		if (technique) {
			mTechniques.push_back(technique);
		}
	}


	void Renderable::processTechniques(const TechniqueCallback& callback)
	{
		for (auto& technique : mTechniques) {
			callback(technique);
		}
	}


	void Renderable::removeTechnique(TechniqueSPtr technique)
	{
		mTechniques.erase(
			std::remove(mTechniques.begin(), mTechniques.end(), technique),
			mTechniques.end()
		);
	}


	void Renderable::submit()
	{
		for (auto& technique : mTechniques) {
			technique->submit(*this);
		}
	}

}
