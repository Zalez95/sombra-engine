#include "MeshReader.h"
#include <map>
#include <string>
#include <sstream>
#include <glm/glm.hpp>
#include "../utils/FileReader.h"
#include "../graphics/3D/Mesh.h"

namespace loaders {

// Static variables definition
	const std::string MeshReader::FILE_FORMAT::FILE_NAME		= "FAZE_MSH_FILE";
	const std::string MeshReader::FILE_FORMAT::FILE_EXTENSION	= ".fzmsh";

// Public Functions
	std::vector<MeshReader::MeshUPtr> MeshReader::load(utils::FileReader& fileReader) const
	{
		try {
			// 1. Get the input file
			if (fileReader.fail()) {
				throw std::runtime_error("Error reading the file\n");
			}

			// 2. Check the file header
			if (!checkHeader(fileReader)) {
				throw std::runtime_error("Error with the header of the file\n");
			}

			// 3. Parse the Meshes
			return parseMeshes(fileReader);
		}
		catch (const std::exception& e) {
			throw std::runtime_error("Error parsing the Mesh in the file \"" + fileReader.getFilePath() + "\":\n" + e.what());
		}
	}


	std::vector<GLfloat> MeshReader::calculateNormals(
		const std::vector<GLfloat>& positions,
		const std::vector<GLushort>& faceIndices
	) const
	{
		std::vector<GLfloat> normals(positions.size(), 0);

		// Sum to the normal of every vertex, the normal of the faces 
		// which it belongs
		for (size_t i = 0; i < faceIndices.size(); i+=3) {
			// Get the normal of triangle
			GLfloat v1_x = positions[3 * faceIndices[i]]		- positions[3 * faceIndices[i+1]];
			GLfloat v1_y = positions[3 * faceIndices[i] + 1]	- positions[3 * faceIndices[i+1] + 1];
			GLfloat v1_z = positions[3 * faceIndices[i] + 2]	- positions[3 * faceIndices[i+1] + 2];
			glm::vec3 v1(v1_x, v1_y, v1_z);

			GLfloat v2_x = positions[3 * faceIndices[i]]		- positions[3 * faceIndices[i+2]];
			GLfloat v2_y = positions[3 * faceIndices[i] + 1]	- positions[3 * faceIndices[i+2] + 1];
			GLfloat v2_z = positions[3 * faceIndices[i] + 2]	- positions[3 * faceIndices[i+2] + 2];
			glm::vec3 v2(v2_x, v2_y, v2_z);

			glm::vec3 normal = glm::cross(v1, v2);

			normals[3 * faceIndices[i]]			+= normal.x;
			normals[3 * faceIndices[i] + 1]		+= normal.y;
			normals[3 * faceIndices[i] + 2]		+= normal.z;

			normals[3 * faceIndices[i+1]]		+= normal.x;
			normals[3 * faceIndices[i+1] + 1]	+= normal.y;
			normals[3 * faceIndices[i+1] + 2]	+= normal.z;

			normals[3 * faceIndices[i+2]]		+= normal.x;
			normals[3 * faceIndices[i+2] + 1]	+= normal.y;
			normals[3 * faceIndices[i+2] + 2]	+= normal.z;
		}

		// Normalize the normal vector of every vertex
		for (size_t i = 0; i < normals.size(); i+=3) {
			GLfloat length	= sqrt( pow(normals[i], 2) + pow(normals[i+1], 2) + pow(normals[i+2], 2) );
			normals[i]		/= length;
			normals[i+1]	/= length;
			normals[i+2]	/= length;
		}

		return normals;
	}

// Private functions
	bool MeshReader::checkHeader(utils::FileReader& fileReader) const
	{
		const std::string FILE_VERSION = std::to_string(FILE_FORMAT::VERSION) + '.' + std::to_string(FILE_FORMAT::REVISION);
		bool ret = false;

		std::string fileName, fileVersion;
		fileReader >> fileName >> fileVersion;
		if (!fileReader.fail() &&
			fileName == FILE_FORMAT::FILE_NAME &&
			fileVersion == FILE_VERSION
		) {
			ret = true;
		}

		return ret;
	}

	
	std::vector<MeshReader::MeshUPtr> MeshReader::parseMeshes(utils::FileReader& fileReader) const
	{
		std::vector<MeshUPtr> meshes;
		unsigned int numMeshes = 0, meshIndex = 0;

		while (!fileReader.isEmpty()) {
			std::string token; fileReader >> token;

			if (token == "num_meshes") {
				fileReader >> numMeshes;
				if (!fileReader.fail()) {
					meshes.reserve(numMeshes);
				}
			}
			else if (token == "mesh") {
				auto curMesh = parseMesh(fileReader);
				if (meshIndex < numMeshes) {
					meshes.push_back(std::move(curMesh));
				}
				++meshIndex;
			}
			else {
				throw std::runtime_error("Error: unexpected word \"" + token + "\" at line " + std::to_string(fileReader.getNumLines()) + '\n');
			}
		}

		if (meshIndex != numMeshes) {
			throw std::runtime_error("Error: expected " + std::to_string(numMeshes) + " meshes, parsed " + std::to_string(meshIndex) + '\n');
		}

		return meshes;
	}


