#include <array>
#include <sstream>
#include <chrono>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_access.hpp>
#include "ImGuiUtils.h"
#include <imgui_internal.h>

namespace fs = std::filesystem;

namespace editor {

	bool drawMat3ImGui(const char* name, glm::mat3& matrix)
	{
		bool ret = false;

		glm::vec3 row0 = glm::row(matrix, 0);
		glm::vec3 row1 = glm::row(matrix, 1);
		glm::vec3 row2 = glm::row(matrix, 2);

		auto r0 = std::string(name) + "##" + std::string(name) + "_r0";
		auto r1 = "##" + std::string(name) + "_r1";
		auto r2 = "##" + std::string(name) + "_r2";
		ret |= ImGui::DragFloat3(r0.c_str(), glm::value_ptr(row0), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
		ret |= ImGui::DragFloat3(r1.c_str(), glm::value_ptr(row1), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
		ret |= ImGui::DragFloat3(r2.c_str(), glm::value_ptr(row2), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);

		matrix = glm::row(matrix, 0, row0);
		matrix = glm::row(matrix, 1, row1);
		matrix = glm::row(matrix, 2, row2);

		return ret;
	}


	bool drawMat4ImGui(const char* name, glm::mat4& matrix)
	{
		bool ret = false;

		glm::vec4 row0 = glm::row(matrix, 0);
		glm::vec4 row1 = glm::row(matrix, 1);
		glm::vec4 row2 = glm::row(matrix, 2);
		glm::vec4 row3 = glm::row(matrix, 3);

		auto r0 = std::string(name) + "##" + std::string(name) + "_r0";
		auto r1 = "##" + std::string(name) + "_r1";
		auto r2 = "##" + std::string(name) + "_r2";
		auto r3 = "##" + std::string(name) + "_r3";
		ret |= ImGui::DragFloat4(r0.c_str(), glm::value_ptr(row0), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
		ret |= ImGui::DragFloat4(r1.c_str(), glm::value_ptr(row1), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
		ret |= ImGui::DragFloat4(r2.c_str(), glm::value_ptr(row2), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
		ret |= ImGui::DragFloat4(r3.c_str(), glm::value_ptr(row3), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);

		matrix = glm::row(matrix, 0, row0);
		matrix = glm::row(matrix, 1, row1);
		matrix = glm::row(matrix, 2, row2);
		matrix = glm::row(matrix, 3, row3);

		return ret;
	}


	bool drawMat3x4ImGui(const char* name, glm::mat3x4& matrix)
	{
		bool ret = false;

		glm::vec3 row0 = glm::row(matrix, 0);
		glm::vec3 row1 = glm::row(matrix, 1);
		glm::vec3 row2 = glm::row(matrix, 2);
		glm::vec3 row3 = glm::row(matrix, 3);

		auto r0 = std::string(name) + "##" + std::string(name) + "_r0";
		auto r1 = "##" + std::string(name) + "_r1";
		auto r2 = "##" + std::string(name) + "_r2";
		auto r3 = "##" + std::string(name) + "_r3";
		ret |= ImGui::DragFloat3(r0.c_str(), glm::value_ptr(row0), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
		ret |= ImGui::DragFloat3(r1.c_str(), glm::value_ptr(row1), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
		ret |= ImGui::DragFloat3(r2.c_str(), glm::value_ptr(row2), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
		ret |= ImGui::DragFloat3(r3.c_str(), glm::value_ptr(row3), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);

		matrix = glm::row(matrix, 0, row0);
		matrix = glm::row(matrix, 1, row1);
		matrix = glm::row(matrix, 2, row2);
		matrix = glm::row(matrix, 3, row3);

		return ret;
	}


	bool drawOrientation(const char* name, glm::quat& orientation, int& orientationType)
	{
		bool ret = false;

		static const char* orientationTypes[] = { "Quaternion", "Angle Axis", "Euler angles" };
		if (ImGui::BeginCombo(name, orientationTypes[orientationType])) {
			for (std::size_t i = 0; i < IM_ARRAYSIZE(orientationTypes); ++i) {
				bool isSelected = (static_cast<int>(i) == orientationType);
				if (ImGui::Selectable(orientationTypes[i], isSelected)) {
					orientationType = static_cast<int>(i);
				}
				if (isSelected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		switch (orientationType) {
			case 0: {
				ret |= ImGui::DragFloat4("Quat", glm::value_ptr(orientation), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
			} break;
			case 1: {
				float angle = glm::degrees(glm::angle(orientation));
				glm::vec3 axis = glm::axis(orientation);
				ret |= ImGui::DragFloat("Angle", &angle, 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
				ret |= ImGui::DragFloat3("Axis", glm::value_ptr(axis), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
				if (ret) {
					orientation = glm::angleAxis(glm::radians(angle), axis);
				}
			} break;
			case 2: {
				glm::vec3 eulerAngles = glm::degrees(glm::eulerAngles(orientation));
				ret |= ImGui::DragFloat3("Euler angles", glm::value_ptr(eulerAngles), 0.005f, -FLT_MAX, FLT_MAX, "%.3f", 1.0f);
				if (ret) {
					orientation = glm::quat(glm::radians(eulerAngles));
				}
			} break;
		}

		return ret;
	}


	bool addDropdown(const char* name, const char* options[], std::size_t numOptions, int& selected)
	{
		bool ret = false;

		const char* selectedTag = (selected >= 0)? options[selected] : nullptr;
		if (ImGui::BeginCombo(name, selectedTag)) {
			for (std::size_t i = 0; i < numOptions; ++i) {
				bool isSelected = (static_cast<int>(i) == selected);
				if (ImGui::Selectable(options[i], isSelected)) {
					selected = static_cast<int>(i);
					ret = true;
				}
				if (isSelected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		return ret;
	}


	bool Alert::execute()
	{
		bool ret = false;

		if (mShow) {
			ImGui::OpenPopup(mTitle);
			mShow = false;
		}

		ImVec2 center(0.5f * ImGui::GetIO().DisplaySize.x, 0.5f * ImGui::GetIO().DisplaySize.y);
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		if (ImGui::BeginPopupModal(mTitle, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::Text("%s", mMessage);
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


	bool FileWindow::execute(std::string& outPath)
	{
		bool ret = false;

		if (!mShow) {
			return ret;
		}

		if (!ImGui::Begin("File", &mShow)) {
			ImGui::End();
			return ret;
		}

		// Top bar
		std::array<char, 1024> location = {};
		std::string locationStr = fs::absolute(mCurrentPath).string();
		std::copy(locationStr.begin(), locationStr.end(), location.begin());

		if (ImGui::ArrowButton("up_dir", ImGuiDir_Up)) {
			if (mCurrentPath.has_parent_path()) {
				mCurrentPath = mCurrentPath.parent_path();
			}
		}
		ImGui::SameLine();
		if (ImGui::InputText("", location.data(), location.size(), ImGuiInputTextFlags_EnterReturnsTrue)) {
			fs::path nextCurrentPath(location.data());
			if (fs::exists(nextCurrentPath) && fs::is_directory(nextCurrentPath)) {
				mCurrentPath = nextCurrentPath;
			}
		}
		ImGui::SameLine();
		mFilter.Draw("Search");

		ImGui::Separator();

		// Content
		bool open = false;

		const float reservedForFooter = 6 * ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
		ImGui::BeginChild("ScrollingRegion", ImVec2(0, -reservedForFooter), false, ImGuiWindowFlags_HorizontalScrollbar);
		ImGui::Columns(4, "filecolumns");
		ImGui::Separator();
		ImGui::Text("File"); ImGui::NextColumn();
		ImGui::Text("Last Write"); ImGui::NextColumn();
		ImGui::Text("Type"); ImGui::NextColumn();
		ImGui::Text("Size"); ImGui::NextColumn();
		ImGui::Separator();

		for (auto& p : fs::directory_iterator(mCurrentPath)) {
			fs::path path = p.path();
			std::string filename = path.filename().string();

			if (!mFilter.PassFilter(filename.c_str())) {
				continue;
			}

			// Show name
			if (ImGui::Selectable(filename.c_str(), (filename == mSelected.data()), ImGuiSelectableFlags_SpanAllColumns)) {
				mSelected = {};
				std::copy(filename.begin(), filename.end(), mSelected.data());
			}
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
				open = true;
			}
			ImGui::NextColumn();

			// Symlinks are treated differently because exceptions with broken links
			if (fs::is_symlink(path)) {
				ImGui::NextColumn();
				if (fs::is_symlink(path)) {
					ImGui::Text("Link");
				}
				ImGui::NextColumn();
				ImGui::NextColumn();
			}
			else {
				// Show last modified time
				auto lastWrite = fs::last_write_time(path);
				auto lastWriteTP = (lastWrite - fs::file_time_type::clock::now()) + std::chrono::system_clock::now();
				auto lastWriteT = std::chrono::system_clock::to_time_t(lastWriteTP);

				std::ostringstream oss;
				oss << std::put_time(std::localtime(&lastWriteT), "%d/%m/%Y %X");
				ImGui::Text("%s", oss.str().c_str());
				ImGui::NextColumn();

				// Show type
				if (fs::is_regular_file(path)) {
					ImGui::Text("File");
				}
				else if (fs::is_directory(path)) {
					ImGui::Text("Directory");
				}
				else {
					ImGui::Text("Other");
				}
				ImGui::NextColumn();

				// Show size
				if (fs::is_regular_file(path)) {
					ImGui::Text("%lu", fs::file_size(path));
				}
				ImGui::NextColumn();
			}
		}

		ImGui::Columns(1);
		ImGui::EndChild();
		ImGui::Separator();

		// Footer
		ImGui::InputText("Name", mSelected.data(), mSelected.size(), ImGuiInputTextFlags_EnterReturnsTrue);

		if (ImGui::Button("Close")) {
			mShow = false;
		}
		ImGui::SameLine();
		if (mSelected.empty()) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		}
		if (ImGui::Button("Open") && !mSelected.empty()) {
			open = true;
		}
		if (mSelected.empty()) {
			ImGui::PopItemFlag();
		}

		if (open) {
			fs::path selectedPath = mCurrentPath / mSelected.data();
			mSelected = {};

			if (fs::is_symlink(selectedPath)) {
				fs::path newPath = fs::read_symlink(selectedPath);
				if (!newPath.empty()) {
					mCurrentPath = newPath;
				}
			}
			else if (fs::is_directory(selectedPath)) {
				mCurrentPath = selectedPath;
			}
			else {
				outPath = selectedPath.string();
				mShow = false;
				ret = true;
			}
		}

		ImGui::End();
		return ret;
	}

}
