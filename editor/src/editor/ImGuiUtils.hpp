#ifndef IMGUI_UTILS_HPP
#define IMGUI_UTILS_HPP

#include <imgui.h>

namespace editor {

	template <typename T>
	bool addRepoDropdownButton(const char* tag, const char* buttonName, se::utils::Repository& repository, std::shared_ptr<T>& selectedValue)
	{
		bool ret = false;

		if (ImGui::BeginCombo(tag, buttonName)) {
			repository.iterate<std::string, T>([&](const std::string& k, std::shared_ptr<T>& v) {
				bool isSelected = (v == selectedValue);
				if (ImGui::Selectable(k.c_str(), isSelected)) {
					selectedValue = v;
					ret = true;
				}
				if (isSelected) {
					ImGui::SetItemDefaultFocus();
				}
			});
			ImGui::EndCombo();
		}

		return ret;
	}


	template <typename T>
	bool addRepoDropdownShowSelected(const char* tag, se::utils::Repository& repository, std::shared_ptr<T>& selectedValue)
	{
		bool ret = false;

		std::string selectedValueName = "";
		repository.findKey<std::string, T>(selectedValue, selectedValueName);

		if (ImGui::BeginCombo(tag, selectedValueName.c_str())) {
			repository.iterate<std::string, T>([&](const std::string& k, std::shared_ptr<T>& v) {
				bool isSelected = (v == selectedValue);
				if (ImGui::Selectable(k.c_str(), isSelected)) {
					selectedValue = v;
					ret = true;
				}
				if (isSelected) {
					ImGui::SetItemDefaultFocus();
				}
			});
			ImGui::EndCombo();
		}

		return ret;
	}

}

#endif		// IMGUI_UTILS_HPP
