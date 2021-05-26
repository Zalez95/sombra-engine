#ifndef IMGUI_UTILS_HPP
#define IMGUI_UTILS_HPP

namespace editor {

	template <typename K, typename V>
	bool addRepoDropdownButton(const char* tag, const char* buttonName, se::app::Repository& repository, K& key)
	{
		bool ret = false;

		if (ImGui::BeginCombo(tag, buttonName)) {
			repository.iterate<K, V>([&](const K& k, std::shared_ptr<V>&) {
				bool isSelected = (k == key);
				if (ImGui::Selectable(k.c_str(), isSelected)) {
					key = k;
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

	template <typename K, typename V>
	bool addRepoDropdownButtonValue(const char* tag, const char* buttonName, se::app::Repository& repository, std::shared_ptr<V>& value)
	{
		bool ret = false;

		if (ImGui::BeginCombo(tag, buttonName)) {
			repository.iterate<K, V>([&](const K& k, std::shared_ptr<V>& v) {
				bool isSelected = (v == value);
				if (ImGui::Selectable(k.c_str(), isSelected)) {
					value = v;
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


	template <typename K, typename V>
	bool addRepoDropdownShowSelected(const char* tag, se::app::Repository& repository, K& key)
	{
		return addRepoDropdownButton<K, V>(tag, key.c_str(), repository, key);
	}


	template <typename K, typename V>
	bool addRepoDropdownShowSelectedValue(const char* tag, se::app::Repository& repository, std::shared_ptr<V>& value)
	{
		K key;
		repository.findKey<K, V>(value, key);
		return addRepoDropdownButtonValue<K, V>(tag, key.c_str(), repository, value);
	}

}

#endif		// IMGUI_UTILS_HPP
