#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>
#include <se/app/Scene.h>
#include <se/app/EntityDatabase.h>
#include "SceneNodesPanel.h"
#include "Editor.h"

namespace editor {

	void SceneNodesPanel::render()
	{
		if (!ImGui::Begin("Scene Hierarchy Panel")) {
			ImGui::End();
			return;
		}

		se::app::Scene* scene = mEditor.getScene();
		if (!scene) {
			mSelectedNode = NodeIterator();
			ImGui::End();
			return;
		}

		// Options for adding root child nodes
		se::animation::AnimationNode* root = &scene->rootNode;
		if (ImGui::BeginPopupContextItem()) {
			mSelectedNode = root->end();
			mWorkingData = se::animation::NodeData();

			mAdd |= ImGui::MenuItem("Add");
			ImGui::EndPopup();
		}

		// Draw the node hierarchy
		for (auto it = root->begin(); (it != root->end()) && (it->getParent() == root); ++it) {
			drawNode(it);
		}

		// Draw the node transforms
		if (mSelectedNode != root->end()) {
			auto& animationData = mSelectedNode->getData();
			bool updated = false;

			ImGui::Separator();
			ImGui::Text("Node selected: 0x%p", static_cast<void*>(&(*mSelectedNode)));
			ImGui::InputText("Name##AnimationNodeName", animationData.name.data(), animationData.name.size());
			ImGui::Text("Local transforms:");
			updated |= ImGui::DragFloat3("Position", glm::value_ptr(animationData.localTransforms.position), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
			updated |= drawOrientation("Orientation##AnimationNodeOrientation", animationData.localTransforms.orientation, mOrientationType);
			updated |= ImGui::DragFloat3("Scale", glm::value_ptr(animationData.localTransforms.scale), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
			animationData.animated = updated;
		}

		// Operations
		if (mRemove) {
			mRemove = false;
			mEditor.getEntityDatabase().iterateComponents<se::animation::AnimationNode*>(
				[&](se::app::Entity, se::animation::AnimationNode** node) {
					if (*node == &(*mSelectedNode)) {
						*node = nullptr;
					}
				}
			);
			root->erase(mSelectedNode);
		}
		if (mRemoveHierarchy) {
			mRemoveHierarchy = false;
			mEditor.getEntityDatabase().iterateComponents<se::animation::AnimationNode*>(
				[&](se::app::Entity, se::animation::AnimationNode** node) {
					if (*node == &(*mSelectedNode) || (mSelectedNode->find(**node) != mSelectedNode->end())) {
						*node = nullptr;
					}
				}
			);
			root->erase(mSelectedNode, true);
		}
		if (mAdd) {
			ImGui::OpenPopup("addPopup");
			if (ImGui::BeginPopup("addPopup")) {
				ImGui::InputText("Name##addName", mWorkingData.name.data(), mWorkingData.name.size());
				if (ImGui::Button("Add")) {
					mAdd = false;
					fixWorkingDataName();
					se::animation::AnimationNode::const_iterator<se::utils::Traversal::BFS> selectedNode = mSelectedNode;
					root->insert(selectedNode, std::make_unique<se::animation::AnimationNode>(mWorkingData));
					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel")) {
					mAdd = false;
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
		}
		if (mChangeParent) {
			ImGui::OpenPopup("changeParentPopup");
			if (ImGui::BeginPopup("changeParentPopup")) {
				static const char* items[] = { "Root", "Other" };
				int current = mRoot? 0 : 1;
				if (ImGui::Combo("Parent node", &current, items, IM_ARRAYSIZE(items))) {
					mRoot = (current == 0);
				}
				if (!mRoot) {
					ImGui::InputText("Name##changeParentName", mWorkingData.name.data(), mWorkingData.name.size());
				}
				ImGui::Checkbox("Update descendants", &mDescendants);

				if (ImGui::Button("Change")) {
					mChangeParent = false;

					bool change = true;
					auto it = root->cend();
					if (!mRoot) {
						it = std::find_if(root->cbegin(), root->cend(), [&](const se::animation::AnimationNode& node) {
							return std::strcmp(node.getData().name.data(), mWorkingData.name.data()) == 0;
						});
						change = (it != root->cend())
							&& (it != se::animation::AnimationNode::const_iterator<se::utils::Traversal::BFS>(mSelectedNode));
					}
					if (change) {
						mSelectedNode = root->move(mSelectedNode, it, mDescendants);
						mWorkingData = mSelectedNode->getData();
					}

					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel")) {
					mChangeParent = false;
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
		}

		ImGui::End();
	}


	void SceneNodesPanel::drawNode(NodeIterator itNode)
	{
		// Draw TreeNode if the Node has childs, Selectable otherwise
		ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth
			| ((itNode == mSelectedNode)? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None)
			| (!itNode->getChild()? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_None);
		bool nodeOpen = ImGui::TreeNodeEx(&(*itNode), nodeFlags, "%s", itNode->getData().name.data());

		// Select the node with the left and right mouse buttons
		if (ImGui::IsItemClicked()) {
			mSelectedNode = itNode;
			mWorkingData = itNode->getData();
		}
		if (ImGui::BeginPopupContextItem()) {
			mSelectedNode = itNode;
			mWorkingData = itNode->getData();

			mAdd |= ImGui::MenuItem("Add");
			mRemove |= ImGui::MenuItem("Remove");
			mRemoveHierarchy |= ImGui::MenuItem("Remove hierarchy");
			mChangeParent |= ImGui::MenuItem("Change parent");
			ImGui::EndPopup();
		}

		// Draw children nodes
		if (nodeOpen) {
			for (auto it = ++NodeIterator(itNode); it != itNode->end(); ++it) {
				if (it->getParent() == &(*itNode)) {
					drawNode(it);
				}
			}
			ImGui::TreePop();
		}
	}


	void SceneNodesPanel::fixWorkingDataName()
	{
		se::animation::AnimationNode& root = mEditor.getScene()->rootNode;
		std::string name = mWorkingData.name.data();

		for (std::size_t i = 0; true; ++i) {
			if (std::none_of(root.cbegin(), root.cend(), [&](const se::animation::AnimationNode& node) {
				return node.getData().name.data() == name;
			})) {
				break;
			}

			name = mWorkingData.name.data();
			std::string number = "." + std::to_string(i);
			std::size_t nameSize = mWorkingData.name.size() - number.size();
			if (name.size() > nameSize) {
				name = name.substr(0, nameSize);
			}
			name += number;
		}

		std::copy(name.begin(), name.end(), mWorkingData.name.begin());
	}

}
