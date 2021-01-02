#include <string>
#include <algorithm>
#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>
#include <se/app/TagComponent.h>
#include <se/app/Scene.h>
#include <se/app/EntityDatabase.h>
#include <se/app/CameraComponent.h>
#include <se/app/LightComponent.h>
#include <se/app/MeshComponent.h>
#include <se/app/TransformsComponent.h>
#include <se/animation/AnimationNode.h>
#include "EntityPanel.h"
#include "Editor.h"

using namespace se::app;
using namespace se::graphics;
using namespace se::animation;

namespace editor {

	template <typename T>
	bool addRepoDropdownButton(const char* buttonName, const char* name, se::utils::Repository& repository, std::shared_ptr<T>& selectedValue)
	{
		bool ret = false;

		if (ImGui::Button(buttonName)) {
			ImGui::OpenPopup(name);
		}
		if (ImGui::BeginPopup(name)) {
			repository.iterate<std::string, T>(
				[&](const std::string& key, std::shared_ptr<T>& value) {
					if (ImGui::MenuItem(key.c_str(), nullptr, false)) {
						selectedValue = value;
						ret = true;
					}
				}
			);

			ImGui::EndPopup();
		}

		return ret;
	}


	template <typename T>
	bool addRepoDropdownShowSelected(const char* name, se::utils::Repository& repository, std::shared_ptr<T>& selectedValue)
	{
		bool ret = false;

		std::string selectedValueName = "       ";
		repository.iterate<std::string, T>(
			[&](const std::string& key, std::shared_ptr<T>& value) {
				if (selectedValue == value) {
					selectedValueName = key;
				}
			}
		);

		if (ImGui::Button(selectedValueName.c_str())) {
			ImGui::OpenPopup(name);
		}
		if (ImGui::BeginPopup(name)) {
			repository.iterate<std::string, T>(
				[&](const std::string& key, std::shared_ptr<T>& value) {
					if (ImGui::MenuItem(key.c_str(), nullptr, false)) {
						selectedValue = value;
						ret = true;
					}
				}
			);

			ImGui::EndPopup();
		}

		return ret;
	}


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
			auto [tag, transforms, animationNode, camera, light, mesh] = mEditor.getEntityDatabase().getComponents<
					TagComponent, TransformsComponent, AnimationNode*, CameraComponent, LightComponent, MeshComponent
				>(selectedEntity);

