#include <string>
#include <algorithm>
#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>
#include <se/app/TagComponent.h>
#include <se/app/Scene.h>
#include <se/app/EntityDatabase.h>
#include <se/app/CameraComponent.h>
#include <se/app/TransformsComponent.h>
#include <se/animation/AnimationNode.h>
#include "EntityPanel.h"
#include "Editor.h"

using namespace se::app;
using namespace se::animation;

namespace editor {

	EntityPanel::EntityPanel(Editor& editor) : mEditor(editor)
	{
		mSelectedEntities.reserve(mEditor.getEntityDatabase().getMaxEntities());
	}


	void EntityPanel::render()
	{
		if (ImGui::Begin("Entity Panel")) {
			drawEntities();
			drawComponents();
			ImGui::End();
		}
	}

// Private functions
	void EntityPanel::drawEntities()
	{
		ImGui::SetNextItemOpen(true, ImGuiCond_Once);
		if (!ImGui::CollapsingHeader("Entities")) { return; }

		auto scene = mEditor.getScene();
		if (!scene) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		// Add an entity
		if (ImGui::SmallButton("Add")) {
			auto entity = mEditor.getEntityDatabase().addEntity();
			scene->entities.push_back(entity);
		}

		ImGui::SameLine();

		// Remove the enities
		if (ImGui::SmallButton("Remove")) {
			for (auto [entity, selected] : mSelectedEntities) {
				if (selected) {
					mEditor.getEntityDatabase().removeEntity(entity);
					auto it = std::find(scene->entities.begin(), scene->entities.end(), entity);
					if (it != scene->entities.end()) {
						std::swap(scene->entities.back(), *it);
						scene->entities.pop_back();
					}
				}
			}
		}

		// Update the selected entities
		if (scene) {
			std::unordered_map<Entity, bool> nextSelectedEntities;
			nextSelectedEntities.reserve(mEditor.getEntityDatabase().getMaxEntities());

			for (Entity entity : scene->entities) {
				bool selected = false;

				auto it = mSelectedEntities.find(entity);
				if (it != mSelectedEntities.end()) {
					selected = it->second;
				}

				nextSelectedEntities.emplace(entity, selected);
			}
			std::swap(mSelectedEntities, nextSelectedEntities);

			ImGui::BeginChild("Entities", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, 260));
			for (auto& [entity, selected] : mSelectedEntities) {
				ImGui::Checkbox(("Entity #" + std::to_string(entity)).c_str(), &selected);
			}
			ImGui::EndChild();
		}
		else {
			mSelectedEntities.clear();
		}

		if (!scene) {
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
	}


