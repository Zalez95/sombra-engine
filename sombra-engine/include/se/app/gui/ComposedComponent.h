#ifndef COMPOSED_COMPONENT_H
#define COMPOSED_COMPONENT_H

#include <vector>
#include "IComponent.h"

namespace se::app {

	/**
	 * Struct Anchor, it holds the location of an IComponent inside its parent
	 * ComposedComponent
	 */
	struct Anchor
	{
		/** The different points of an IComponent the can be sticked */
		enum class Origin
		{ TopLeft, TopRight, BottomLeft, BottomRight, Center };

		/** The origin point of the IComponent that will be sticked to its
		 * parent */
		Origin origin = Origin::Center;

		/** The relative position of the origin of the IComponent to its parent
		 * in the range [0, 1], being 0 the top/left and 1 the bottom/right
		 * in each axis respectively */
		glm::vec2 relativePosition = glm::vec2(0.5f);

		/** The offset of the origin of the IComponent to @see relativePosition
		 * in pixels */
		glm::vec2 offset = glm::vec2(0.0f);
	};


	/**
	 * Struct Proportions, it holds the size that an IComponent must have
	 * inside its parent ComposedComponent
	 */
	struct Proportions
	{
		/** The size of a IComponent relative to its parent */
		glm::vec2 relativeSize = glm::vec2(1.0f);

		/** If the aspect ratio must be preserved if the IComponent must be
		 * resized */
		bool preserveAspectRatio = false;

		/** The aspect ratio to preserve if @see preserveAspectRatio is true */
		float aspectRatio = 1.0f;

		/** The minimum size in pixels of the IComponent */
		glm::vec2 minimumSize = glm::vec2(0.0f);

		/** The maximum size in pixels of the IComponent */
		glm::vec2 maximumSize = glm::vec2(std::numeric_limits<float>::max());
	};


	/**
	 * Class ComposedComponent, it's a GUI element that can hold multiple
	 * child IComponents inside so they can be updated when the current one
	 * is updated
	 */
	class ComposedComponent : public IComponent
	{
	protected:	// Nested types
		/** Holds all the data of a child added to a ComposedComponent */
		struct ChildData {
			IComponent* child;			///< A pointer to the child IComponent
			Anchor anchor;				///< The anchor of the child
			Proportions proportions;	///< The Proportions of the child
		};

	protected:	// Attributes
		/** The children IComponents of the current one */
		std::vector<ChildData> mChildren;

	public:		// Functions
		/** Class destructor */
		virtual ~ComposedComponent() = default;

		/** Sets the position of the ComposedComponent
		 *
		 * @param	position the new Position of the top-left corner of the
		 *			ComposedComponent */
		virtual void setPosition(const glm::vec2& position) override;

		/** Sets the size of the ComposedComponent
		 *
		 * @param	size the new Size of the ComposedComponent */
		virtual void setSize(const glm::vec2& size) override;

		/** Sets the z-index of the ComposedComponent
		 *
		 * @param	zIndex the new z-index of the ComposedComponent */
		virtual void setZIndex(unsigned char zIndex) override;

		/** Sets the ComposedComponent visibility on/off
		 *
		 * @param	isVisible if ComposedComponent must be shown or not */
		virtual void setVisibility(bool isVisible) override;

		/** Handles a mouse pointer over the ComposedComponent
		 *
		 * @param	event the MouseMoveEvent that holds the location of the
		 *			mouse */
		virtual void onHover(const MouseMoveEvent& event) override;

		/** Handles a mouse click on the ComposedComponent
		 *
		 * @param	event the MouseButtonEvent that holds the state of the
		 *			button pressed */
		virtual void onClick(const MouseButtonEvent& event) override;

		/** Handles a mouse click release on the ComposedComponent
		 *
		 * @param	event the MouseButtonEvent that holds the state of the
		 *			button pressed */
		virtual void onRelease(const MouseButtonEvent& event) override;

		/** Adds the given IComponent as a child of the ComposedComponent
		 *
		 * @param	child a pointer to the new child IComponent
		 * @param	anchor the Anchor where the child IComponent will be
		 *			added so it can change it's position when the current one
		 *			does so
		 * @param	proportions the Proportions that the child IComponent must
		 *			have so it can be resized when the current one does so
		 * @note	the child IComponent will be added on top of the current
		 *			one (with the next z-index) */
		void add(
			IComponent* child,
			const Anchor& anchor, const Proportions& proportions
		);

		/** Removes the given IComponent from the ComposedComponent
		 *
		 * @param	child a pointer to the child IComponent to remove */
		void remove(IComponent* child);
	private:
		/** Calculates the position where the given child IComponent should be
		 * located
		 *
		 * @param	child the child IComponent of the current one
		 * @param	anchor the Anchor point of the child IComponent */
		glm::vec2 calculateChildPosition(
			const IComponent& child, const Anchor& anchor
		) const;

		/** Calculates the size of the given child IComponent
		 *
		 * @param	proportions the Proportions of the child IComponent */
		glm::vec2 calculateChildSize(const Proportions& proportions) const;
	};

}

#endif		// COMPOSED_COMPONENT_H