			ImGui::AlignTextToFramePadding();
			std::string entityName = "Entity #" + std::to_string(selectedEntity) + " selected";
			ImGui::Text(entityName.c_str());
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
				if (!light) {
					if (ImGui::MenuItem("Add Light", nullptr, false)) {
						light = mEditor.getEntityDatabase().emplaceComponent<LightComponent>(selectedEntity);
					}
				}
				if (!mesh) {
					if (ImGui::MenuItem("Add Mesh", nullptr, false)) {
						mesh = mEditor.getEntityDatabase().emplaceComponent<MeshComponent>(selectedEntity, mEditor.getEventManager(), selectedEntity);
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
				updated |= ImGui::DragFloat3("Position", glm::value_ptr(transforms->position), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
				updated |= ImGui::DragFloat3("Velocity", glm::value_ptr(transforms->velocity), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
				updated |= ImGui::DragFloat4("Orientation", glm::value_ptr(transforms->orientation), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
				updated |= ImGui::DragFloat3("Scale", glm::value_ptr(transforms->scale), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);

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
				updated |= ImGui::DragFloat3("Position", glm::value_ptr(animationData.localTransforms.position), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
				updated |= ImGui::DragFloat4("Orientation", glm::value_ptr(animationData.localTransforms.orientation), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
				updated |= ImGui::DragFloat3("Scale", glm::value_ptr(animationData.localTransforms.scale), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
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

					updated |= ImGui::DragFloat("Left", &left, 0.05f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
					updated |= ImGui::DragFloat("Right", &right, 0.05f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
					updated |= ImGui::DragFloat("Bottom", &bottom, 0.05f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
					updated |= ImGui::DragFloat("Top", &top, 0.05f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
					updated |= ImGui::DragFloat("zNear", &zNear, 0.05f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
					updated |= ImGui::DragFloat("zFar", &zFar, 0.05f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);

					if (updated) {
						camera->setOrthographicProjection(left, right, bottom, top, zNear, zFar);
					}
				}
				else {
					float fovy = glm::pi<float>() / 3.0f, aspectRatio = 1280.0f / 720.0f, zNear = 0.1f, zFar = 10000.0f;
					camera->getPerspectiveParams(fovy, aspectRatio, zNear, zFar);

					float fovyDegrees = glm::degrees(fovy);
					updated |= ImGui::DragFloat("fovy", &fovyDegrees, 0.05f, 0.0f, 360, "%.3f", 1.0f);
					updated |= ImGui::DragFloat("Aspect Ratio", &aspectRatio, 0.05f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
					updated |= ImGui::DragFloat("zNear", &zNear, 0.05f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
					updated |= ImGui::DragFloat("zFar", &zFar, 0.05f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);

					if (updated) {
						camera->setPerspectiveProjection(glm::radians(fovyDegrees), aspectRatio, zNear, zFar);
					}
				}

				ImGui::TreePop();
			}

			if (light && ImGui::TreeNode("Light")) {
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Source:");
				ImGui::SameLine();
				addRepoDropdownShowSelected("light_sources", mEditor.getScene()->repository, light->source);
				ImGui::TreePop();
			}

			if (mesh && ImGui::TreeNode("Mesh")) {
				bool canAddRMesh = (mesh->size() < MeshComponent::kMaxMeshes);
				if (!canAddRMesh) {
					ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
					ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
				}
				std::shared_ptr<Mesh> gMesh;
				if (addRepoDropdownButton("Add RenderableMesh", "add_renderable_mesh", mEditor.getScene()->repository, gMesh)) {
					mesh->add(false, gMesh);
				}
				if (!canAddRMesh) {
					ImGui::PopItemFlag();
					ImGui::PopStyleVar();
				}

				for (std::size_t i = 0; i < mesh->size(); ++i) {
					std::string rMeshName = "RenderableMesh #" + std::to_string(i);
					if (ImGui::TreeNode(rMeshName.c_str())) {
						if (ImGui::RadioButton("Has Skin", mesh->hasSkinning(i))) {
							// TODO:
						}

						ImGui::AlignTextToFramePadding();
						ImGui::Text("Mesh:");
						ImGui::SameLine();
						gMesh = mesh->get(i).getMesh();
						if (addRepoDropdownShowSelected(rMeshName.c_str(), mEditor.getScene()->repository, gMesh)) {
							mesh->get(i).setMesh(gMesh);
						}

						if (ImGui::TreeNode("Shaders:")) {
							std::shared_ptr<se::app::RenderableShader> shader;
							std::string shadersName = "shaders_mesh_" + std::to_string(i);
							if (addRepoDropdownButton("Add Shader", shadersName.c_str(), mEditor.getScene()->repository, shader)) {
								mesh->addRenderableShader(i, shader);
							}

							int j = 0;
							mesh->processRenderableShaders(i, [&](const auto& shader1) {
								ImGui::AlignTextToFramePadding();
								ImGui::Text("Shader:");
								ImGui::SameLine();
								std::shared_ptr<se::app::RenderableShader> shader2 = shader1;
								std::string shaderName2 = shadersName + "_shader_" + std::to_string(j);
								if (addRepoDropdownShowSelected(shaderName2.c_str(), mEditor.getScene()->repository, shader2)) {
									mesh->removeRenderableShader(i, shader1);
									mesh->addRenderableShader(i, shader2);
								}
							});
							ImGui::TreePop();
						}
						ImGui::TreePop();
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
