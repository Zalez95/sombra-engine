#include <algorithm>
#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>
#include <se/app/Scene.h>
#include <se/app/EntityDatabase.h>
#include "EntityPanel.h"
#include "Editor.h"

namespace editor {

	EntityPanel::EntityPanel(Editor& editor) : mEditor(editor)
	{
		mSelectedEntities.reserve(mEditor.getEntityDatabase().getMaxEntities());
	}


	se::app::Entity EntityPanel::getActiveEntity() const
	{
		se::app::Entity ret = se::app::kNullEntity;
		se::app::Scene* scene = mEditor.getScene();

		if (scene && !mSelectedEntities.empty()) {
			se::app::Entity entity = mSelectedEntities.back();
			auto itEntity = std::find(scene->entities.begin(), scene->entities.end(), entity);
			if (itEntity != scene->entities.end()) {
				ret = entity;
			}
		}

		return ret;
	}


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
			// Add an entity
			if (ImGui::SmallButton("Add")) {
				se::app::Entity entity = mEditor.getEntityDatabase().addEntity();
				scene->entities.push_back(entity);
			}

			ImGui::SameLine();

			// Remove the entities
			if (ImGui::SmallButton("Remove")) {
				for (se::app::Entity entity : mSelectedEntities) {
					auto itEntity = std::find(scene->entities.begin(), scene->entities.end(), entity);
					if (itEntity != scene->entities.end()) {
						scene->entities.erase(itEntity);
						mEditor.getEntityDatabase().removeEntity(entity);
					}
				}
			}

			// Update the selected entities
			std::vector<se::app::Entity> nextSelectedEntities;
			nextSelectedEntities.reserve(mEditor.getEntityDatabase().getMaxEntities());

			for (se::app::Entity entity : mSelectedEntities) {
				auto it = std::find(scene->entities.begin(), scene->entities.end(), entity);
				if (it != scene->entities.end()) {
					nextSelectedEntities.push_back(entity);
				}
			}

			std::swap(mSelectedEntities, nextSelectedEntities);

			ImGui::BeginChild("Entities");
			for (se::app::Entity entity : scene->entities) {
				auto itEntity = std::find(mSelectedEntities.begin(), mSelectedEntities.end(), entity);
				bool selected = (itEntity != mSelectedEntities.end());
				if (ImGui::Checkbox(("Entity #" + std::to_string(entity)).c_str(), &selected)) {
					if ((itEntity == mSelectedEntities.end()) && selected) {
						mSelectedEntities.push_back(entity);
					}
					else if ((itEntity != mSelectedEntities.end()) && !selected) {
						mSelectedEntities.erase(itEntity);
					}
				}
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

		ImGui::End();
	}

}
