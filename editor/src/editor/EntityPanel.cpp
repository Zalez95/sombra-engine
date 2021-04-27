#include <algorithm>
#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>
#include <se/app/Scene.h>
#include <se/app/TagComponent.h>
#include "EntityPanel.h"
#include "Editor.h"

namespace editor {

	void EntityPanel::render()
	{
		if (!ImGui::Begin("Entity Panel")) {
			ImGui::End();
			return;
		}

		se::app::Scene* scene = mEditor.getScene();
		if (!scene) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		if (scene) {
			if (ImGui::BeginPopupContextItem()) {
				if (ImGui::MenuItem("Add")) {
					se::app::Entity newEntity = mEditor.getEntityDatabase().addEntity();
					scene->entities.push_back(newEntity);
					mEditor.setActiveEntity(newEntity);
				}
				if (ImGui::MenuItem("Remove")) {
					se::app::Entity entity = mEditor.getActiveEntity();
					auto itEntity = std::find(scene->entities.begin(), scene->entities.end(), entity);
					if (itEntity != scene->entities.end()) {
						scene->entities.erase(itEntity);
						mEditor.getEntityDatabase().removeEntity(entity);
					}
				}
				ImGui::EndPopup();
			}

			ImGui::BeginChild("Entities");
			for (se::app::Entity entity : scene->entities) {
				auto [tag] = mEditor.getEntityDatabase().getComponents<se::app::TagComponent>(entity);

				std::string name = "#" + std::to_string(entity);
				if (tag) {
					name += " \"" + std::string(tag->getName()) + "\"";
				}

				ImGui::Bullet();
				ImGui::SameLine();
				if (ImGui::Selectable(name.c_str(), mEditor.getActiveEntity() == entity)) {
					mEditor.setActiveEntity(entity);
				}
			}
			ImGui::EndChild();
		}

		if (!scene) {
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}

		ImGui::End();
	}

}
