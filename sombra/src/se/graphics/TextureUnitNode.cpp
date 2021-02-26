#include "se/graphics/TextureUnitNode.h"
#include "se/graphics/core/Texture.h"

namespace se::graphics {

	TextureUnitNode::TextureUnitNode(const std::string& name, int unit) :
		BindableRenderNode(name), mUnit(unit)
	{
		mBindableIndex = addBindable();
		addInput( std::make_unique<BindableRNodeInput<Texture>>("input", this, mBindableIndex) );
		addOutput( std::make_unique<BindableRNodeOutput<Texture>>("output", this, mBindableIndex) );
	}


	void TextureUnitNode::execute()
	{
		BindableSPtr bindable = getBindable(mBindableIndex);
		if (bindable) {
			std::dynamic_pointer_cast<Texture>(bindable)->setTextureUnit(mUnit);
		}
	}

}