	void EntityPanel::drawComponents()
	{
		ImGui::SetNextItemOpen(true, ImGuiCond_Once);
		if (!ImGui::CollapsingHeader("Components")) { return; }

		auto it = std::find_if(mSelectedEntities.begin(), mSelectedEntities.end(), [](auto pair) { return pair.second; });
		if (it != mSelectedEntities.end()) {
			Entity selectedEntity = it->first;
			auto [tag, transforms, animationNode, camera] = mEditor.getEntityDatabase().getComponents<
					TagComponent, TransformsComponent, AnimationNode*, CameraComponent
				>(selectedEntity);

			ImGui::AlignTextToFramePadding();
			ImGui::Text(("Entity #" + std::to_string(selectedEntity) + " selected").c_str());
			ImGui::SameLine();
			if (ImGui::Button("Add component")) {
				ImGui::OpenPopup("components");
			}
			if (ImGui::BeginPopup("components")) {
				if (!tag) {
					if (ImGui::MenuItem("Add Tag", nullptr, false)) {
						tag = mEditor.getEntityDatabase().emplaceComponent<TagComponent>(selectedEntity, "");
					}
				}
				if (!transforms) {
					if (ImGui::MenuItem("Add Transforms", nullptr, false)) {
						transforms = mEditor.getEntityDatabase().emplaceComponent<TransformsComponent>(selectedEntity);
					}
				}
				if (!animationNode) {
					if (ImGui::MenuItem("Add AnimationNode", nullptr, false)) {
						animationNode = mEditor.getEntityDatabase().emplaceComponent<AnimationNode*>(selectedEntity, new AnimationNode());
					}
				}
				if (!camera) {
					if (ImGui::MenuItem("Add Camera", nullptr, false)) {
						camera = mEditor.getEntityDatabase().emplaceComponent<CameraComponent>(selectedEntity);
					}
				}
				ImGui::EndPopup();
			}

			if (tag && ImGui::TreeNode("Tag")) {
				char nameBuffer[TagComponent::kMaxLength] = {};
				std::copy(tag->getName(), tag->getName() + tag->getLength(), nameBuffer);
				if (ImGui::InputText("Name", nameBuffer, TagComponent::kMaxLength)) {
					tag->setName(nameBuffer);
				}

				ImGui::TreePop();
			}

			if (transforms && ImGui::TreeNode("Transforms")) {
				transforms->updated.reset( static_cast<int>(TransformsComponent::Update::Input) );

				bool updated = false;
				updated |= ImGui::InputFloat3("Position", glm::value_ptr(transforms->position), "%.3f");
				updated |= ImGui::InputFloat3("Velocity", glm::value_ptr(transforms->velocity), "%.3f");
				updated |= ImGui::InputFloat4("Orientation", glm::value_ptr(transforms->orientation), "%.3f");
				updated |= ImGui::InputFloat3("Scale", glm::value_ptr(transforms->scale), "%.3f");

				if (updated) {
					transforms->updated.set( static_cast<int>(TransformsComponent::Update::Input) );
				}

				ImGui::TreePop();
			}

			if (animationNode && ImGui::TreeNode("AnimationNode")) {
				auto& animationData = (*animationNode)->getData();
				ImGui::InputText("Name", animationData.name.data(), animationData.name.size());

				ImGui::Text("Local transforms:");
				bool updated = false;
				updated |= ImGui::InputFloat3("Position", glm::value_ptr(animationData.localTransforms.position), "%.3f");
				updated |= ImGui::InputFloat4("Orientation", glm::value_ptr(animationData.localTransforms.orientation), "%.3f");
				updated |= ImGui::InputFloat3("Scale", glm::value_ptr(animationData.localTransforms.scale), "%.3f");
				animationData.animated = updated;

				if (updated) {
					transforms->updated.set( static_cast<int>(TransformsComponent::Update::Input) );
				}

				ImGui::TreePop();
			}

			if (camera && ImGui::TreeNode("Camera")) {
				bool updated = false;
				bool ortho = camera->hasOrthographicProjection();
				if (ImGui::RadioButton("Orthographic", ortho)) { updated = !ortho; ortho = true; }
				ImGui::SameLine();
				if (ImGui::RadioButton("Perspective", !ortho)) { updated = ortho; ortho = false; }

				if (ortho) {
					float left = 0.0f, right = 1280.0f, bottom = 0.0f, top = 720.0f, zNear = 0.1f, zFar = 10000.0f;
					camera->getOrthographicParams(left, right, bottom, top, zNear, zFar);

					updated |= ImGui::InputFloat("Left", &left, 0.05f, 0.0f, "%.3f");
					updated |= ImGui::InputFloat("Right", &right, 0.05f, 0.0f, "%.3f");
					updated |= ImGui::InputFloat("Bottom", &bottom, 0.05f, 0.0f, "%.3f");
					updated |= ImGui::InputFloat("Top", &top, 0.05f, 0.0f, "%.3f");
					updated |= ImGui::InputFloat("zNear", &zNear, 0.05f, 0.0f, "%.3f");
					updated |= ImGui::InputFloat("zFar", &zFar, 0.05f, 0.0f, "%.3f");

					if (updated) {
						camera->setOrthographicProjection(left, right, bottom, top, zNear, zFar);
					}
				}
				else {
					float fovy = glm::pi<float>() / 3.0f, aspectRatio = 1280.0f / 720.0f, zNear = 0.1f, zFar = 10000.0f;
					camera->getPerspectiveParams(fovy, aspectRatio, zNear, zFar);

					updated |= ImGui::InputFloat("fovy", &fovy, 0.05f, 0.0f, "%.3f");
					updated |= ImGui::InputFloat("Aspect Ratio", &aspectRatio, 0.05f, 0.0f, "%.3f");
					updated |= ImGui::InputFloat("zNear", &zNear, 0.05f, 0.0f, "%.3f");
					updated |= ImGui::InputFloat("zFar", &zFar, 0.05f, 0.0f, "%.3f");

					if (updated) {
						camera->setPerspectiveProjection(fovy, aspectRatio, zNear, zFar);
					}
				}

				ImGui::TreePop();
			}
		}
		else {
			ImGui::Text("No Entity selected");
		}
	}

}
