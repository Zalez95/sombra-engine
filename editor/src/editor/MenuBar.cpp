#include <imgui.h>
#include "MenuBar.h"
#include "Editor.h"
#include "AcceptPopUp.h"

using namespace se::app;

namespace editor {

	MenuBar::MenuBar(Editor& editor) : mEditor(editor) {}


	void MenuBar::render()
	{
		AcceptPopUp closePopUp("Close?", "Are you sure that you want to close the current Scene?", "Close");
		AcceptPopUp createPopUp("Close first?", "The current scene must be closed first, are you sure that you want to close the current Scene?", "Close");

		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("New Scene")) {
					if (mEditor.getScene()) {
						createPopUp.show();
					}
					else {
						mEditor.createScene();
					}
				}
				if (ImGui::MenuItem("Close Scene", "", false, mEditor.getScene())) {
					closePopUp.show();
				}
				if (ImGui::BeginMenu("Import", mEditor.getScene())) {
					if (ImGui::MenuItem("GLTF")) {
						
					}
					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		if (closePopUp.execute()) {
			mEditor.destroyScene();
		}

		if (createPopUp.execute()) {
			mEditor.destroyScene();
			mEditor.createScene();
		}
	}

}
