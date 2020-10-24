#ifndef SKIN_COMPONENT_H
#define SKIN_COMPONENT_H

#include <memory>
#include "../utils/FixedVector.h"
#include "../animation/AnimationNode.h"

namespace se::app {

	/**
	 * Struct Skin, holds the shared joints data between multiple Entities'
	 * Renderable3Ds used for the skinning of their Meshes
	 */
	struct Skin
	{
		/** The maximum number of joints in the skin */
		static constexpr std::size_t kMaxJoints = 64;

		/** The inverse bind matrices of the joints, they're used for
		 * transforming the mesh to the local space of each joint */
		utils::FixedVector<glm::mat4, kMaxJoints> inverseBindMatrices;
	};


	/**
	 * Struct SkinComponent, holds the relationship between the AnimationNodes
	 * of an Entity and the joints of its Renderable3D used for the skinning of
	 * its Mesh
	 */
	class SkinComponent
	{
	public:		// Nested types
		using SkinSPtr = std::shared_ptr<Skin>;
		using MapNodeJoint = utils::FixedVector<
			std::pair<animation::AnimationNode*, std::size_t>,
			Skin::kMaxJoints
		>;

	private:	// Attributes
		/** A pointer to the Skin that is going to be used for the Skinning
		 * animation */
		SkinSPtr mSkin;

		/** Maps the AnimationNodes with their respective joint indices in
		 * @see skin */
		MapNodeJoint mJointIndices;

	public:		// Functions
		/** Creates a new SkinComponent
		 *
		 * @param	skin a pointer to the Skin used by the SkinComponent
		 * @param	jointIndices maps each AnimationNode with its respective
		 *			joint index in the skin */
		SkinComponent(SkinSPtr skin, MapNodeJoint jointIndices = {}) :
			mSkin(skin), mJointIndices(jointIndices) {};

		/** @return	a pointer to the Skin used by the SkinComponent */
		SkinSPtr getSkin() const { return mSkin; };

		/** Creates a new SkinComponent from the current one
		 *
		 * @param	sourceRootNode a pointer to the root node of the hierarchy
		 *			that the current SkinComponent references to
		 * @param	copyRootNode a pointer to the root node of the hierarchy
		 *			that the new SkinComponent will reference to
		 * @return	the new SkinComponent
		 * @note	the hierarchy of the descendants of both nodes must be the
		 *			same */
		SkinComponent duplicateSkinComponent(
			animation::AnimationNode* sourceRootNode,
			animation::AnimationNode* copyRootNode
		);

		/** Calculates the joint matrices of the current SkinComponent
		 *
		 * @param	modelMatrix the model matrix that transforms from the local
		 *			space of a Renderable3D to the global space
		* @return	a vector with the joint matrices of the SkinComponent in the
		*			local space of the Renderable3D */
		utils::FixedVector<glm::mat4, Skin::kMaxJoints> calculateJointMatrices(
			const glm::mat4& modelMatrix
		);
	};

}

#endif		// SKIN_COMPONENT_H
