#include <algorithm>
#include "se/graphics/Technique.h"
#include "se/graphics/Pass.h"

namespace se::graphics {

	Technique& Technique::addPass(PassSPtr pass)
	{
		if (pass) {
			mPasses.emplace_back(std::move(pass));
		}

		return *this;
	}


	Technique& Technique::removePass(PassSPtr pass)
	{
		mPasses.erase(std::remove(mPasses.begin(), mPasses.end(), pass), mPasses.end());

		return *this;
	}


	void Technique::submit(Renderable& renderable)
	{
		for (auto& pass : mPasses) {
			pass->submit(renderable);
		}
	}

}
