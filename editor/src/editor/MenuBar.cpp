#include <se/utils/Log.h>
#include <se/app/io/SceneSerializer.h>
#include "DefaultShaderBuilder.h"
#include "DefaultScene.h"
#include "MenuBar.h"
#include "Editor.h"
#include "EntityPanel.h"
#include "ComponentPanel.h"
#include "RepositoryPanel.h"
#include "SceneNodesPanel.h"
#include "Gizmo.h"

using namespace se::app;

namespace editor {

	void MenuBar::render()
	{
		Alert closePopUp("Close?", "Are you sure that you want to close the current Scene?", "Close");
		Alert createPopUp("Close first?", "The current scene must be closed first, are you sure that you want to close the current Scene?", "Close");
		Alert errorPopUp("Error", "Operation failed, check logs for more details", "Close");

		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("New")) {
					if (mEditor.getScene()) {
						createPopUp.show();
					}
					else {
						mEditor.createScene();
						buildDefaultScene(*mEditor.getScene());
					}
				}
				if (ImGui::MenuItem("Open")) {
					if (mEditor.getScene()) {
						createPopUp.show();
					}
					else {
						mWindow.show();
					}
					mOpen = true;
				}
				if (ImGui::BeginMenu("Import", mEditor.getScene())) {
					if (ImGui::MenuItem("GLTF")) {
						mWindow.show();
						mImport = true;
					}
					ImGui::EndMenu();
				}
				if (ImGui::MenuItem("Save", "", false, mEditor.getScene())) {
					mWindow.show();
					mSave = true;
				}
				if (ImGui::MenuItem("Close", "", false, mEditor.getScene())) {
					closePopUp.show();
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("View")) {
				if (ImGui::MenuItem("Entity", "")) {
					mEditor.addPanel(new EntityPanel(mEditor));
				}
				if (ImGui::MenuItem("Component", "")) {
					mEditor.addPanel(new ComponentPanel(mEditor));
				}
				if (ImGui::MenuItem("Repository", "")) {
					mEditor.addPanel(new RepositoryPanel(mEditor));
				}
				if (ImGui::MenuItem("SceneNodes", "")) {
					mEditor.addPanel(new SceneNodesPanel(mEditor));
				}
				if (ImGui::MenuItem("Gizmo", "")) {
					mEditor.addPanel(new Gizmo(mEditor));
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Help")) {
				if (ImGui::MenuItem("Controls", "")) {
					mShowControlsWindow = true;
				}
				if (ImGui::MenuItem("About Sombra", "")) {
					mShowAboutWindow = true;
				}
				ImGui::EndMenu();
			}

			auto io = ImGui::GetIO();
			ImGui::SameLine(ImGui::GetWindowWidth() - 150.0f);
			ImGui::Text("%.1f FPS (%.3f ms)", io.Framerate, 1000.0f / io.Framerate);

			ImGui::EndMenuBar();
		}

		std::string file;
		if (mWindow.execute(file)) {
			se::app::Result result;
			if (mOpen) {
				SOMBRA_INFO_LOG << "Opening " << file << "...";
				mEditor.createScene();
				result = SceneSerializer::deserialize(file, *mEditor.getScene());
				SOMBRA_INFO_LOG << "Open finished";
			}
			else if (mImport) {
				SOMBRA_INFO_LOG << "Importing from " << file << "...";
				DefaultShaderBuilder shaderBuilder(mEditor, mEditor.getScene()->repository);
				auto myReader = se::app::SceneImporter::createSceneImporter(se::app::SceneImporter::FileType::GLTF, shaderBuilder);
				result = myReader->load(file, *mEditor.getScene());
				SOMBRA_INFO_LOG << "Import finished";
			}
			else if (mSave) {
				SOMBRA_INFO_LOG << "Saving to " << file << "...";
				result = SceneSerializer::serialize(file, *mEditor.getScene());
				SOMBRA_INFO_LOG << "Save finished";
			}

			if (!result) {
				errorPopUp.show();
				SOMBRA_ERROR_LOG << result.description();
			}

			mOpen = mImport = mSave = false;
		}

		if (closePopUp.execute()) {
			mEditor.destroyScene();
		}

		if (createPopUp.execute()) {
			mEditor.destroyScene();
			if (mOpen) {
				mWindow.show();
			}
			else {
				mEditor.createScene();
				buildDefaultScene(*mEditor.getScene());
			}
		}

		if (mShowControlsWindow) {
			if (ImGui::Begin("Controls", &mShowControlsWindow, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
				if (ImGui::BeginTable("controls_table", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
					ImGui::TableSetupColumn("Action");
					ImGui::TableSetupColumn("Shortcut");
					ImGui::TableHeadersRow();

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Move Sideways");
					ImGui::TableSetColumnIndex(1);
					ImGui::Text("<Left Shift> + <Alt> + <click>");

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Move forward");
					ImGui::TableSetColumnIndex(1);
					ImGui::Text("<Left Ctrl> + <Alt> + <click>");

					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Rotate");
					ImGui::TableSetColumnIndex(1);
					ImGui::Text("<Alt> + <click>");

					ImGui::EndTable();
				}
			}
			ImGui::End();
		}

		if (mShowAboutWindow) {
			if (ImGui::Begin("About", &mShowAboutWindow, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
				ImGui::Text("Sombra Engine Editor Pre-Alpha");
				ImGui::Separator();
				ImGui::Text("By your Anon fren.");
				ImGui::Text("Sombra Engine is licensed under the MPL2 License, see LICENSE.txt for more information.");
			}
			ImGui::End();
		}

		errorPopUp.execute();
	}

}
