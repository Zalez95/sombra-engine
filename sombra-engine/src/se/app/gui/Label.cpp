#include <cctype>
#include <algorithm>
#include "se/app/gui/Label.h"
#include "se/utils/StringUtils.h"

namespace se::app {

	Label::Label(graphics::Layer2D* layer2D) :
		mLayer2D(layer2D), mCharacterSize(0.0f),
		mHorizontalAlignment(HorizontalAlignment::Left),
		mVerticalAlignment(VerticalAlignment::Top),
		mColor(0.0f, 0.0f, 0.0f, 1.0f)
	{
		setVisibility(true);
	}


	Label::Label(const Label& other) :
		IComponent(other),
		mLayer2D(other.mLayer2D),
		mFont(other.mFont),
		mCharacterSize(other.mCharacterSize),
		mHorizontalAlignment(other.mHorizontalAlignment),
		mVerticalAlignment(other.mVerticalAlignment),
		mColor(other.mColor),
		mFullText(other.mFullText)
	{
		mRenderableTexts.reserve(other.mRenderableTexts.size());
		for (const auto& renderableText : other.mRenderableTexts) {
			mRenderableTexts.emplace_back( std::make_unique<graphics::RenderableText>(*renderableText) );
		}
	}


	Label::~Label()
	{
		setVisibility(false);
	}


	Label& Label::operator=(const Label& other)
	{
		IComponent::operator=(other);
		mLayer2D = other.mLayer2D;
		mFont = other.mFont;
		mCharacterSize = other.mCharacterSize;
		mHorizontalAlignment = other.mHorizontalAlignment;
		mVerticalAlignment = other.mVerticalAlignment;
		mColor = other.mColor;
		mFullText = other.mFullText;

		mRenderableTexts.reserve(other.mRenderableTexts.size());
		for (const auto& renderableText : other.mRenderableTexts) {
			mRenderableTexts.emplace_back( std::make_unique<graphics::RenderableText>(*renderableText) );
		}

		return *this;
	}


	void Label::setPosition(const glm::vec2& position)
	{
		IComponent::setPosition(position);
		updateRenderableTexts();
	}


	void Label::setSize(const glm::vec2& size)
	{
		IComponent::setSize(size);
		updateRenderableTexts();
	}


	void Label::setZIndex(unsigned char zIndex)
	{
		for (auto& renderable : mRenderableTexts) {
			mLayer2D->removeRenderableText(renderable.get(), mZIndex);
		}

		IComponent::setZIndex(zIndex);

		for (auto& renderable : mRenderableTexts) {
			mLayer2D->addRenderableText(renderable.get(), mZIndex);
		}
	}


	void Label::setVisibility(bool isVisible)
	{
		bool wasVisible = mIsVisible;
		IComponent::setVisibility(isVisible);

		if (wasVisible && !mIsVisible) {
			for (auto& renderable : mRenderableTexts) {
				mLayer2D->removeRenderableText(renderable.get(), mZIndex);
			}
		}
		else if (!wasVisible && mIsVisible) {
			for (auto& renderable : mRenderableTexts) {
				mLayer2D->addRenderableText(renderable.get(), mZIndex);
			}
		}
	}


	void Label::setFont(FontRef font)
	{
		mFont = font;
		updateRenderableTexts();
	}


	void Label::setCharacterSize(const glm::vec2& size)
	{
		mCharacterSize = size;
		updateRenderableTexts();
	}


	void Label::setHorizontalAlignment(HorizontalAlignment alignment)
	{
		mHorizontalAlignment = alignment;
		updateRenderableTexts();
	}


	void Label::setVerticalAlignment(VerticalAlignment alignment)
	{
		mVerticalAlignment = alignment;
		updateRenderableTexts();
	}


	void Label::setColor(const glm::vec4& color)
	{
		mColor = color;
		for (auto& renderable : mRenderableTexts) {
			renderable->setColor(mColor);
		}
	}


	void Label::setText(const char* text)
	{
		mFullText = text;
		updateRenderableTexts();
	}


	void Label::onHover(const MouseMoveEvent& /*event*/) {}


	void Label::onClick(const MouseButtonEvent& /*event*/) {}


