#include <algorithm>
#include "se/graphics/core/Bindable.h"
#include "se/graphics/3D/Renderable3D.h"

namespace se::graphics {

	Renderable3D& Renderable3D::setModelMatrix(const glm::mat4& modelMatrix)
	{
		mModelMatrix = modelMatrix;
		return *this;
	}


	Renderable3D& Renderable3D::addPassBindable(Pass* pass, BindableSPtr bindable)
	{
		if (pass && bindable) {
			mPassBindables[pass].push_back(bindable);
		}

		return *this;
	}


	Renderable3D& Renderable3D::removePassBindable(Pass* pass, BindableSPtr bindable)
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


	void Renderable3D::bind(Pass* pass) const
	{
		auto it = mPassBindables.find(pass);
		if (it != mPassBindables.end()) {
			for (auto& bindable : it->second) {
				bindable->bind();
			}
		}
	}


	void Renderable3D::unbind(Pass* pass) const
	{
		auto it = mPassBindables.find(pass);
		if (it != mPassBindables.end()) {
			for (auto itBindable = it->second.rbegin(); itBindable != it->second.rend(); ++itBindable) {
				(*itBindable)->unbind();
			}
		}
	}

}
