#ifndef LABEL_H
#define LABEL_H

#include "../../graphics/2D/RenderableText.h"
#include "../Repository.h"
#include "IComponent.h"

namespace se::app {

	class GUIManager;


	/**
	 * Class Label, it's an IComponent used for drawing the texts of the GUI
	 */
	class Label : public IComponent
	{
	public:		// Nested types
		/** The horizontal alignment of the text lines inside the Label */
		enum class HorizontalAlignment { Left, Center, Right };

		/** The vertical alignment of the text lines inside the Label */
		enum class VerticalAlignment { Top, Center, Bottom };
	private:
		using TechniqueResource = Repository::ResourceRef<graphics::Technique>;
		using FontResource = Repository::ResourceRef<graphics::Font>;
		using RenderableTextUPtr = std::unique_ptr<graphics::RenderableText>;

	private:	// Attributes
		/** A pointer to the GUIManager used for drawing the Label */
		GUIManager* mGUIManager;

		/** The RenderableText used for drawing each line of the Label */
		std::vector<RenderableTextUPtr> mRenderableTexts;

		/** The technique of the Text */
		TechniqueResource mTechnique;

		/** The font of the Text */
		FontResource mFont;

		/** The maximum character size to use with the Text */
		glm::vec2 mCharacterSize;

		/** The horizontal alignment of the Text lines */
		HorizontalAlignment mHorizontalAlignment;

		/** The vertical alignment of the Text lines */
		VerticalAlignment mVerticalAlignment;

		/** The RGBA color of the Text */
		glm::vec4 mColor;

		/** The full text that should be written
		 * @note	if it's too large the printed one could be smaller */
		std::string mFullText;

	public:		// Functions
		/** Creates a new Label
		 *
		 * @param	guiManager a pointer to the GUIManager used for drawing
		 *			the Label */
		Label(GUIManager* guiManager);
		Label(const Label& other);
		Label(Label&& other) = default;

		/** Class destructor */
		virtual ~Label();

		/** Assignment operator */
		Label& operator=(const Label& other);
		Label& operator=(Label&& other) = default;

		/** Sets the position of the Label
		 *
		 * @param	position the new Position of the top-left corner of the
		 *			Label */
		virtual void setPosition(const glm::vec2& position) override;

		/** Sets the size of the Label
		 *
		 * @param	size the new Size of the Label */
		virtual void setSize(const glm::vec2& size) override;

		/** Sets the z-index of the Label
		 *
		 * @param	zIndex the new z-index of the Label */
		virtual void setZIndex(unsigned char zIndex) override;

		/** Sets the Label visibility on/off
		 *
		 * @param	isVisible if Label must be shown or not */
		virtual void setVisibility(bool isVisible) override;

		/** Sets the font of the Label
		 *
		 * @param	font the Font of the Label */
		void setFont(const FontResource& font);

		/** Sets the size of each character in the Label text
		 *
		 * @param	size the new size in pixels of the characters of the text */
		void setCharacterSize(const glm::vec2& size);

		/** Sets the horizontal alignment of the text lines of the Label
		 *
		 * @param	alignment the new horizontal alignment of the text */
		void setHorizontalAlignment(HorizontalAlignment alignment);

		/** Sets the vertical alignment of the text lines of the Label
		 *
		 * @param	alignment the new vertical alignment of the text */
		void setVerticalAlignment(VerticalAlignment alignment);

		/** Sets the color of the Label
		 *
		 * @param	color the new RGBA color of the Label */
		void setColor(const glm::vec4& color);

		/** Sets the text of the Label
		 *
		 * @param	text the new text of the Label */
		void setText(const char* text);

		/** Handles a mouse pointer over the Label
		 *
		 * @param	event the MouseMoveEvent that holds the location of the
		 *			mouse */
		virtual void onHover(const MouseMoveEvent& event) override;

		/** Handles a mouse click on the Label
		 *
		 * @param	event the MouseButtonEvent that holds the state of the
		 *			button pressed */
		virtual void onClick(const MouseButtonEvent& event) override;

		/** Handles a mouse click release on the Label
		 *
		 * @param	event the MouseButtonEvent that holds the state of the
		 *			button pressed */
		virtual void onRelease(const MouseButtonEvent& event) override;
	private:
		/** Updates the RenderableTexts with the current properties of
		 * the Label */
		void updateRenderableTexts();

		/** Splits the given input text into multiple lines limited by the
		 * Label size
		 *
		 * @param	input a reference to the input text without break lines
		 *			(it will be modified)
		 * @param	output the vector where the lines will be appended
		 * @note	the lines in the output vector will be taken in count for
		 *			calculating the size left */
		void wordWrap(std::string& input, std::vector<std::string>& output);

		/** Calculates the 2D position of the given line based on the Label
		 * position, alignment and its line number
		 *
		 * @param	lines the vector of text lines to print
		 * @param	iLine the index of the text line in @see lines to calculate
		 *			its position
		 * @return	a vector with the 2D position in pixels of the line */
		glm::vec2 calculateLinePosition(
			const std::vector<std::string>& lines,
			std::size_t iLine
		) const;
	};

}

#endif		// LABEL_H
