#ifndef IMGUI_UTILS_HPP
#define IMGUI_UTILS_HPP

namespace editor {

	template <typename T>
	bool addRepoDropdownButton(const char* tag, const char* buttonName, se::app::Repository& repository, se::app::Repository::ResourceRef<T>& value)
	{
		bool ret = false;

		if (ImGui::BeginCombo(tag, buttonName)) {
			repository.iterate<T>([&](const se::app::Repository::ResourceRef<T>& ref) {
				bool isSelected = (ref == value);
				if (ImGui::Selectable(ref.getResource().getName(), isSelected)) {
					value = ref;
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
	bool addRepoDropdownShowSelected(const char* tag, se::app::Repository& repository, se::app::Repository::ResourceRef<T>& value)
	{
		const char* buttonName = (value)? value.getResource().getName() : "";
		return addRepoDropdownButton<T>(tag, buttonName, repository, value);
	}


	template <typename T>
	void setRepoName(se::app::Resource<T>& resource, const char* name, se::app::Repository& repository)
	{
		std::size_t i = 1;
		std::string nameStr = name;
		while (true) {
			if (!repository.findByName<T>(nameStr.c_str())) {
				resource.setName(nameStr.c_str());
				return;
			}
			nameStr = name + std::to_string(i++);
		}
	}

}

#endif		// IMGUI_UTILS_HPP
