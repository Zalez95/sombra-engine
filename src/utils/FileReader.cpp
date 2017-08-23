#include "FileReader.h"

namespace utils {

	FileReader::FileReader(const std::string& path) :
		mPath(path), mInputFStream(path), mNumLines(0), mFailed(false)
	{
		mInputFString = std::string(
			std::istreambuf_iterator<char>(mInputFStream),
			std::istreambuf_iterator<char>()
		);

		mInputFStream.clear();
		mInputFStream.seekg(0, std::ios::beg);
	}


	FileReader::~FileReader()
	{
		mInputFStream.close();
	}


	std::string FileReader::getFileName() const
	{
		return mPath.substr(mPath.find_last_of("/\\") + 1, std::string::npos);
	}


	std::string FileReader::getDirectory() const
	{
		return mPath.substr(0, mPath.find_last_of("/\\") + 1);
	}


	bool FileReader::eof() const
	{
		return mCurLineStream.eof() && mInputFStream.eof();
	}


	bool FileReader::isEmpty()
	{
		int t1 = mCurLineStream.tellg();
		bool emptyLine = (
			(t1 == -1) ||
			(mCurLineString.find_first_not_of(" \t\n", t1) == std::string::npos)
		);

		int t2 = mInputFStream.tellg();
		bool emptyFile = (
			(t2 == -1) ||
			(mInputFString.find_first_not_of(" \t\n", t2) == std::string::npos)
		);

		return (emptyLine && emptyFile);
	}


	void FileReader::discardLine()
	{
		mCurLineStream.clear();
		mCurLineStream.str(std::string());

		mCurLineString = "";
	}

}