	void Label::onRelease(const MouseButtonEvent& /*event*/) {}

// Private functions
	void Label::updateRenderableTexts()
	{
		// Calculate the lines
		std::vector<std::string> lines;
		for (auto& str : utils::splitBy(mFullText, '\n')) {
			wordWrap(str, lines);
		}

		// Add more RenderableTexts
		while (mRenderableTexts.size() < lines.size()) {
			mRenderableTexts.emplace_back( std::make_unique<graphics::RenderableText>(mPosition, mCharacterSize) );
			if (mIsVisible) {
				mLayer2D->addRenderableText(mRenderableTexts.back().get(), mZIndex);
			}
		}

		// Remove unneeded RenderableTexts
		while (mRenderableTexts.size() > lines.size()) {
			if (mIsVisible) {
				mLayer2D->removeRenderableText(mRenderableTexts.back().get(), mZIndex);
			}
			mRenderableTexts.pop_back();
		}

		// Set the RenderableTexts properties
		for (std::size_t iLine = 0; iLine < lines.size(); ++iLine) {
			mRenderableTexts[iLine]->setPosition( calculateLinePosition(lines, iLine) );
			mRenderableTexts[iLine]->setSize(mCharacterSize);
			mRenderableTexts[iLine]->setFont(mFont);
			mRenderableTexts[iLine]->setColor(mColor);
			mRenderableTexts[iLine]->setText(lines[iLine]);
		}
	}


	void Label::wordWrap(std::string& input, std::vector<std::string>& output)
	{
		utils::trimRight(input);

		while (true) {
			// Check if there is space left for another line
			if ((output.size() + 1) * mCharacterSize.y >= mSize.y) {
				return;
			}

			utils::trimLeft(input);

			// Check if the input fits in a single line
			std::size_t iLastCharacter = 0;
			float textWidth = 0.0f;
			float xScale = mCharacterSize.x / mFont->maxCharacterSize.x;
			for (; iLastCharacter < input.length(); ++iLastCharacter) {
				auto itCharacter = mFont->characters.find(input[iLastCharacter]);
				if (itCharacter != mFont->characters.end()) {
					if (textWidth + xScale * itCharacter->second.advance < mSize.x) {
						textWidth += xScale * itCharacter->second.advance;
					}
					else {
						break;
					}
				}
			}

			if (iLastCharacter == input.length()) {
				output.push_back(input);
				return;
			}

			// Split by the last space
			std::size_t lineLength = iLastCharacter;
			auto itLastSpace = std::find_if(
				input.rbegin() + input.length() - iLastCharacter, input.rend(),
				[](char c) { return std::isspace(static_cast<int>(c)); }
			);
			if (itLastSpace != input.rend()) {
				lineLength = std::distance(input.begin(), itLastSpace.base());
			}

			std::string line = input.substr(0, lineLength);
			utils::trimRight(line);

			output.push_back(line);
			input = input.substr(lineLength);
		}
	}


	glm::vec2 Label::calculateLinePosition(const std::vector<std::string>& lines, std::size_t iLine) const
	{
		glm::vec2 position = mPosition;

		if (mHorizontalAlignment != HorizontalAlignment::Left) {
			float lineWidth = 0.0f;
			float xScale = mCharacterSize.x / mFont->maxCharacterSize.x;
			for (char c : lines[iLine]) {
				auto itCharacter = mFont->characters.find(c);
				if (itCharacter != mFont->characters.end()) {
					lineWidth += xScale * itCharacter->second.advance;
				}
			}

			if (mHorizontalAlignment == HorizontalAlignment::Right) {
				position.x += mSize.x - lineWidth;
			}
			else {
				position.x += (mSize.x - lineWidth) / 2.0f;
			}
		}

		position.y += iLine * mCharacterSize.y;
		if (mVerticalAlignment != VerticalAlignment::Top) {
			float lineBlockHeight = lines.size() * mCharacterSize.y;

			if (mVerticalAlignment == VerticalAlignment::Bottom) {
				position.y += mSize.y - lineBlockHeight;
			}
			else {
				position.y += (mSize.y - lineBlockHeight) / 2.0f;
			}
		}

		return position;
	}

}
