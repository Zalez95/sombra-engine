#ifndef MESH_READER_H
#define MESH_READER_H

#include <memory>
#include "RawMesh.h"

namespace fe { namespace utils { class FileReader; }}

namespace fe { namespace loaders {

	/**
	 * Class MeshReader, it's used to create RawMeshes from raw data or from
	 * the given files
	 */
	class MeshReader
	{
	private:	// Nested types
		typedef std::unique_ptr<RawMesh> RawMeshUPtr;

		/** Struct FileFormat, it holds the name, version and other data of
		 * our Mesh file format */
		struct FileFormat
		{
			static const std::string sFileName;
			static const std::string sFileExtension;
			static const int sVersion = 1;
			static const int sRevision = 3;
		};

	public:		// Functions
		/** Creates a new MeshReader */
		MeshReader() {};

		/** Class destructor */
		~MeshReader() {};

		/** Parses the Meshes in the given file and returns them
		 *
		 * @note	the cursor of the file reader will be moved after parsing
		 *			the file
		 * @param	fileReader the file reader with the Meshes that we want
		 *			to parse
		 * @return	a vector with the parsed RawMeshes */
		std::vector<RawMeshUPtr> read(utils::FileReader& fileReader) const;

		/** Calculates the Normals of the given vertices
		 *
		 * @param	positions a vector with the positions of the vertices
		 * @param	faceIndices a vector with indices of the vertices that
		 *			compose the faces of a Mesh
		 * @return	a vector with the normals of the vertices
		 * @throw	runtime_error in case of any error while parsing */
		std::vector<glm::vec3> calculateNormals(
			const std::vector<glm::vec3>& positions,
			const std::vector<unsigned short>& faceIndices
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
		 * @return	a vector with the parsed RawMeshes
		 * @throw	runtime_error in case of any error while parsing */
		std::vector<RawMeshUPtr> parseRawMeshes(utils::FileReader& fileReader) const;

		/** Parses the Mesh at the current position of the given file and
		 * returns it
		 *
		 * @param	fileReader the file reader with the file that we want
		 *			to read
		 * @return	a pointer to the parsed RawMesh
		 * @throw	runtime_error in case of an unexpected text */
		RawMeshUPtr parseRawMesh(utils::FileReader& fileReader) const;

		/** With the positions, uvs and the vectors of indices given
		 * calculates the vertices and its indices for each face of the mesh
		 *
		 * @param	name the name of the new mesh
		 * @param	positions the positions of the vertices
		 * @param	uvs the UV positions of the vertices
		 * @param	posIndices the indices to the positions of the vertices
		 *			of each face
		 * @param	posIndices the indices to the UV positions of the vertices
		 *			of each face
		 * @return	a pointer to the new RawMesh */
		RawMeshUPtr processRawMeshData(
			const std::string& name,
			const std::vector<glm::vec3>& positions,
			const std::vector<glm::vec2>& uvs,
			const std::vector<unsigned short>& posIndices,
			const std::vector<unsigned short>& uvIndices
		) const;
	};

}}

#endif		// MESH_READER_H
