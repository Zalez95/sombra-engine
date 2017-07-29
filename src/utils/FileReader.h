#ifndef FILE_READER_H
#define FILE_READER_H

#include <string>
#include <fstream>
#include <sstream>

namespace utils {

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

		/** The number of lines readed in the current file */
		unsigned int mNumLines;

		/** If there was an error while parsing */
		bool mFailed;

	public:
		/** Creates a new FileLineReader
		 *
		 * @param	path the path of the file that we are going to read */
		FileReader(const std::string& path);

		/** Class destructor */
		~FileReader();

		/** @return	the path of the file that the Reader is currently
		 *			reading */
		inline std::string getFilePath() const { return mPath; };

		/** @return the name of the current file with its extension */
		std::string getFileName() const;

		/** @return	the path of the directory where is located the current
		 *			file */
		std::string getDirectory() const;

		/** @return	the number of readed lines in the current file */
		inline unsigned int getNumLines() const { return mNumLines; };

		/** @return	true if we reached the end of file */
		bool eof() const;

		/** @return	true if there was an error parsinga value or reading the
		 *			file */
		inline bool fail() const { return mFailed; };

		/** Reads the next value and stores it in the given parameter
		 * 
		 * @param	token the variable where we are going to store the readed
		 *			value
		 * @note	if there was an error parsing the parameter, the fail
		 *			function will return true */
		template<typename T>
		void getValue(T& token);

		/** Extraction operator. It does the same than the @see getValue
		 * function, but allowing concatenation

		 * @param	token the variable where we are going to store the readed
		 *			value
		 * @note	if there was an error parsing the parameter, the fail
		 *			function will return true */
		template<typename T>
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
		template<typename T1, typename T2>
		void getValuePair(T1& first, const std::string& separator, T2& second);

		/** @return	true if there aren't any tokens left in the file, false
		 *			otherwise */
		bool isEmpty();

		/** Discards the content of the current line */
		void discardLine();
	};


// Template functions definition
	template<typename T>
	void FileReader::getValue(T& token)
	{
		int t = mCurLineStream.tellg();
		if ((t != -1) &&
			(mCurLineString.find_first_not_of(" \t\n", t) != std::string::npos)
		) {
			// If the current line has still some tokens we parse the next one
			mCurLineStream >> token;
			if (mCurLineStream.fail()) { mFailed = true; }
		}
		else if (!mInputFStream.eof()) {
			// Read the next lines recursively until we find a not empty line
			std::getline(mInputFStream, mCurLineString);

			// Remove the carriage return character located at the end of line
			// if it exists
			if ((!mCurLineString.empty()) &&
				(mCurLineString[mCurLineString.size() - 1] == '\r')
			) {
				mCurLineString = mCurLineString.substr(0, mCurLineString.size() - 1);
			}

			mCurLineStream = std::stringstream(mCurLineString);
			++mNumLines;

			getValue(token);
		}
	}


	template<typename T>
	FileReader& FileReader::operator>>(T& token)
	{
		getValue(token);
		return (*this);
	}


	template<typename T1, typename T2>
	void FileReader::getValuePair(
		T1& first, const std::string& separator, T2& second
	) {
		// Get the pair
		std::string valuePairStr;
		getValue(valuePairStr);
		if (mFailed) return;

		// Get the position of the separator in the string
		size_t separatorPosition = valuePairStr.find(separator);
		if (separatorPosition == std::string::npos) {
			mFailed = true;
			return;
		}

		// Parse the first param
		std::string firstStr = valuePairStr.substr(0, separatorPosition);
		std::stringstream(firstStr) >> first;

		// Parse the second param
		std::string secondStr = valuePairStr.substr(
			separatorPosition + separator.length(),
			std::string::npos
		);
	   	std::stringstream(secondStr) >> second;
	}

}

#endif		// FILE_READER_H
