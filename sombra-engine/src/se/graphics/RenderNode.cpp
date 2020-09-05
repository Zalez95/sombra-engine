#include <algorithm>
#include "se/graphics/RenderNode.h"
#include "se/utils/Log.h"

namespace se::graphics {

	bool RenderNode::addInput(InputUPtr input)
	{
		if (std::none_of(
			mInputs.begin(), mInputs.end(),
			[&](const InputUPtr& input2) { return input2->getName() == input->getName(); }
		)) {
			mInputs.emplace_back( std::move(input) );
			return true;
		}
		else {
			SOMBRA_ERROR_LOG << "Input found with the same name";
			return false;
		}
	}


	RNodeInput* RenderNode::findInput(const std::string& name) const
	{
		auto itInput = std::find_if(
			mInputs.begin(), mInputs.end(),
			[&](const InputUPtr& input) { return input->getName() == name; }
		);
		if (itInput != mInputs.end()) {
			return itInput->get();
		}
		return nullptr;
	}


	bool RenderNode::addOutput(OutputUPtr output)
	{
		if (std::none_of(
			mOutputs.begin(), mOutputs.end(),
			[&](const OutputUPtr& output2) { return output2->getName() == output->getName(); }
		)) {
			mOutputs.emplace_back( std::move(output) );
			return true;
		}
		else {
			SOMBRA_ERROR_LOG << "Output found with the same name";
			return false;
		}
	}


	RNodeOutput* RenderNode::findOutput(const std::string& name) const
	{
		auto itOutput = std::find_if(
			mOutputs.begin(), mOutputs.end(),
			[&](const OutputUPtr& output) { return output->getName() == name; }
		);
		if (itOutput != mOutputs.end()) {
			return itOutput->get();
		}
		return nullptr;
	}

}
