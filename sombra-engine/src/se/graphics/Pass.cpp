#include <algorithm>
#include "se/graphics/Pass.h"
#include "se/graphics/Renderer.h"

namespace se::graphics {

	void Pass::submit(Renderable& renderable)
	{
		mRenderer.submit(renderable, *this);
	}


	Pass& Pass::addBindable(BindableSPtr bindable)
	{
		if (bindable) {
			mBindables.push_back(bindable);
		}

		return *this;
	}


	void Pass::processBindables(const BindableCallback& callback)
	{
		for (auto& bindable : mBindables) {
			callback(bindable);
		}
	}


	Pass& Pass::removeBindable(BindableSPtr bindable)
	{
		mBindables.erase(std::remove(mBindables.begin(), mBindables.end(), bindable), mBindables.end());

		return *this;
	}


	void Pass::bind() const
	{
		for (auto& bindable : mBindables) {
			bindable->bind();
		}
	}


	void Pass::unbind() const
	{
		for (auto itBindable = mBindables.rbegin(); itBindable != mBindables.rend(); ++itBindable) {
			(*itBindable)->unbind();
		}
	}

}
