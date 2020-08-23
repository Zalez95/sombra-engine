#include <algorithm>
#include "se/graphics/3D/Renderable3D.h"

namespace se::graphics {

	Renderable3D& Renderable3D::addBindable(BindableSPtr bindable)
	{
		if (bindable) {
			mBindables.push_back(bindable);
		}

		return *this;
	}


	void Renderable3D::processBindables(const BindableCallback& callback)
	{
		for (auto& bindable : mBindables) {
			callback(bindable);
		}
	}


	Renderable3D& Renderable3D::removeBindable(BindableSPtr bindable)
	{
		mBindables.erase(std::remove(mBindables.begin(), mBindables.end(), bindable), mBindables.end());

		return *this;
	}


	void Renderable3D::bind() const
	{
		for (auto& bindable : mBindables) {
			bindable->bind();
		}
	}


	void Renderable3D::unbind() const
	{
		for (auto itBindable = mBindables.rbegin(); itBindable != mBindables.rend(); ++itBindable) {
			(*itBindable)->unbind();
		}
	}

}
