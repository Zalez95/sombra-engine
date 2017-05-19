#ifndef MESH_LOADER_H
#define MESH_LOADER_H

#include <memory>
#include <vector>
#include <GL/glew.h>
class FileReader;

namespace graphics {

	class Mesh;


	/**
	 * Class MeshLoader, it's used to create meshes from raw data or from the
	 * given files
	 */
	class MeshLoader
	{
	private:	// Nested types
		/** Struct FILE_FORMAT, it holds the name, version and other data of
		 * our Mesh file format */
		struct FILE_FORMAT
		{
			static const std::string	FILE_NAME;
			static const std::string	FILE_EXTENSION;
			static const unsigned int	VERSION = 1;
			static const unsigned int	REVISION = 3;
		};

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

		/** Parses the Meshes in the given file and returns them
		 * 
		 * @note	the cursor of the file reader will be moved after parsing
		 *			the file
		 * @param	fileReader the file reader with the Meshes that we want
		 *			to parse
		 * @return	a vector with the parsed Meshes */
		std::vector<MeshUPtr> load(FileReader* fileReader);
		
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
	private:
		/** Checks the header of the given file
		 *
		 * @param	fileReader the reader of the file with the header we want
		 *			to check
		 * @return	true if the file and version is ok, false otherwise */
		bool checkHeader(FileReader* fileReader);

		/** Parses the Meshes in the given file and returns them
		 * 
		 * @param	fileReader the file reader with the meshes that we want to
		 *			parse
		 * @return	a vector with the parsed meshes */
		std::vector<MeshUPtr> parseMeshes(FileReader* fileReader);

		/** Parses the Mesh at the current position of the given file and
		 * returns it
		 *
		 * @param	fileReader the file reader with the file that we want
		 *			to read
		 * @return	a pointer to the parsed Mesh */
		MeshUPtr parseMesh(FileReader* fileReader);

		/** With the positions, uvs and the vectors of indices given
		 * calculates the vertices and its indices for each face of the mesh
		 * 
		 * @param	name the name of the new mesh
		 * @param	positions the positions of the readed vertices
		 * @param	uvs the UV positions of the readed vertices
		 * @param	posIndices the indices to the positions of the vertices
		 *			of each face
		 * @param	posIndices the indices to the UV positions of the vertices
		 *			of each face */
		MeshUPtr processMeshData(
			const std::string& name,
			const std::vector<GLfloat>& positions,
			const std::vector<GLfloat>& uvs,
			const std::vector<GLushort>& posIndices,
			const std::vector<GLushort>& uvIndices
		);
	};

}

#endif		// MESH_LOADER_H
