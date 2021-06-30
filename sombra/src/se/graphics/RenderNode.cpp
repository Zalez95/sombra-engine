#include <algorithm>
#include "se/graphics/RenderNode.h"
#include "se/utils/Log.h"

namespace se::graphics {

	bool RNodeOutput::connect(RNodeConnector* connector)
	{
		RNodeInput* input = dynamic_cast<RNodeInput*>(connector);
		if (!input) {
			SOMBRA_ERROR_LOG << "Trying to attach " << connector->getParentNode()->getName() << "[" << connector->getName() << "]"
				<< " with invalid type to " << mParentNode->getName() << "[" << mName << "]";
			return false;
		}

		return input->connect(this);
	}


	void RNodeOutput::disconnect()
	{
		while (!mConnectedInputs.empty()) {
			mConnectedInputs.back()->disconnect();
		}
	}


	void RNodeOutput::addInput(RNodeInput* input)
	{
		mConnectedInputs.push_back(input);
	}


	void RNodeOutput::removeInput(RNodeInput* input)
	{
		mConnectedInputs.erase(
			std::remove(mConnectedInputs.begin(), mConnectedInputs.end(), input),
			mConnectedInputs.end()
		);
	}


	bool RNodeInput::connect(RNodeConnector* connector)
	{
		RNodeOutput* output = dynamic_cast<RNodeOutput*>(connector);
		if (!output) {
			SOMBRA_ERROR_LOG << "Trying to attach " << connector->getParentNode()->getName() << "[" << connector->getName() << "]"
				<< " with invalid type to " << mParentNode->getName() << "[" << mName << "]";
			return false;
		}

		if (mConnectedOutput) {
			SOMBRA_ERROR_LOG << mParentNode->getName() << "[" << mName << "] is already connected to "
				<< mConnectedOutput->getParentNode()->getName() + "[" + mConnectedOutput->getName() << "]"
				<< ": Can't create new connection with " << output->getParentNode()->getName() << "[" << output->getName() << "]";
			return false;
		}

		mConnectedOutput = output;
		mConnectedOutput->addInput(this);
		return true;
	}


	void RNodeInput::disconnect()
	{
		if (mConnectedOutput) {
			mConnectedOutput->removeInput(this);
			mConnectedOutput = nullptr;
		}
	}


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


	bool RenderNode::removeInput(RNodeInput* input)
	{
		if (input->getConnectedOutput()) {
			SOMBRA_ERROR_LOG << getName() << "[" << input->getName() << "] has connections, it can't be removed";
			return false;
		}

		mInputs.erase(
			std::remove_if(mInputs.begin(), mInputs.end(), [&](const InputUPtr& input2) { return input == input2.get(); }),
			mInputs.end()
		);
		return true;
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


	bool RenderNode::removeOutput(RNodeOutput* output)
	{
		if (output->hasConnections()) {
			SOMBRA_ERROR_LOG << getName() << "[" << output->getName() << "] has connections, it can't be removed";
			return false;
		}

		mOutputs.erase(
			std::remove_if(mOutputs.begin(), mOutputs.end(), [&](const OutputUPtr& output2) { return output == output2.get(); }),
			mOutputs.end()
		);
		return true;
	}


	void RenderNode::disconnect()
	{
		for (auto& output : mOutputs) {
			output->disconnect();
		}
		for (auto& input : mInputs) {
			input->disconnect();
		}
	}

}
