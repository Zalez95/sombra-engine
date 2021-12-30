#include <se/utils/Log.h>
#include <se/utils/ThreadPool.h>
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
		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("New")) {
					mCreate = true;
				}
				if (ImGui::MenuItem("Open")) {
					mOpen = true;
				}
				if (ImGui::MenuItem("Append")) {
					mAppend = true;
				}
				if (ImGui::MenuItem("Link")) {
					mLink = true;
				}
				if (ImGui::BeginMenu("Import", mEditor.getScene())) {
					if (ImGui::MenuItem("GLTF")) {
						mImport = true;
					}
					ImGui::EndMenu();
				}
				if (ImGui::MenuItem("Save", "", false, mEditor.getScene())) {
					mSave = true;
				}
				if (ImGui::MenuItem("Close", "", false, mEditor.getScene())) {
					mClose = true;
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

			ImGui::EndMainMenuBar();
		}

		doError();
		closeScene();
		createScene();
		doWindow();
		showControls();
		showAbout();
	}


	void MenuBar::doError()
	{
		if (!mError.empty()) {
			Alert errorPopUp("MenuBar::errorAlert", "Error", ("Operation failed: " + mError).c_str(), "Ok");
			switch (errorPopUp.execute()) {
				case Alert::Result::Nothing:
					break;
				default:
					mError = "";
					mCancel = true;
					break;
			}
		}
	}


	void MenuBar::closeScene()
	{
		static Alert closePopUp("MenuBar::closeAlert", "Close?", "Are you sure that you want to close the current Scene?", "Close");

		if (!mClose) { return; }

		if (mEditor.getScene()) {
			switch (closePopUp.execute()) {
				case Alert::Result::Button:
					mEditor.destroyScene();
					mClose = false;
					break;
				case Alert::Result::Cancel:
					mClose = false;
					mCancel = true;
					break;
				default:
					break;
			}
		}
		else {
			mClose = false;
		}
	}


	void MenuBar::createScene()
	{
		if (mCreate) {
			if (mCancel) {
				mCancel = mCreate = false;
			}
			else if (!mClose) {
				if (mEditor.getScene()) {
					mClose = true;
				}
				else {
					mEditor.createScene();
					buildDefaultScene(*mEditor.getScene());
					mCreate = false;
				}
			}
		}
	}


	void MenuBar::doWindow()
	{
		if (!mOpen && !mAppend && !mLink && !mImport && !mSave) { return; }

		if (mCancel) {
			mCancel = mOpen = mAppend = mLink = mImport = mSave = false;
		}

		std::string file;
		switch (mWindow.execute(file)) {
			case FileWindow::Result::Open: {
				if (mOpen) {
					SOMBRA_INFO_LOG << "Opening " << file << "...";
					mFutureResult = mEditor.getThreadPool().async([this, file]() {
						mEditor.createScene();
						auto result = SceneSerializer::deserialize(file, *mEditor.getScene());
						SOMBRA_INFO_LOG << "Open finished";
						return result;
					});
				}
				else if (mAppend) {
					SOMBRA_INFO_LOG << "Appending " << file << "...";
					mFutureResult = mEditor.getThreadPool().async([this, file]() {
						auto result = SceneSerializer::deserialize(file, *mEditor.getScene());
						SOMBRA_INFO_LOG << "Append finished";
						return result;
					});
				}
				else if (mLink) {
					SOMBRA_INFO_LOG << "Linking " << file << "...";
					mFutureResult = mEditor.getThreadPool().async([this, file]() {
						SOMBRA_INFO_LOG << "Link finished";
						return Result(false, "TODO:");
					});
				}
				else if (mImport) {
					SOMBRA_INFO_LOG << "Importing from " << file << "...";
					mFutureResult = mEditor.getThreadPool().async([this, file]() {
						DefaultShaderBuilder shaderBuilder(mEditor, mEditor.getScene()->repository);
						auto myReader = se::app::SceneImporter::createSceneImporter(se::app::SceneImporter::FileType::GLTF, shaderBuilder);
						auto result = myReader->load(file, *mEditor.getScene());
						SOMBRA_INFO_LOG << "Import finished";
						return result;
					});
				}
				else if (mSave) {
					SOMBRA_INFO_LOG << "Saving to " << file << "...";
					mFutureResult = mEditor.getThreadPool().async([this, file]() {
						auto result = SceneSerializer::serialize(file, *mEditor.getScene());
						SOMBRA_INFO_LOG << "Save finished";
						return result;
					});
				}

				if (mFutureResult.valid() && se::utils::is_ready(mFutureResult)) {
					auto result = mFutureResult.get();
					if (!result) {
						mError = result.description();
						SOMBRA_ERROR_LOG << result.description();
					}
					else {
						mOpen = mAppend = mLink = mImport = mSave = false;
					}
				}
			} break;
			case FileWindow::Result::Cancel:
				mCancel = true;
				break;
			default:
				break;
		}
	}


	void MenuBar::showControls()
	{
		if (!mShowControlsWindow) { return; }

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


	void MenuBar::showAbout()
	{
		if (!mShowAboutWindow) { return; }

		if (ImGui::Begin("About", &mShowAboutWindow, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
			ImGui::Text("Sombra Engine Editor Pre-Alpha");
			ImGui::Separator();
			ImGui::Text("By your Anon fren.");
			ImGui::Text("Sombra Engine is licensed under the MPL2 License, see LICENSE.txt for more information.");
		}
		ImGui::End();
	}

}
