#ifndef PRIMITIVE_TYPES_H
#define PRIMITIVE_TYPES_H

namespace fe { namespace graphics {

	/** The different render ways to render */
	enum RenderFlags : unsigned char
	{
		DEFAULT				= 1 << 0,
		WIREFRAME			= 1 << 1,
		DISABLE_DEPTH_TEST	= 1 << 2
	};


	/** Struct RGBColor, represents a Color with 3 components */
	struct RGBColor
	{
		/** The red value of the color in the range [0, 1] */
		float mR;

		/** The green value of the color in the range [0, 1] */
		float mG;

		/** The blue value of the color in the range [0, 1] */
		float mB;

		/** Creates a new RGBColor */
		RGBColor() {};

		/** Creates a new RGBColor
		 *
		 * @param	r the red value of the color in the range [0, 1]
		 * @param	g the green value of the color in the range [0, 1]
		 * @param	b the blue value of the color in the range [0, 1] */
		RGBColor(float r, float g, float b) : mR(r), mG(g), mB(b) {};

		/** Destructor */
		~RGBColor() {};
	};


	/** Struct RGBAColor, represents a Color with 4 components */
	struct RGBAColor
	{
		/** The red value of the color in the range [0, 1] */
		float mR;

		/** The green value of the color in the range [0, 1] */
		float mG;

		/** The blue value of the color in the range [0, 1] */
		float mB;

		/** The alpha value of the color in the range [0, 1] */
		float mA;

		/** Creates a new RGBAColor */
		RGBAColor() {};

		/** Creates a new RGBAColor
		 *
		 * @param	r the red value of the color in the range [0, 1]
		 * @param	g the green value of the color in the range [0, 1]
		 * @param	b the blue value of the color in the range [0, 1]
		 * @param	a the alpha value of the color in the range [0, 1] */
		RGBAColor(float r, float g, float b, float a) :
			mR(r), mG(g), mB(b), mA(a) {};

		/** Destructor */
		~RGBAColor() {};
	};


	/** Struct Weight, each weight contains the index of a vertex in
	 * the mesh and the influence of a bone over the vertex */
	struct Weight
	{
		/** The Index of the current vertex inside a mesh */
		unsigned int mIndex;

		/** The Weight of a Bone on the current vertex in the range [0,1].
		 * It measures the influence of a Bone on the current vertex */
		float mWeight;
	};

}}

#endif		// PRIMITIVE_TYPES_H
