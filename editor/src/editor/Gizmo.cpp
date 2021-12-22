#include <imgui.h>
#include <ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>
#include <se/app/CameraComponent.h>
#include <se/app/TransformsComponent.h>
#include <se/utils/MathUtils.h>
#include "Gizmo.h"
#include "Editor.h"

namespace editor {

	bool Gizmo::render()
	{
		bool open = true;
		if (ImGui::Begin(("Gizmo Panel##GizmoPanel" + std::to_string(mPanelId)).c_str(), &open)) {
			ImGui::Text("Operation");
			ImGui::SameLine();
			if (ImGui::Selectable("T", mOperation == Operation::Translation, 0, ImVec2(15, 15))) { mOperation = Operation::Translation; }
			ImGui::SameLine();
			if (ImGui::Selectable("R", mOperation == Operation::Rotation, 0, ImVec2(15, 15))) { mOperation = Operation::Rotation; }
			ImGui::SameLine();
			if (ImGui::Selectable("S", mOperation == Operation::Scale, 0, ImVec2(15, 15))) { mOperation = Operation::Scale; }

			ImGui::SameLine();
			ImGui::Text("|");
			ImGui::SameLine();

			ImGui::Text("Mode");
			ImGui::SameLine();
			if (ImGui::Selectable("World", mWorld, 0, ImVec2(40, 15))) { mWorld = true; }
			ImGui::SameLine();
			if (ImGui::Selectable("Local", !mWorld, 0, ImVec2(40, 15))) { mWorld = false; }
		}
		ImGui::End();

		auto [camera] = mEditor.getEntityDatabase().getComponents<se::app::CameraComponent>(mEditor.getViewportEntity(), true);
		if (!camera) {
			return open;
		}

		glm::mat4 viewMatrix = camera->getViewMatrix();
		glm::mat4 projectionMatrix = camera->getProjectionMatrix();

		ImGuizmo::OPERATION operation = (mOperation == Operation::Translation)? ImGuizmo::OPERATION::TRANSLATE :
			(mOperation == Operation::Rotation)? ImGuizmo::OPERATION::ROTATE : ImGuizmo::OPERATION::SCALE;
		ImGuizmo::MODE mode = mWorld? ImGuizmo::MODE::WORLD : ImGuizmo::MODE::LOCAL;

		auto [transforms] = mEditor.getEntityDatabase().getComponents<se::app::TransformsComponent>(mEditor.getActiveEntity(), true);
		if (!transforms) {
			return open;
		}

		glm::mat4 matrixTransform = se::app::getModelMatrix(*transforms);
		if (ImGuizmo::Manipulate(glm::value_ptr(viewMatrix), glm::value_ptr(projectionMatrix), operation, mode, glm::value_ptr(matrixTransform))) {
			se::utils::decompose(matrixTransform, transforms->position, transforms->orientation, transforms->scale);
			transforms->updated.reset();
		}

		return open;
	}

}