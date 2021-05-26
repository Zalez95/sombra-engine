#ifndef IMGUI_UTILS_H
#define IMGUI_UTILS_H

#include <array>
#include <string>
#include <filesystem>
#include <imgui.h>
#include <glm/glm.hpp>
#include <se/app/Repository.h>

namespace editor {

	/** Draws a 3x3 matrix
	 *
	 * @param	name the name of the ImGui component
	 * @param	matrix the matrix to update
	 * @return	true if any matrix value was updated, false otherwise */
	bool drawMat3ImGui(const char* name, glm::mat3& matrix);


	/** Draws a 4x4 matrix
	 *
	 * @param	name the name of the ImGui component
	 * @param	matrix the matrix to update
	 * @return	true if any matrix value was updated, false otherwise */
	bool drawMat4ImGui(const char* name, glm::mat4& matrix);


	/** Draws a 3x4 matrix
	 *
	 * @param	name the name of the ImGui component
	 * @param	matrix the matrix to update
	 * @return	true if any matrix value was updated, false otherwise */
	bool drawMat3x4ImGui(const char* name, glm::mat3x4& matrix);


	/** Draws the given orientation
	 *
	 * @param	name the name of the orientation to draw
	 * @param	orientation the orientation stored as a Quaternion
	 * @param	orientationType the type of orientation to show:
	 *			0 = quaternion, 1 = angle axis, 2 = euler angles
	 * @return	true if the orientation was updated, false otherwise */
	bool drawOrientation(
		const char* name, glm::quat& orientation, int& orientationType
	);


	/** Creates a dropdown button used for selecting a value from the given
	 * options
	 *
	 * @param	name the name of the Dropdown
	 * @param	options the different options
	 * @param	numOptions the number of options
	 * @param	selected the current selected value, values below zero mean
	 *			that there is no selected value. It will be modified on user
	 *			selection
	 * @return	true if the dropdown was updated, false otherwise */
	bool addDropdown(
		const char* name, const char* options[], std::size_t numOptions,
		int& selected
	);


	/** Creates a dropdown button used for selecting a value with the given key
	 * from the given Repository
	 *
	 * @param	tag the tag of the button
	 * @param	buttonName the string to show
	 * @param	repository the Repository that holds the values
	 * @param	key return parameter
	 * @return	true if the dropdown was updated, false otherwise */
	template <typename K, typename V>
	bool addRepoDropdownButton(
		const char* tag, const char* buttonName,
		se::app::Repository& repository, K& key
	);


	/** Creates a dropdown button used for selecting a value with type @tparam V
	 * from the given Repository
	 *
	 * @param	tag the tag of the button
	 * @param	buttonName the string to show
	 * @param	repository the Repository that holds the values
	 * @param	value return parameter
	 * @return	true if the dropdown was updated, false otherwise */
	template <typename K, typename V>
	bool addRepoDropdownButtonValue(
		const char* tag, const char* buttonName,
		se::app::Repository& repository, std::shared_ptr<V>& value
	);


	/** Creates a dropdown button used for selecting a value with the given key
	 * from the given Repository
	 *
	 * @param	tag the tag of the button
	 * @param	repository the Repository that holds the values
	 * @param	key the current selected value key, also the return parameter
	 * @return	true if the dropdown was updated, false otherwise */
	template <typename K, typename V>
	bool addRepoDropdownShowSelected(
		const char* tag, se::app::Repository& repository, K& key
	);


	/** Creates a dropdown button used for selecting a value with type @tparam V
	 * from the given Repository
	 *
	 * @param	tag the tag of the button
	 * @param	repository the Repository that holds the values
	 * @param	value the current selected value, also the return parameter
	 * @return	true if the dropdown was updated, false otherwise */
	template <typename K, typename V>
	bool addRepoDropdownShowSelectedValue(
		const char* tag, se::app::Repository& repository,
		std::shared_ptr<V>& value
	);


	/**
	 * Class Alert, it's a pop up used for alerting the user of something
	 */
	class Alert
	{
	private:	// Attributes
		/** The text that the pop up window title must show */
		const char* mTitle;

		/** The text that the pop up window must show */
		const char* mMessage;

		/** The text that the accept button must show */
		const char* mButton;

		/** If the pop up must be shown or not */
		bool mShow;

	public:		// Functions
		/** Creates a new Alert
		 *
		 * @param	title the text that the pop up window title
		 * @param	message the text that the pop up window must show
		 * @param	button the text that the accept button must show */
		Alert(const char* title, const char* message, const char* button) :
			mTitle(title), mMessage(message), mButton(button), mShow(false) {};

		/** If the pop up must be shown */
		void show() { mShow = true; };

		/** Draws the current pop up if @see show was called before
		 *
		 * @return	true if the pop up accept button was pressed, false
		 *			otherwise */
		bool execute();
	};


	/**
	 * Class FileWindow, it's a window used for selecting files from the file
	 * system
	 */
	class FileWindow
	{
	private:	// Attributes
		/** The maximum length of a Filename */
		static constexpr std::size_t kMaxFilename = 256;

		/** If the window must be shown or not */
		bool mShow = false;

		/** The current path to show */
		std::filesystem::path mCurrentPath;

		/** The filter for the filenames */
		ImGuiTextFilter mFilter;

		/** The selected filename */
		std::array<char, kMaxFilename> mSelected = {};

	public:		// Functions
		/** Creates a new FileWindow */
		FileWindow() : mCurrentPath(std::filesystem::current_path()) {};

		/** If the window must be shown */
		void show() { mShow = true; };

		/** Draws the current window if @see show was called before
		 *
		 * @param	outPath the path of the file selected (return value)
		 * @return	true if a file was selected, false otherwise */
		bool execute(std::string& outPath);
	};

}

#include "ImGuiUtils.hpp"

#endif		// IMGUI_UTILS_H
