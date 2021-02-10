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

		if (!ImGui::Begin("File")) {
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
		if (ImGui::InputText("Location", location.data(), location.size(), ImGuiInputTextFlags_EnterReturnsTrue)) {
			fs::path nextCurrentPath(location.data());
			if (fs::exists(nextCurrentPath) && fs::is_directory(nextCurrentPath)) {
				mCurrentPath = nextCurrentPath;
			}
		}

		ImGui::Separator();

		// Content
		bool open = false;

		const float reservedForFooter = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
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

			// Show name
			std::string filename = path.filename().string();
			if (ImGui::Selectable(filename.c_str(), (path == mSelected), ImGuiSelectableFlags_SpanAllColumns)) {
				mSelected = path;
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
			if (fs::is_symlink(mSelected)) {
				fs::path newPath = fs::read_symlink(mSelected);
				if (!newPath.empty()) {
					mCurrentPath = newPath;
				}
			}
			else if (fs::is_directory(mSelected)) {
				mCurrentPath = mSelected;
			}
			else {
				outPath = mSelected.string();
				mShow = false;
				ret = true;
			}
		}

		ImGui::End();
		return ret;
	}

}
