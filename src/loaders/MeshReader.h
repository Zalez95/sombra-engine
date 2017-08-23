#ifndef MESH_READER_H
#define MESH_READER_H

#include "MeshLoader.h"

namespace utils { class FileReader; }

namespace loaders {

	/**
	 * Class MeshReader, it's used to create meshes from raw data or from the
	 * given files
	 */
	class MeshReader
	{
	private:	// Nested types
		typedef std::unique_ptr<graphics::Mesh> MeshUPtr;

		/** Struct FILE_FORMAT, it holds the name, version and other data of
		 * our Mesh file format */
		struct FILE_FORMAT
		{
			static const std::string	FILE_NAME;
			static const std::string	FILE_EXTENSION;
			static const unsigned int	VERSION = 1;
			static const unsigned int	REVISION = 3;
		};

	private:	// Attributes
		/** The MeshLoader used to load the meshes */
		MeshLoader& mMeshLoader;

	public:		// Functions
		/** Creates a new MeshReader
		 * 
		 * @param	meshLoader the MeshLoader used to load the meshes */
		MeshReader(MeshLoader& meshLoader) : mMeshLoader(meshLoader) {};

		/** Class destructor */
		~MeshReader() {};

		/** Parses the Meshes in the given file and returns them
		 * 
		 * @note	the cursor of the file reader will be moved after parsing
		 *			the file
		 * @param	fileReader the file reader with the Meshes that we want
		 *			to parse
		 * @return	a vector with the parsed Meshes */
		std::vector<MeshUPtr> load(utils::FileReader& fileReader) const;
		
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
		bool checkHeader(utils::FileReader& fileReader) const;

		/** Parses the Meshes in the given file and returns them
		 * 
		 * @param	fileReader the file reader with the meshes that we want to
		 *			parse
		 * @return	a vector with the parsed meshes */
		std::vector<MeshUPtr> parseMeshes(utils::FileReader& fileReader) const;

		/** Parses the Mesh at the current position of the given file and
		 * returns it
		 *
		 * @param	fileReader the file reader with the file that we want
		 *			to read
		 * @return	a pointer to the parsed Mesh */
		MeshUPtr parseMesh(utils::FileReader& fileReader) const;

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
		) const;
	};

}

#endif		// MESH_READER_H
