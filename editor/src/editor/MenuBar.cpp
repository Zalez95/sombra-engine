#include <se/utils/Log.h>
#include <se/app/io/SceneSerializer.h>
#include "DefaultShaderBuilder.h"
#include "DefaultScene.h"
#include "MenuBar.h"
#include "Editor.h"

using namespace se::app;

namespace editor {

	MenuBar::MenuBar(Editor& editor) :
		mEditor(editor), mOpen(false), mImport(false), mSave(false) {}


	void MenuBar::render()
	{
		Alert closePopUp("Close?", "Are you sure that you want to close the current Scene?", "Close");
		Alert createPopUp("Close first?", "The current scene must be closed first, are you sure that you want to close the current Scene?", "Close");
		Alert errorPopUp("Error", "Operation failed, check logs for more details", "Close");

		if (ImGui::BeginMainMenuBar()) {
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
			ImGui::EndMainMenuBar();
		}

		std::string file;
		if (mWindow.execute(file)) {
			se::app::Result result;
			if (mOpen) {
				SOMBRA_INFO_LOG << "Opening " << file << "...";
				mEditor.createScene();
				result = SceneSerializer(mEditor.getEntityDatabase()).deserialize(file, *mEditor.getScene());
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
				result = SceneSerializer(mEditor.getEntityDatabase()).serialize(file, *mEditor.getScene());
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

		errorPopUp.execute();
	}

}
