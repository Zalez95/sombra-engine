#include "se/utils/FileReader.h"

namespace se::utils {

	FileReader::FileReader(const std::string& path) :
		mPath(path), mInputFStream(path), mCurrentState(FileState::OK), mNumLines(0)
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


	void FileReader::discardLine()
	{
		mCurLineStream.clear();
		mCurLineStream.str(std::string());

		mCurLineString = "";
	}

}
