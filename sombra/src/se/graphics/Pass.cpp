#include <algorithm>
#include "se/graphics/Pass.h"
#include "se/graphics/Renderer.h"

namespace se::graphics {

	void Pass::submit(Renderable& renderable)
	{
		mRenderer->submit(renderable, *this);
	}


	Pass& Pass::addBindable(const Context::BindableRef& bindable)
	{
		if (bindable) {
			mBindables.push_back(bindable);
		}

		return *this;
	}


	Pass& Pass::removeBindable(const Context::BindableRef& bindable)
	{
		mBindables.erase(std::remove(mBindables.begin(), mBindables.end(), bindable), mBindables.end());

		return *this;
	}


	void Pass::bind(Context::Query& q) const
	{
		for (auto& bindable : mBindables) {
			q.getBindable(bindable)->bind();
		}
	}


	void Pass::unbind(Context::Query& q) const
	{
		for (auto itBindable = mBindables.rbegin(); itBindable != mBindables.rend(); ++itBindable) {
			q.getBindable(*itBindable)->unbind();
		}
	}

}
