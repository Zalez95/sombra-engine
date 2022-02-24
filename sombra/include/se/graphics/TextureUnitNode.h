#ifndef TEXTURE_UNIT_NODE_H
#define TEXTURE_UNIT_NODE_H

#include "BindableRenderNode.h"

namespace se::graphics {

	/**
	 * Class TextureUnitNode, it's BindableRenderNode that can change the
	 * texture unit where a Texture must be bound. It has an "input"
	 * BindableRNodeInput where the Texture to update can be inserted and an
	 * "output" BindableRNodeOutput where the same Texture can be recovered
	 */
	class TextureUnitNode : public BindableRenderNode
	{
	private:	// Attributes
		/** The texture unit to set */
		int mUnit;

		/** The bindable index where the Texture is going to be attached */
		std::size_t mBindableIndex;

	public:		// Functions
		/** Creates a new TextureUnitNode
		 *
		 * @param	name the name of the new TextureUnitNode
		 * @param	unit the Texture unit to set in the texture */
		TextureUnitNode(const std::string& name, int unit);

		/** Class destructor */
		virtual ~TextureUnitNode() = default;

		/** @copydoc RenderNode::execute(Context::Query&) */
		virtual void execute(Context::Query& q) override;
	};

}

#endif		// TEXTURE_UNIT_NODE_H
