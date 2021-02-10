#ifndef RESULT_H
#define RESULT_H

#include <string>

namespace se::app {

	/**
	 * Class Result, it holds the result of some operation
	 */
	class Result
	{
	private:	// Attributes
		/** If there was an error or not */
		bool mGood;

		/** The error message */
		std::string mErrorMessage;

	public:		// Functions
		/** Creates a new Result object
		 *
		 * @param	good true if the operation was successfull (by default),
		 *			false otherwise.
		 * @param	errorMessage the error message, empty by default */
		Result(bool good = true, const std::string& errorMessage = "") :
			mGood(good), mErrorMessage(errorMessage) {};

		/** @return	true if there the load operation was successfull, false
		 *			otherwise */
		operator bool() const { return mGood; };

		/** @return	the error description */
		const char* description() const { return mErrorMessage.c_str(); };
	};

}

#endif		// RESULT_H
