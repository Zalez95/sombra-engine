#ifndef FILE_READER_H
#define FILE_READER_H

#include <string>
#include <fstream>
#include <sstream>

/**
 * Class FileReader reads a file line by line
 */
class FileReader
{
private:	// Attributes
	/** The path of the file that we are currently reading */
	std::string mPath;

	/** The file that we are currently reading */
	std::ifstream mInputFStream;

	/** The current stream line */
	std::stringstream mCurLineStream;

	/** The number of lines readed in the current file */
	unsigned int mNumLines;

public:
	/** Creates a new FileLineReader */
	FileReader() {};

	/** Creates a new FileLineReader
	 *
	 * @param	path the path of the file that we are going to read */
	FileReader(const std::string& path) :
		mPath(path), mInputFStream(path), mNumLines(0) {};

	/** Class destructor */
	~FileReader() { mInputFStream.close(); };

	/** @return	the path of the file that the Reader is currently reading */
	inline std::string getFilePath() const { return mPath; };

	/** @return the name of the current file with its extension */
	inline std::string getFileName() const
	{ return mPath.substr(mPath.find_last_of("/\\") + 1, std::string::npos); };

	/** @return	the path of the directory where is located the current file */
	inline std::string getDirectory() const
	{ return mPath.substr(0, mPath.find_last_of("/\\")); };

	/** @return	the number of readed lines in the current file */
	inline unsigned int getNumLines() const { return mNumLines; };

	/** @return	true if we reached the end of file */
	inline bool eof() const { return mInputFStream.eof(); };

	/** @return	true if there was an error reading the file */
	inline bool fail() const { return mInputFStream.fail(); };

	/** Reads the next value and stores it in the given parameter
	 * 
	 * @param	token the variables where we are going to store the readed
	 *			value
	 * @return	true if the value was readed and loaded succesfully, false
	 * 			otherwise */
	template<typename T>
	bool getValue(T& token);

	/** Reads the next value and splits it by the given separator, storing the
	 * two resulting values in the given params
	 *
	 * @param	first the first of the tokens where we are going the first
	 * 			readed value
	 * @param	separator the string that is between the values to read
	 * @param	second the second of the tokens where we are going the second
	 * 			readed value
	 * @return	true if the values were readed and loaded succesfully, false
	 * 			otherwise */
	template<typename T1, typename T2>
	bool getValuePair(T1& first, const std::string& separator, T2& second);

	/** Discards the content of the current line */
	inline void discardLine() { mCurLineStream = std::stringstream(); };
};


// Template functions definition
template<typename T>
bool FileReader::getValue(T& token)
{
	bool ret = true;

	if (mCurLineStream.rdbuf()->in_avail() > 0) {
		// If the current line isn't empty we parse the token
		try {
			mCurLineStream >> token;
		}
		catch (std::exception&) {
			ret = false;
		}
	}
	else if (!mInputFStream.eof()) {
		// Read the next lines recursively until we find a not empty line
		std::string stringLine;
		std::getline(mInputFStream, stringLine);

		// Remove the carriage return character located at the end of line if it exists
		if ((!stringLine.empty()) && (stringLine[stringLine.size() - 1] == '\r')) {
			stringLine = stringLine.substr(0, stringLine.size() - 1);
		}

		mCurLineStream = std::stringstream(stringLine);
		++mNumLines;

		ret = getValue(token);
	}
	else {
		ret = false;
	}

	return ret;
};


template<typename T1, typename T2>
bool FileReader::getValuePair(T1& first, const std::string& separator, T2& second)
{
	bool ret = true;
	
	try {
		// Get the pair
		std::string valuePairStr;
		ret = getValue(valuePairStr);

		// Parse the first param
		std::string firstStr = valuePairStr.substr(0, valuePairStr.find(separator));
		std::stringstream(firstStr) >> first;

		// Parse the second param
		std::string secondStr = valuePairStr.substr(first.length() + separator.length(), std::string::npos);
	   	std::stringstream(secondStr) >> second;
	}
	catch (std::exception& e) {
		ret = false;
	}

	return ret;
};

#endif		// FILE_READER_H
