#include <algorithm>
#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>
#include <se/app/Scene.h>
#include <se/app/TagComponent.h>
#include "EntityPanel.h"
#include "Editor.h"

namespace editor {

	bool EntityPanel::render()
	{
		bool open = true;
		if (!ImGui::Begin(("Entity Panel##EntityPanel" + std::to_string(mPanelId)).c_str(), &open)) {
			ImGui::End();
			return open;
		}

		se::app::Scene* scene = mEditor.getScene();
		if (!scene) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		if (scene) {
			if (ImGui::SmallButton(("Add##EntityPanel" + std::to_string(mPanelId) + "::Add").c_str())) {
				se::app::Entity newEntity = se::app::kNullEntity;

				mEditor.getEntityDatabase().executeQuery([&](se::app::EntityDatabase::Query& query) {
					newEntity = query.addEntity();
					if (newEntity != se::app::kNullEntity) {
						scene->entities.push_back(newEntity);
					}
				});

				mEditor.setActiveEntity(newEntity);
			}
			ImGui::SameLine();
			if (ImGui::SmallButton(("Remove##EntityPanel" + std::to_string(mPanelId) + "::Remove").c_str())) {
				se::app::Entity entity = mEditor.getActiveEntity();
				auto itEntity = std::find(scene->entities.begin(), scene->entities.end(), entity);
				if (itEntity != scene->entities.end()) {
					scene->entities.erase(itEntity);
					mEditor.setActiveEntity(se::app::kNullEntity);
					mEditor.getEntityDatabase().executeQuery([&](se::app::EntityDatabase::Query& query) {
						query.removeEntity(entity);
					});
				}
			}
			ImGui::SameLine();
			if (ImGui::SmallButton(("Copy##EntityPanel" + std::to_string(mPanelId) + "::Copy").c_str())) {
				se::app::Entity entity = mEditor.getActiveEntity();
				auto itEntity = std::find(scene->entities.begin(), scene->entities.end(), entity);
				if (itEntity != scene->entities.end()) {
					se::app::Entity newEntity = se::app::kNullEntity;

					mEditor.getEntityDatabase().executeQuery([&](se::app::EntityDatabase::Query& query) {
						newEntity = query.copyEntity(entity);
						if (newEntity != se::app::kNullEntity) {
							scene->entities.push_back(newEntity);
						}
					});

					mEditor.setActiveEntity(newEntity);
				}
			}

			ImGui::BeginChild("Entities");
			for (se::app::Entity entity : scene->entities) {
				std::string name = "#" + std::to_string(entity);

				mEditor.getEntityDatabase().executeQuery([&](se::app::EntityDatabase::Query& query) {
					auto [tag] = query.getComponents<se::app::TagComponent>(entity);
					if (tag) {
						name += " " + std::string(tag->getName(), tag->getLength());
					}
				});

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
		return open;
	}

}
