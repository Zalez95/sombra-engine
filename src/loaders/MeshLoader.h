#ifndef MESH_LOADER_H
#define MESH_LOADER_H

#include <memory>
#include <vector>
#include <GL/glew.h>
#include "../graphics/3D/AABB.h"

namespace graphics {

	class Mesh;


	/**
	 * Class MeshLoader, it's used to create meshes from raw data
	 */
	class MeshLoader
	{
	private:	// Nested types
		/** The attribute indices of the Meshes */
		enum ATTRIBUTES
		{
			POSITION_ATTRIBUTE,
			NORMAL_ATTRIBUTE,
			UV_ATTRIBUTE,
			JOINT_WEIGHT_ATTRIBUTE,
			JOINT_INDEX_ATTRIBUTE
		};

		typedef std::unique_ptr<Mesh> MeshUPtr;

	public:		// Functions
		/** Creates a new MeshLoader */
		MeshLoader() {};

		/** Class destructor */
		~MeshLoader() {};
		
		/** creates a Mesh with the given mesh data
		 *
		 * @param	name the name of the Mesh
		 * @param	positions the coordinates of the vertices of the Mesh
		 * @param	normals the normals of the vertices of the Mesh
		 * @param	uvs the texture coordinates of the vertices
		 * @param	faceIndices the indices of the vertices that form the
		 *			faces of the Mesh
		 * @return	a pointer to the new created Mesh */
		MeshUPtr createMesh(
			const std::string& name,
			const std::vector<GLfloat>& positions,
			const std::vector<GLfloat>& normals,
			const std::vector<GLfloat>& uvs,
			const std::vector<GLushort>& faceIndices
		);

		/** creates a Mesh with the given mesh data
		 *
		 * @param	name the name of the Mesh
		 * @param	positions the coordinates of the vertices of the Mesh
		 * @param	normals the normals of the vertices of the Mesh
		 * @param	uvs the texture coordinates of the vertices
		 * @param	faceIndices the indices of the vertices that form the
		 *			faces of the Mesh
		 * @param	jointIndices the indices of the joints
		 * @param	weights the weights of the joint in each vertex
		 * @return	a pointer to the new created Mesh
		 * @note	one vertex can be influenced by a maximum of 4 joints */
		MeshUPtr createMesh(
			const std::string& name,
			const std::vector<GLfloat>& positions,
			const std::vector<GLfloat>& normals,
			const std::vector<GLfloat>& uvs,
			const std::vector<GLfloat>& jointWeights,
			const std::vector<GLushort>& jointIndices,
			const std::vector<GLushort>& faceIndices
		);
		
		/** Calculates the Normals of the given vertices
		 * 
		 * @param	positions a vector with the positions of the vertices
		 * @param	faceIndices a vector with indices of the vertices that
		 *			compose the faces of a Mesh
		 * @return	a vector with the normals of the vertices */
		std::vector<GLfloat> calculateNormals(
			const std::vector<GLfloat>& positions,
			const std::vector<GLushort>& faceIndices
		) const;
	};

}

#endif		// MESH_LOADER_H
