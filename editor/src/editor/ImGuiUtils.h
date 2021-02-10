#ifndef IMGUI_UTILS_H
#define IMGUI_UTILS_H

#include <string>
#include <filesystem>
#include <glm/glm.hpp>
#include <se/utils/Repository.h>

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


	/** Creates a dropdown button used for selecting a value with type @tparam T
	 * from the given Repository
	 *
	 * @param	tag the tag of the button
	 * @param	buttonName the string to show
	 * @param	repository the Repository that holds the values
	 * @param	selectedValue return parameter */
	template <typename T>
	bool addRepoDropdownButton(
		const char* tag, const char* buttonName,
		se::utils::Repository& repository, std::shared_ptr<T>& selectedValue
	);


	/** Creates a dropdown button used for selecting a value with type @tparam T
	 * from the given Repository
	 *
	 * @param	tag the tag of the button
	 * @param	repository the Repository that holds the values
	 * @param	selectedValue return parameter */
	template <typename T>
	bool addRepoDropdownShowSelected(
		const char* tag,
		se::utils::Repository& repository, std::shared_ptr<T>& selectedValue
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
	private:
		/** If the window must be shown or not */
		bool mShow;

		/** The current path to show */
		std::filesystem::path mCurrentPath;

		/** The file selected */
		std::filesystem::path mSelected;

	public:		// Functions
		/** Creates a new FileWindow */
		FileWindow() :
			mShow(false), mCurrentPath(std::filesystem::current_path()) {};

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
