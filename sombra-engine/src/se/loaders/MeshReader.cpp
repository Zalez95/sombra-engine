#include <map>
#include "se/loaders/MeshReader.h"
#include "se/utils/FileReader.h"

namespace se::loaders {

// Static variables definition
	const std::string MeshReader::FileFormat::sFileName			= "SOMBRA_MSH_FILE";
	const std::string MeshReader::FileFormat::sFileExtension	= ".semsh";

// Public Functions
	std::vector<MeshReader::RawMeshUPtr> MeshReader::read(utils::FileReader& fileReader) const
	{
		try {
			// 1. Get the input file
			if (fileReader.getState() != utils::FileState::OK) {
				throw std::runtime_error("Error reading the file");
			}

			// 2. Check the file header
			if (!checkHeader(fileReader)) {
				throw std::runtime_error("Error with the header of the file");
			}

			// 3. Parse the RawMeshes
			return parseRawMeshes(fileReader);
		}
		catch (const std::exception& e) {
			throw std::runtime_error("Error parsing the Mesh in the file \"" + fileReader.getFilePath() + "\":\n" + e.what());
		}
	}


	std::vector<glm::vec3> MeshReader::calculateNormals(
		const std::vector<glm::vec3>& positions,
		const std::vector<unsigned short>& faceIndices
	) const
	{
		std::vector<glm::vec3> normals(positions.size(), glm::vec3(0.0f));

		// Sum to the normal of every vertex, the normal of the faces
		// which it belongs
		for (std::size_t i = 0; i < faceIndices.size(); i+=3) {
			// Get the normal of triangle
			const glm::vec3& v1 = positions[faceIndices[i]] - positions[faceIndices[i+1]];
			const glm::vec3& v2 = positions[faceIndices[i]] - positions[faceIndices[i+2]];
			glm::vec3 normal = glm::cross(v1, v2);

			normals[faceIndices[i]]		+= normal;
			normals[faceIndices[i+1]]	+= normal;
			normals[faceIndices[i+2]]	+= normal;
		}

		// Normalize the normal vector of every vertex
		for (glm::vec3& normal : normals) {
			normal = glm::normalize(normal);
		}

		return normals;
	}

// Private functions
	bool MeshReader::checkHeader(utils::FileReader& fileReader) const
	{
		const std::string curFileVersion = std::to_string(FileFormat::sVersion) + '.' + std::to_string(FileFormat::sRevision);
		bool ret = false;

		std::string fileName, fileVersion;
		fileReader >> fileName >> fileVersion;
		if ((fileReader.getState() == utils::FileState::OK)
			&& (fileName == FileFormat::sFileName)
			&& (fileVersion == curFileVersion)
		) {
			ret = true;
		}

		return ret;
	}


	std::vector<MeshReader::RawMeshUPtr> MeshReader::parseRawMeshes(utils::FileReader& fileReader) const
	{
		std::vector<RawMeshUPtr> rawMeshes;
		unsigned int nRawMeshes = 0, iRawMesh = 0;

		std::string token;
		while (fileReader.getValue(token) == utils::FileState::OK) {
			if (token == "num_meshes") {
				if (fileReader.getValue(nRawMeshes) == utils::FileState::OK) {
					rawMeshes.reserve(nRawMeshes);
				}
			}
			else if (token == "mesh") {
				auto curRawMesh = parseRawMesh(fileReader);
				if (iRawMesh < nRawMeshes) {
					rawMeshes.push_back(std::move(curRawMesh));
				}
				++iRawMesh;
			}
			else {
				throw std::runtime_error("Error: unexpected word \"" + token + "\" at line " + std::to_string(fileReader.getNumLines()));
			}
		}

		if (iRawMesh != nRawMeshes) {
			throw std::runtime_error("Error: expected " + std::to_string(nRawMeshes) + " meshes, parsed " + std::to_string(iRawMesh));
		}

		return rawMeshes;
	}


