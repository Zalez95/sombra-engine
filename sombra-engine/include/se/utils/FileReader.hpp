#ifndef FILE_READER_HPP
#define FILE_READER_HPP

namespace se::utils {

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

#endif		// FILE_READER_HPP
