#include <algorithm>
#include "se/graphics/Technique.h"

namespace se::graphics {

	Step& Step::addBindable(BindableSPtr bindable)
	{
		if (bindable) {
			mBindables.push_back(bindable);
		}

		return *this;
	}


	Step& Step::removeBindable(BindableSPtr bindable)
	{
		mBindables.erase(std::remove(mBindables.begin(), mBindables.end(), bindable), mBindables.end());

		return *this;
	}


	void Step::bind() const
	{
		for (auto& bindable : mBindables) {
			bindable->bind();
		}
	}


	void Step::unbind() const
	{
		for (auto itBindable = mBindables.rbegin(); itBindable != mBindables.rend(); ++itBindable) {
			(*itBindable)->unbind();
		}
	}


	Technique& Technique::addStep(StepSPtr step)
	{
		mSteps.emplace_back(std::move(step));
		return *this;
	}


	void Technique::processSteps(const StepCallback& callback)
	{
		for (auto& step : mSteps) {
			callback(step);
		}
	}


	Technique& Technique::removeStep(StepSPtr step)
	{
		mSteps.erase(std::remove(mSteps.begin(), mSteps.end(), step), mSteps.end());

		return *this;
	}


	void Technique::submit(Renderable& renderable)
	{
		for (auto& step : mSteps) {
			step->submit(renderable);
		}
	}

}
