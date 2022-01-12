#include <algorithm>
#include "se/graphics/3D/Renderable3D.h"

namespace se::graphics {

	Renderable3D& Renderable3D::addPassBindable(Pass* pass, const Context::BindableRef& bindable)
	{
		if (pass && bindable) {
			mPassBindables[pass].push_back(bindable);
		}

		return *this;
	}


	Renderable3D& Renderable3D::clearBindables(Pass* pass)
	{
		auto it = mPassBindables.find(pass);
		if (it != mPassBindables.end()) {
			mPassBindables.erase(it);
		}

		return *this;
	}


	Renderable3D& Renderable3D::removePassBindable(Pass* pass, const Context::BindableRef& bindable)
	{
		auto it = mPassBindables.find(pass);
		if (it != mPassBindables.end()) {
			it->second.erase(std::remove(it->second.begin(), it->second.end(), bindable), it->second.end());
			if (it->second.empty()) {
				mPassBindables.erase(it);
			}
		}

		return *this;
	}


	void Renderable3D::bind(Context::Query& q, Pass* pass) const
	{
		processPassBindables(pass, [&](const Context::BindableRef& bindable) {
			q.getBindable(bindable)->bind();
		});
	}


	void Renderable3D::unbind(Context::Query& q, Pass* pass) const
	{
		processPassBindables(pass, [&](const Context::BindableRef& bindable) {
			q.getBindable(bindable)->unbind();
		});
	}

}
