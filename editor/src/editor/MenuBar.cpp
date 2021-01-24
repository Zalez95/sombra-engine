#include <se/utils/Log.h>
#include "DefaultShaderBuilder.h"
#include "MenuBar.h"
#include "Editor.h"

using namespace se::app;

namespace editor {

	MenuBar::MenuBar(Editor& editor) : mEditor(editor) {}


	void MenuBar::render()
	{
		Alert closePopUp("Close?", "Are you sure that you want to close the current Scene?", "Close");
		Alert createPopUp("Close first?", "The current scene must be closed first, are you sure that you want to close the current Scene?", "Close");
		Alert importErrorPopUp("Error", "Failed to import, see logs for more details", "Close");

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
						mGLTFWindow.show();
					}
					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		std::string file;
		if (mGLTFWindow.execute(file)) {
			DefaultShaderBuilder shaderBuilder(mEditor, mEditor.getScene()->repository);
			auto myReader = se::app::SceneReader::createSceneReader(se::app::SceneReader::FileType::GLTF, shaderBuilder);
			auto result = myReader->load(file, *mEditor.getScene());
			if (!result) {
				importErrorPopUp.show();
				SOMBRA_ERROR_LOG << result.description();
			}
		}

		if (closePopUp.execute()) {
			mEditor.destroyScene();
		}

		if (createPopUp.execute()) {
			mEditor.destroyScene();
			mEditor.createScene();
		}

		importErrorPopUp.execute();
	}

}
