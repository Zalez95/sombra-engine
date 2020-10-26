#include <imgui.h>
#include "AcceptPopUp.h"

namespace editor {

	AcceptPopUp::AcceptPopUp(const char* title, const char* message, const char* button) :
		mTitle(title), mMessage(message), mButton(button), mShow(false) {}


	void AcceptPopUp::show()
	{
		mShow = true;
	}


	bool AcceptPopUp::execute()
	{
		bool ret = false;

		if (mShow) {
			ImGui::OpenPopup(mTitle);
			mShow = false;
		}

		ImVec2 center(0.5f * ImGui::GetIO().DisplaySize.x, 0.5f * ImGui::GetIO().DisplaySize.y);
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		if (ImGui::BeginPopupModal(mTitle, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::Text(mMessage);
			ImGui::Separator();

			if (ImGui::Button(mButton)) {
				ImGui::CloseCurrentPopup();
				ret = true;
			}
			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) {
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		return ret;
	}

}
