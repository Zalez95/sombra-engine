#ifndef PRIMITIVE_TYPES_H
#define PRIMITIVE_TYPES_H

#include <string>
#include <vector>
#include <glm/glm.hpp>

namespace graphics {

	/** Struct RGBColor, represents a Color with 3 components */
	struct RGBColor
	{
		/** The red value of the color in the range [0, 1] */
		float		r;

		/** The green value of the color in the range [0, 1] */
		float		g;

		/** The blue value of the color in the range [0, 1] */
		float		b;
	};


	/** Struct RGBAColor, represents a Color with 4 components */
	struct RGBAColor
	{
		/** The red value of the color in the range [0, 1] */
		float		r;

		/** The green value of the color in the range [0, 1] */
		float		g;
		
		/** The blue value of the color in the range [0, 1] */
		float		b;

		/** The alpha value of the color in the range [0, 1] */
		float		a;
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

}

#endif		// PRIMITIVE_TYPES_H