	MeshReader::RawMeshUPtr MeshReader::parseRawMesh(utils::FileReader& fileReader) const
	{
		std::string name;
		std::vector<glm::vec3> positions;
		std::vector<glm::vec2> uvs;
		std::vector<unsigned short> posIndices, uvIndices;
		unsigned int numPositions = 0, numUVs = 0, numFaces = 0, numJoints = 0;
		unsigned int positionIndex = 0, uvIndex = 0, faceIndex = 0;

		std::string trash;
		fileReader >> name >> trash;

		bool end = false;
		while (!end) {
			std::string token; fileReader >> token;

			if (token == "num_positions") {
				if (fileReader.getValue(numPositions) == utils::FileState::OK) {
					positions.resize(numPositions);
				}
			}
			else if (token == "num_uvs") {
				if (fileReader.getValue(numUVs) == utils::FileState::OK) {
					uvs.resize(numUVs);
				}
			}
			else if (token == "num_faces") {
				if (fileReader.getValue(numFaces) == utils::FileState::OK) {
					posIndices.resize(3 * numFaces);
					if (numUVs > 0) { uvIndices.resize(3 * numFaces); }
				}
			}
			else if (token == "num_joints") {
				fileReader >> numJoints;
			}
			else if (token == "v") {
				if (positionIndex < numPositions) {
					fileReader	>> positions[positionIndex].x
								>> positions[positionIndex].y
								>> positions[positionIndex].z;
				}
				else { fileReader.discardLine(); }
				++positionIndex;
			}
			else if (token == "uv") {
				unsigned int vi;
				if ((fileReader.getValue(vi) == utils::FileState::OK)
					&& (vi < numPositions)
				) {
					fileReader	>> uvs[vi].x
								>> uvs[vi].y;
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
						fileReader	>> trash >> uvIndices[faceIndex]
									>> uvIndices[faceIndex]
									>> uvIndices[faceIndex] >> trash;
					}
				}
				else { fileReader.discardLine(); }
				++faceIndex;
			}
			else if (token == "}") { end = true; }
			else {
				throw std::runtime_error("Error: unexpected word \"" + token + "\" at line "+ std::to_string(fileReader.getNumLines()));
			}
		}

		if (positionIndex != numPositions) {
			throw std::runtime_error("Error: expected " + std::to_string(numPositions) + " positions, parsed " + std::to_string(positionIndex));
		}
		if (uvIndex != numUVs) {
			throw std::runtime_error("Error: expected " + std::to_string(numUVs) + " UVs, parsed " + std::to_string(uvIndex));
		}
		if (faceIndex != numFaces) {
			throw std::runtime_error("Error: expected " + std::to_string(numFaces) + " faces, parsed " + std::to_string(faceIndex));
		}

		return processRawMeshData(name, positions, uvs, posIndices, uvIndices);
	}


	MeshReader::RawMeshUPtr MeshReader::processRawMeshData(
		const std::string& name,
		const std::vector<glm::vec3>& positions,
		const std::vector<glm::vec2>& uvs,
		const std::vector<unsigned short>& posIndices,
		const std::vector<unsigned short>& uvIndices
	) const {
		auto rawMesh = std::make_unique<RawMesh>(name);

		if (!uvIndices.empty()) {
			rawMesh->faceIndices = std::vector<unsigned short>(posIndices.size());
			std::map<std::pair<unsigned short, unsigned short>, unsigned short> faceIndicesMap;

			for (std::size_t i = 0; i < rawMesh->faceIndices.size(); ++i) {
				auto mapKey = std::make_pair(posIndices[i], uvIndices[i]);
				if (faceIndicesMap.find(mapKey) != faceIndicesMap.end()) {
					rawMesh->faceIndices[i] = faceIndicesMap[mapKey];
				}
				else {
					unsigned short vertexIndex	= static_cast<unsigned short>(rawMesh->positions.size());

					rawMesh->positions.push_back( positions[posIndices[i]] );
					rawMesh->uvs.push_back( uvs[uvIndices[i]] );
					rawMesh->faceIndices[i]	= vertexIndex;
					faceIndicesMap[mapKey]		= vertexIndex;
				}
			}
		}
		else {
			rawMesh->positions		= positions;
			rawMesh->faceIndices	= posIndices;
		}

		rawMesh->normals = calculateNormals(rawMesh->positions, rawMesh->faceIndices);
		return rawMesh;
	}

}
