#ifndef PRIMITIVE_TYPES_H
#define PRIMITIVE_TYPES_H

namespace fe { namespace graphics {

	/** The different render ways to render */
	enum RenderFlags : unsigned char
	{
		DEFAULT					= 1 << 0,
		WIREFRAME				= 1 << 1,
		DISABLE_DEPTH_TEST		= 1 << 2,
		DISABLE_FACE_CULLING	= 1 << 3
	};


	/** Struct Weight, each weight contains the index of a vertex in
	 * the mesh and the influence of a bone over the vertex */
	struct Weight
	{
		/** The Index of the current vertex inside a mesh */
		unsigned int index;

		/** The Weight of a Bone on the current vertex in the range [0,1].
		 * It measures the influence of a Bone on the current vertex */
		float weight;
	};

}}

#endif		// PRIMITIVE_TYPES_H
