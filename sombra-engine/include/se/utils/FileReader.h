#ifndef FILE_READER_H
#define FILE_READER_H

#include <string>
#include <fstream>
#include <sstream>

namespace se::utils {

	enum class FileState
	{
		OK,
		ENDED,
		FAILED
	};


	/**
	 * Class FileReader reads a file line by line
	 */
	class FileReader
	{
	private:	// Attributes
		/** The path of the file that we are currently reading */
		std::string mPath;

		/** The file that we are currently reading (ifstream) */
		std::ifstream mInputFStream;

		/** The file that we are currently reading (string) */
		std::string mInputFString;

		/** The current line (sstream) */
		std::stringstream mCurLineStream;

		/** The current line (string) */
		std::string mCurLineString;

		/** The state of the current file */
		FileState mCurrentState;

		/** The number of lines readed in the current file */
		unsigned int mNumLines;

	public:		// Functions
		/** Creates a new FileLineReader
		 *
		 * @param	path the path to the file that we are going to read */
		FileReader(const std::string& path);

		/** Class destructor */
		~FileReader();

		/** @return	the path to the file that the FileReader is currently
		 *			reading */
		inline std::string getFilePath() const { return mPath; };

		/** @return the name of the current file with its extension */
		std::string getFileName() const;

		/** @return	the path to the directory where is located the current
		 *			file */
		std::string getDirectory() const;

		/** @return	the current state of the file */
		inline FileState getState() const { return mCurrentState; };

		/** @return	the number of readed lines in the current file */
		inline unsigned int getNumLines() const { return mNumLines; };

		/** Reads the next value and stores it in the given parameter
		 *
		 * @param	token the variable where we are going to store the readed
		 *			value
		 * @note	if there was an error parsing the parameter, the fail
		 *			function will return true */
		template <typename T>
		FileState getValue(T& token);

		/** Extraction operator. It does the same than the @see getValue
		 * function, but allowing concatenation

		 * @param	token the variable where we are going to store the readed
		 *			value
		 * @note	if there was an error parsing the parameter, the fail
		 *			function will return true */
		template <typename T>
		FileReader& operator>>(T& token);

		/** Reads the next value and splits it by the given separator, storing
		 * the two resulting values in the given params
		 *
		 * @param	first the first of the tokens where we are going the first
		 * 			readed value
		 * @param	separator the string that is between the values to read
		 * @param	second the second of the tokens where we are going the
		 *			second readed value
		 * @return	true if the values were readed and loaded succesfully,
		 *			false otherwise */
		template <typename T1, typename T2>
		FileState getValuePair(
			T1& first, const std::string& separator, T2& second
		);

		/** Discards the contents of the current line and moves to the next
		 * one */
		void discardLine();
	};


// Template functions definition
	template <typename T>
	FileState FileReader::getValue(T& token)
	{
		int t = mCurLineStream.tellg();
		if ((t != -1)
			&& (mCurLineString.find_first_not_of(" \t\r\n", t)
				!= std::string::npos)
		) {
			// If the current line has still some tokens we parse the next one
			mCurLineStream >> token;
			if (mCurLineStream.fail()) {
				mCurrentState = FileState::FAILED;
			}
		}
		else if (!mInputFStream.eof()) {
			// Read the next lines recursively until we find a not empty line
			std::getline(mInputFStream, mCurLineString);
			mCurLineStream = std::stringstream(mCurLineString);
			++mNumLines;

			getValue(token);
		}
		else {
			mCurrentState = FileState::ENDED;
		}

		return mCurrentState;
	}


	template <typename T>
	FileReader& FileReader::operator>>(T& token)
	{
		getValue(token);
		return (*this);
	}


	template <typename T1, typename T2>
	FileState FileReader::getValuePair(
		T1& first, const std::string& separator, T2& second
	) {
		// Get the token
		std::string token;
		if (getValue(token) != FileState::OK) {
			return mCurrentState;
		}

		// Get the position of the separator in the token
		std::size_t separatorPosition = token.find(separator);
		if (separatorPosition == std::string::npos) {
			mCurrentState = FileState::FAILED;
			return mCurrentState;
		}

		// Split the token
		std::string firstStr = token.substr(0, separatorPosition);
		std::string secondStr = token.substr(
			separatorPosition + separator.length(),
			std::string::npos
		);

		// Parse the data
		std::stringstream(firstStr) >> first;
		std::stringstream(secondStr) >> second;
		if (mCurLineStream.fail()) {
			mCurrentState = FileState::FAILED;
		}

		return mCurrentState;
	}

}

#endif		// FILE_READER_H