	MeshReader::MeshUPtr MeshReader::parseMesh(utils::FileReader& fileReader) const
	{
		std::string name;
		std::vector<GLfloat> positions, uvs;
		std::vector<GLushort> posIndices, uvIndices;
		unsigned int numPositions = 0, numUVs = 0, numFaces = 0, numJoints = 0;
		unsigned int positionIndex = 0, uvIndex = 0, faceIndex = 0;
		
		std::string trash;
		fileReader >> name >> trash;

		bool end = false;
		while (!end) {
			std::string token; fileReader >> token;

			if (token == "num_positions") {
				fileReader >> numPositions;
				if (!fileReader.fail()) {
					positions.resize(3 * numPositions);
				}
			}
			else if (token == "num_uvs") {
				fileReader >> numUVs;
				if (!fileReader.fail()) {
					uvs.resize(2 * numUVs);
				}
			}
			else if (token == "num_faces") {
				fileReader >> numFaces;
				if (!fileReader.fail()) {
					posIndices.resize(3 * numFaces);
					if (numUVs > 0) { uvIndices.resize(3 * numFaces); }
				}
			}
			else if (token == "num_joints") {
				fileReader >> numJoints;
			}
			else if (token == "v") {
				if (positionIndex < numPositions) {
					fileReader	>> positions[3 * positionIndex]
								>> positions[3 * positionIndex + 1]
								>> positions[3 * positionIndex + 2];
				}
				else { fileReader.discardLine(); }
				++positionIndex;
			}
			else if (token == "uv"){
				unsigned int vi;
				fileReader >> vi;
				if (!fileReader.fail() && vi < numPositions) {
					fileReader	>> uvs[2 * vi]
								>> uvs[2 * vi + 1];
				}
				else { fileReader.discardLine(); }
				++uvIndex;
			}
			else if (token == "f") {
				if (faceIndex < numFaces) {
					fileReader	>> trash >> posIndices[3 * faceIndex]
								>> posIndices[3 * faceIndex + 1]
								>> posIndices[3 * faceIndex + 2] >> trash;
					if (numUVs > 0) {
						fileReader	>> trash >> uvIndices[3 * faceIndex]
									>> uvIndices[3 * faceIndex + 1]
									>> uvIndices[3 * faceIndex + 2] >> trash;
					}
				}
				else { fileReader.discardLine(); }
				++faceIndex;
			}
			else if (token == "}") { end = true; }
			else {
				throw std::runtime_error("Error: unexpected word \"" + token + "\" at line "+ std::to_string(fileReader.getNumLines()) + '\n');
			}
		}

		if (positionIndex != numPositions) {
			throw std::runtime_error("Error: expected " + std::to_string(numPositions) + " positions, parsed " + std::to_string(positionIndex) + '\n');
		}
		if (uvIndex != numUVs) {
			throw std::runtime_error("Error: expected " + std::to_string(numUVs) + " UVs, parsed " + std::to_string(uvIndex) + '\n');
		}
		if (faceIndex != numFaces) {
			throw std::runtime_error("Error: expected " + std::to_string(numFaces) + " faces, parsed " + std::to_string(faceIndex) + '\n');
		}

		return processMeshData(name, positions, uvs, posIndices, uvIndices);
	}


	MeshReader::MeshUPtr MeshReader::processMeshData(
		const std::string& name,
		const std::vector<GLfloat>& positions,
		const std::vector<GLfloat>& uvs,
		const std::vector<GLushort>& posIndices,
		const std::vector<GLushort>& uvIndices
	) const {
		std::vector<GLfloat> positions2, uvs2, normals;
		std::vector<GLushort> faceIndices;

		if (!uvIndices.empty()) {
			faceIndices = std::vector<GLushort>(posIndices.size());
			std::map<std::pair<GLushort, GLushort>, GLushort> faceIndicesMap;

			for (size_t i = 0; i < faceIndices.size(); ++i) {
				GLushort positionIndex	= posIndices[i];
				GLushort uvIndex		= uvIndices[i];

				auto mapKey = std::make_pair(positionIndex, uvIndex);
				if (faceIndicesMap.find(mapKey) != faceIndicesMap.end()) {
					faceIndices[i] = faceIndicesMap[mapKey];
				}
				else {
					GLushort vertexIndex	= static_cast<GLushort>(positions2.size() / 3);

					positions2.push_back( positions[3 * positionIndex] );
					positions2.push_back( positions[3 * positionIndex + 1] );
					positions2.push_back( positions[3 * positionIndex + 2] );
					uvs2.push_back( uvs[2 * uvIndex] );
					uvs2.push_back( uvs[2 * uvIndex + 1] );
					faceIndicesMap[mapKey]	= vertexIndex;
					faceIndices[i]			= vertexIndex;
				}
			}
		}
		else {
			positions2	= positions;
			uvs2		= std::vector<GLfloat>(positions.size());
			faceIndices	= posIndices;
		}

		normals = calculateNormals(positions2, faceIndices);
		return mMeshLoader.createMesh(name, positions2, normals, uvs2, faceIndices);
	}

}
