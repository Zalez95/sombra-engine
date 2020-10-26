#ifndef ACCEPT_POP_UP_H
#define ACCEPT_POP_UP_H

namespace editor {

	/**
	 * Class AcceptPopUp, It's the pop up used for requesting the user to
	 * accept something
	 */
	class AcceptPopUp
	{
	private:	// Attributes
		/** The text that the pop up window title must show */
		const char* mTitle;

		/** The text that the pop up window must show */
		const char* mMessage;

		/** The text that the accept button must show */
		const char* mButton;

		/** If the window must be shown or not */
		bool mShow;

	public:		// Functions
		/** Creates a new AcceptPopUp
		 *
		 * @param	title the text that the pop up window title
		 * @param	message the text that the pop up window must show
		 * @param	button the text that the accept button must show */
		AcceptPopUp(const char* title, const char* message, const char* button);

		/** If the pop up must be shown */
		void show();

		/** Draws the current pop up if @see show was called before
		 *
		 * @return	true if the pop up accept button was pressed, false
		 *			otherwise */
		bool execute();
	};

}

#endif		// ACCEPT_POP_UP_H
