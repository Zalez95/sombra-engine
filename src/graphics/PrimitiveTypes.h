#ifndef PRIMITIVE_TYPES_H
#define PRIMITIVE_TYPES_H

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>

namespace graphics {

	/** Struct RGBColor, represents a Color with 3 components */
	struct RGBColor
	{
		/** The red value of the color in the range [0, 1] */
		GLfloat		mR;

		/** The green value of the color in the range [0, 1] */
		GLfloat		mG;

		/** The blue value of the color in the range [0, 1] */
		GLfloat		mB;

		/** Creates a new RGBColor */
		RGBColor() {};

		/** Creates a new RGBColor
		 * 
		 * @param	r the red value of the color in the range [0, 1]
		 * @param	g the green value of the color in the range [0, 1]
		 * @param	b the blue value of the color in the range [0, 1] */
		RGBColor(GLfloat r, GLfloat g, GLfloat b) : mR(r), mG(g), mB(b) {};

		/** Destructor */
		~RGBColor() {};
	};


	/** Struct RGBAColor, represents a Color with 4 components */
	struct RGBAColor
	{
		/** The red value of the color in the range [0, 1] */
		GLfloat		mR;

		/** The green value of the color in the range [0, 1] */
		GLfloat		mG;
		
		/** The blue value of the color in the range [0, 1] */
		GLfloat		mB;

		/** The alpha value of the color in the range [0, 1] */
		GLfloat		mA;

		/** Creates a new RGBAColor */
		RGBAColor() {};

		/** Creates a new RGBAColor
		 * 
		 * @param	r the red value of the color in the range [0, 1]
		 * @param	g the green value of the color in the range [0, 1]
		 * @param	b the blue value of the color in the range [0, 1]
		 * @param	a the alpha value of the color in the range [0, 1] */
		RGBAColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a) :
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
		GLfloat mWeight;
	};

}

#endif		// PRIMITIVE_TYPES_H
