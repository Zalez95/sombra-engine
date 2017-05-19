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
	inline std::string getCurrentFilePath() const { return mPath; };

	/** @return	the number of readed lines in the current file */
	inline unsigned int getNumLines() const { return mNumLines; };

	/** Reads the next value and stores it in the given parameter
	 * 
	 * @param	token the variables where we are going to store the readed
	 *			value
	 * @return	true if the value was readed and loaded succesfully, false
	 * otherwise */
	template<typename T> bool getParam(T& token);

	/** @return	true if we reached the end of file */
	inline bool eof() const { return mInputFStream.eof(); };

	/** @return	true if there was an error reading the file */
	inline bool fail() const { return mInputFStream.fail(); };
};


// Template function definitions
template<typename T> bool FileReader::getParam(T& token)
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

		ret = getParam(token);
	}
	else {
		ret = false;
	}

	return ret;
};

#endif		// FILE_READER_H
