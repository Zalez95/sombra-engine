#ifndef TAG_COMPONENT_H
#define TAG_COMPONENT_H

#include <array>
#include <string_view>

namespace se::app {

	/**
	 * Class TagComponent, it's a Component that holds the name tag of an
	 * Entity
	 */
	class TagComponent
	{
	public:		// Attributes
		/** The maximum length allowed */
		static constexpr std::size_t kMaxLength = 256;

	private:
		/** The name of the Tag */
		std::array<char, kMaxLength> mName;

		/** The length @see mName */
		std::size_t mLength;

	public:		// Functions
		/** Creates a new TagComponent
		 *
		 * @param	name the name of the new TagComponent */
		TagComponent(std::string_view name = "") : mName{} { setName(name); };

		/** @return	the name of the Tag */
		const char* getName() const { return mName.data(); };

		/** @return	the length of the name of the Tag */
		std::size_t getLength() const { return mLength; };

		/** Sets the name of the TagComponent
		 *
		 * @param	name the new name of the TagComponent */
		void setName(std::string_view name = "")
		{
			mLength = name.size();
			if (mLength >= kMaxLength) {
				mLength = kMaxLength - 1;
			}
			name.copy(mName.data(), mLength);
		};
	};

}

#endif		// TAG_COMPONENT_H
