#ifndef I_COMMAND_H
#define I_COMMAND_H

namespace se::app {

	/**
	 * Class ICommand, it's the base class that all commands should inherit from
	 * to get executed after the user input
	 */
	class ICommand
	{
	public:		// Functions
		/** Class destructor */
		virtual ~ICommand() = default;

		/** Executes the current command action */
		virtual void execute() = 0;
	};


	/**
	 * Class MouseCommand, it's the base class that all mouse movement commands
	 * should inherit from to get executed after the user input
	 */
	class MouseCommand : public ICommand
	{
	protected:	// Attributes
		/** The mouse X position */
		double mX;

		/** The mouse Y position */
		double mY;

	public:		// Functions
		/** Creates a new MouseCommand */
		MouseCommand() : mX(0.0), mY(0.0) {};

		/** Class destructor */
		virtual ~MouseCommand() = default;

		/** Sets the mouse position of the MouseCommand
		 *
		 * @param	x the mouse x position
		 * @param	y the mouse y position */
		void setMousePosition(double x, double y) { mX = x; mY = y; };
	};


	/**
	 * Class ScrollCommand, it's the base class that all scroll movement
	 * commands should inherit from to get executed after the user input
	 */
	class ScrollCommand : public ICommand
	{
	protected:	// Attributes
		/** The scroll X position */
		double mX;

		/** The scroll Y position */
		double mY;

	public:		// Functions
		/** Creates a new ScrollCommand */
		ScrollCommand() : mX(0.0), mY(0.0) {};

		/** Class destructor */
		virtual ~ScrollCommand() = default;

		/** Sets the scroll position of the ScrollCommand
		 *
		 * @param	x the scroll x position
		 * @param	y the scroll y position */
		void setScrollPosition(double x, double y) { mX = x; mY = y; };
	};

}

#endif		// I_COMMAND_H
