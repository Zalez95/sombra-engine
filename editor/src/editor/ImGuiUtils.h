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


	/** Creates a dropdown button used for selecting a value of @tparam T type
	 * from the given Repository
	 *
	 * @param	tag the tag of the button
	 * @param	buttonName the string to show
	 * @param	repository the Repository that holds the values
	 * @param	value the selected value (return parameter)
	 * @return	true if the dropdown was updated, false otherwise */
	template <typename T>
	bool addRepoDropdownButton(
		const char* tag, const char* buttonName,
		se::app::Repository& repository,
		se::app::Repository::ResourceRef<T>& value
	);


	/** Creates a dropdown button used for selecting a value of @tparam T type
	 * from the given Repository
	 *
	 * @param	tag the tag of the button
	 * @param	repository the Repository that holds the values
	 * @param	value the current selected value, it's name will be shown in
	 *			the dropdown, if it's invalid it will show an empty string.
	 *			It's also the return parameter
	 * @return	true if the dropdown was updated, false otherwise */
	template <typename T>
	bool addRepoDropdownShowSelected(
		const char* tag, se::app::Repository& repository,
		se::app::Repository::ResourceRef<T>& value
	);


	/** Sets the name of the given resource, appending a number in case that
	 * another resource with the same type and name already exists in the
	 * repository
	 *
	 * @param	value the Resource to set it's name
	 * @param	name the new name of the Resource
	 * @param	repository the Repository where the resource is located */
	template <typename T>
	void setRepoName(
		se::app::Resource<T>& resource,
		const char* name, se::app::Repository& repository
	);


	/**
	 * Class Alert, it's a pop up used for alerting the user of something
	 */
	class Alert
	{
	public:		// Nested types
		/** Which action was selected by the User */
		enum class Result : int { Nothing = 0, Cancel, Button };

	private:	// Attributes
		/** The tag used in the Alert ImGui elements */
		std::string mTag;

		/** The text that the pop up window title must show */
		std::string mTitle;

		/** The text that the pop up window must show */
		std::string mMessage;

		/** The text that the accept button must show */
		std::string mButton;

	public:		// Functions
		/** Creates a new Alert
		 *
		 * @param	tag the tag used in the Alert ImGui elements
		 * @param	title the text that the pop up window title
		 * @param	message the text that the pop up window must show
		 * @param	button the text that the accept button must show */
		Alert(
			const char* tag,
			const char* title, const char* message, const char* button
		) : mTag(tag),
			mTitle(title), mMessage(message), mButton(button) {};

		/** Draws the current pop up if @see show was called before
		 *
		 * @return	the Result of the user interaction with the Alert window */
		Result execute();
	};


	/**
	 * Class FileWindow, it's a window used for selecting files from the file
	 * system
	 */
	class FileWindow
	{
	public:		// Nested types
		/** Which action was selected by the User */
		enum class Result : int { Nothing = 0, Cancel, Open };

	private:	// Attributes
		/** The maximum length of a Filename */
		static constexpr std::size_t kMaxFilename = 256;

		/** The prefix used for all the ImGui elements of the FileWindow */
		std::string mTag;

		/** The current path to show */
		std::filesystem::path mCurrentPath;

		/** The filter for the filenames */
		ImGuiTextFilter mFilter;

		/** The selected filename */
		std::array<char, kMaxFilename> mSelected = {};

	public:		// Functions
		/** Creates a new FileWindow
		 *
		 * @param	tag the prefix used for all the ImGui elements of the
		 *			FileWindow */
		FileWindow(const char* tag) :
			mTag(tag), mCurrentPath(std::filesystem::current_path()) {};

		/** Draws the current window if @see show was called before
		 *
		 * @param	outPath the path of the file selected (return value)
		 * @return	true if a file was selected, false otherwise */
		Result execute(std::string& outPath);
	};

}

#include "ImGuiUtils.hpp"

#endif		// IMGUI_UTILS_H
