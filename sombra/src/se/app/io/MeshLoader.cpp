#include <array>
#include <glm/gtc/constants.hpp>
#include "se/app/io/MeshLoader.h"
#include "se/graphics/core/VertexBuffer.h"
#include "se/graphics/core/IndexBuffer.h"
#include "se/graphics/core/VertexArray.h"
#include "se/collision/HalfEdgeMeshExt.h"
#include "se/utils/FixedVector.h"

namespace se::app {

	static void createInternalRingsMesh(RawMesh&, std::size_t, std::size_t, float, glm::vec2);


	graphics::Mesh MeshLoader::createGraphicsMesh(const RawMesh& rawMesh)
	{
		using namespace graphics;

		VertexArray vao;
		vao.bind();

		std::vector<VertexBuffer> vbos;
		if (!rawMesh.positions.empty()) {
			auto& vbo = vbos.emplace_back();
			vbo.resizeAndCopy(rawMesh.positions.data(), rawMesh.positions.size());

			vbo.bind();
			vao.setVertexAttribute(MeshAttributes::PositionAttribute, TypeId::Float, false, 3, 0);
		}
		if (!rawMesh.normals.empty()) {
			auto& vbo = vbos.emplace_back();
			vbo.resizeAndCopy(rawMesh.normals.data(), rawMesh.normals.size());

			vbo.bind();
			vao.setVertexAttribute(MeshAttributes::NormalAttribute, TypeId::Float, false, 3, 0);
		}
		if (!rawMesh.tangents.empty()) {
			auto& vbo = vbos.emplace_back();
			vbo.resizeAndCopy(rawMesh.tangents.data(), rawMesh.tangents.size());

			vbo.bind();
			vao.setVertexAttribute(MeshAttributes::TangentAttribute, TypeId::Float, false, 3, 0);
		}
		if (!rawMesh.texCoords.empty()) {
			auto& vbo = vbos.emplace_back();
			vbo.resizeAndCopy(rawMesh.texCoords.data(), rawMesh.texCoords.size());

			vbo.bind();
			vao.setVertexAttribute(MeshAttributes::TexCoordAttribute0, TypeId::Float, false, 2, 0);
		}
		if (!rawMesh.jointIndices.empty()) {
			auto& vbo = vbos.emplace_back();
			vbo.resizeAndCopy(rawMesh.jointIndices.data(), rawMesh.jointIndices.size());

			vbo.bind();
			vao.setVertexIntegerAttribute(MeshAttributes::JointIndexAttribute, TypeId::UnsignedShort, 4, 0);
		}
		if (!rawMesh.jointWeights.empty()) {
			auto& vbo = vbos.emplace_back();
			vbo.resizeAndCopy(rawMesh.jointWeights.data(), rawMesh.jointWeights.size());

			vbo.bind();
			vao.setVertexAttribute(MeshAttributes::JointWeightAttribute, TypeId::Float, false, 4, 0);
		}

		IndexBuffer ibo;
		ibo.resizeAndCopy(rawMesh.faceIndices.data(), TypeId::UnsignedShort, rawMesh.faceIndices.size());

		ibo.bind();

		auto [minimum, maximum] = calculateBounds(rawMesh);

		Mesh ret(std::move(vbos), std::move(ibo), std::move(vao));
		ret.setBounds(minimum, maximum);
		return ret;
	}


	std::pair<glm::vec3, glm::vec3> MeshLoader::calculateBounds(
		const RawMesh& rawMesh
	) {
		glm::vec3 minimum(0.0f), maximum(0.0f);

		if (!rawMesh.positions.empty()) {
			minimum = maximum = rawMesh.positions[0];
			for (std::size_t i = 1; i < rawMesh.positions.size(); ++i) {
				minimum = glm::min(minimum, rawMesh.positions[i]);
				maximum = glm::max(maximum, rawMesh.positions[i]);
			}
		}

		return { minimum, maximum };
	}


	std::pair<collision::HalfEdgeMesh, bool> MeshLoader::createHalfEdgeMesh(const RawMesh& rawMesh)
	{
		collision::HalfEdgeMesh heMesh;

		// Add the HEVertices
		std::vector<int> heVertexIndices;
		heVertexIndices.reserve(rawMesh.positions.size());
		for (const glm::vec3& position : rawMesh.positions) {
			heVertexIndices.push_back( collision::addVertex(heMesh, position) );
		}

		// Add the HEFaces
		bool allFacesLoaded = true;
		for (std::size_t i = 0; i < rawMesh.faceIndices.size(); i += 3) {
			std::array<int, 3> vertexIndices = {
				heVertexIndices[ rawMesh.faceIndices[i] ],
				heVertexIndices[ rawMesh.faceIndices[i+1] ],
				heVertexIndices[ rawMesh.faceIndices[i+2] ]
			};
			if (collision::addFace(heMesh, vertexIndices.begin(), vertexIndices.end()) < 0) {
				allFacesLoaded = false;
			}
		}

		// Validate the HEMesh
		return std::pair(heMesh, allFacesLoaded && collision::validateMesh(heMesh).first);
	}


	std::pair<RawMesh, bool> MeshLoader::createRawMesh(
		const collision::HalfEdgeMesh& heMesh,
		const utils::PackedVector<glm::vec3>& normals
	) {
		if (heMesh.faces.size() != normals.size()) { return std::pair(RawMesh(), false); }

		RawMesh rawMesh("heMeshTriangles");

		// The faces must be triangles
		collision::HalfEdgeMesh heMeshTriangles = collision::triangulateFaces(heMesh);

		rawMesh.positions.reserve(heMeshTriangles.vertices.size());
		rawMesh.normals.reserve(heMeshTriangles.vertices.size());
		rawMesh.faceIndices.reserve(3 * heMeshTriangles.faces.size());

		std::unordered_map<std::size_t, std::size_t> vertexMap;
		for (auto itVertex = heMeshTriangles.vertices.begin(); itVertex != heMeshTriangles.vertices.end(); ++itVertex) {
			glm::vec3 normal = collision::calculateVertexNormal(heMesh, normals, itVertex.getIndex());

			glm::vec3 c1 = glm::cross(normal, glm::vec3(0.0f, 0.0f, 1.0f));
			glm::vec3 c2 = glm::cross(normal, glm::vec3(0.0f, 1.0f, 0.0f));
			glm::vec3 tangent = (glm::dot(c1, c1) > glm::dot(c2, c2))? c1 : c2;

			rawMesh.positions.push_back(itVertex->location);
			rawMesh.normals.push_back(normal);
			rawMesh.tangents.push_back(tangent);
			vertexMap.emplace(itVertex.getIndex(), rawMesh.positions.size() - 1);
		}

		for (auto itFace = heMeshTriangles.faces.begin(); itFace != heMeshTriangles.faces.end(); ++itFace) {
			utils::FixedVector<int, 3> faceIndices;
			collision::getFaceIndices(heMeshTriangles, itFace.getIndex(), std::back_inserter(faceIndices));
			for (int iVertex : faceIndices) {
				rawMesh.faceIndices.push_back(static_cast<unsigned short>(vertexMap[iVertex]));
			}
		}

		return std::pair(rawMesh, true);
	}


	RawMesh MeshLoader::createBoxMesh(const std::string& name, glm::vec3 lengths)
	{
		se::app::RawMesh ret(name);
		glm::vec3 halfL = 0.5f * lengths;

		ret.positions = {
			glm::vec3{ halfL.x, halfL.y,-halfL.z},		glm::vec3{ halfL.x,-halfL.y,-halfL.z},
			glm::vec3{-halfL.x,-halfL.y,-halfL.z},		glm::vec3{-halfL.x, halfL.y,-halfL.z},
			glm::vec3{ halfL.x, halfL.y, halfL.z},		glm::vec3{ halfL.x,-halfL.y, halfL.z},
			glm::vec3{-halfL.x,-halfL.y, halfL.z},		glm::vec3{-halfL.x, halfL.y, halfL.z},
			glm::vec3{ halfL.x, halfL.y,-halfL.z},		glm::vec3{ halfL.x,-halfL.y,-halfL.z},
			glm::vec3{ halfL.x, halfL.y, halfL.z},		glm::vec3{ halfL.x,-halfL.y, halfL.z},
			glm::vec3{ halfL.x, halfL.y, halfL.z},		glm::vec3{ halfL.x,-halfL.y, halfL.z},
			glm::vec3{-halfL.x,-halfL.y, halfL.z},		glm::vec3{-halfL.x, halfL.y, halfL.z},
			glm::vec3{ halfL.x, halfL.y,-halfL.z},		glm::vec3{ halfL.x,-halfL.y,-halfL.z},
			glm::vec3{-halfL.x, halfL.y,-halfL.z},		glm::vec3{-halfL.x, halfL.y,-halfL.z},
			glm::vec3{-halfL.x,-halfL.y,-halfL.z},		glm::vec3{-halfL.x,-halfL.y,-halfL.z},
			glm::vec3{-halfL.x,-halfL.y, halfL.z},		glm::vec3{-halfL.x, halfL.y, halfL.z}
		};
		ret.texCoords = {
			{0.666467010f, 0.666466951f},	{0.999800264f, 0.000199760f},
			{0.333533257f, 0.333133578f},	{0.333533287f, 0.666466951f},
			{0.666467010f, 0.333533167f},	{0.999800145f, 0.333133548f},
			{0.333533197f, 0.000199760f},	{0.333533197f, 0.333533257f},
			{0.333133667f, 0.333533167f},	{0.000199899f, 0.333533197f},
			{0.333133548f, 0.666466951f},	{0.000199760f, 0.666466951f},
			{0.333133697f, 0.333133548f},	{0.333133488f, 0.000199760f},
			{0.000199760f, 0.000199909f},	{0.000199869f, 0.333133667f},
			{0.333133548f, 0.999800264f},	{0.000199760f, 0.999800264f},
			{0.333133548f, 0.666866540f},	{0.666467010f, 0.333133488f},
			{0.000199770f, 0.666866540f},	{0.666866540f, 0.000199799f},
			{0.666866540f, 0.333133578f},	{0.666466891f, 0.000199760f}
		};
		ret.faceIndices = {
			16, 20, 18,		5, 21, 1,
			2, 23, 19,		0, 7, 4,
			10, 9, 8,		15, 13, 12,
			16, 17, 20,		5, 22, 21,
			2, 6, 23,		0, 3, 7,
			10, 11, 9,		15, 14, 13
		};

		return ret;
	}


	RawMesh MeshLoader::createSphereMesh(
		const std::string& name,
		std::size_t segments, std::size_t rings, float radius
	) {
		RawMesh rawMesh(name);
		rawMesh.positions.reserve((rings - 1) * segments + 2);
		rawMesh.faceIndices.reserve(6 * (rings - 2) * segments + 3 * 2 * segments);

		// Creates the bottom skullcap
		rawMesh.positions.push_back({ 0.0f, -radius, 0.0f });
		for (std::size_t j = 0; j < segments; ++j) {
			rawMesh.faceIndices.push_back(0);
			rawMesh.faceIndices.push_back(j + 1);
			rawMesh.faceIndices.push_back((j + 1 < segments)? j + 2 : 1);
		}

		// Creates the internal rings
		float ringAngle = glm::pi<float>() / rings;
		createInternalRingsMesh(rawMesh, segments, rings-2, radius, { ringAngle - glm::half_pi<float>(), glm::half_pi<float>() - ringAngle });

		// Creates the top skullcap
		rawMesh.positions.push_back({ 0.0f, radius, 0.0f });
		for (std::size_t j = 0; j < segments; ++j) {
			rawMesh.faceIndices.push_back(rawMesh.positions.size() - 2 - segments + j);
			rawMesh.faceIndices.push_back(rawMesh.positions.size() - 1);
			rawMesh.faceIndices.push_back((j + 1 < segments)? rawMesh.positions.size() - 1 - segments + j : rawMesh.positions.size() - 2 - segments);
		}

		return rawMesh;
	}


	RawMesh MeshLoader::createDomeMesh(
		const std::string& name,
		std::size_t segments, std::size_t rings, float radius
	) {
		RawMesh rawMesh(name);
		rawMesh.positions.reserve(rings * segments + 1);
		rawMesh.faceIndices.reserve(6 * (rings - 2) * segments + 3 * 2 * segments);

		// Creates the internal rings
		float ringAngle = glm::pi<float>() / rings;
		createInternalRingsMesh(rawMesh, segments, rings-1, radius, { 0, glm::half_pi<float>() - ringAngle });

		// Creates the top skullcap
		rawMesh.positions.push_back({ 0.0f, radius, 0.0f });
		for (std::size_t j = 0; j < segments; ++j) {
			rawMesh.faceIndices.push_back(rawMesh.positions.size() - 2 - segments + j);
			rawMesh.faceIndices.push_back(rawMesh.positions.size() - 1);
			rawMesh.faceIndices.push_back((j + 1 < segments)? rawMesh.positions.size() - 1 - segments + j : rawMesh.positions.size() - 2 - segments);
		}

		return rawMesh;
	}


	std::vector<glm::vec3> MeshLoader::calculateNormals(
		const std::vector<glm::vec3>& positions,
		const std::vector<unsigned short>& faceIndices
	) {
		std::vector<glm::vec3> normals(positions.size(), glm::vec3(0.0f));

		// The normal vector of every vertex is calculated as the sum of the
		// normal vectors of the faces it belongs to
		for (std::size_t i = 0; i < faceIndices.size(); i+=3) {
			// Get the normal of triangle
			const glm::vec3& v1 = positions[faceIndices[i+1]] - positions[faceIndices[i]];
			const glm::vec3& v2 = positions[faceIndices[i+2]] - positions[faceIndices[i]];
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


	std::vector<glm::vec3> MeshLoader::calculateTangents(
		const std::vector<glm::vec3>& positions,
		const std::vector<glm::vec2>& texCoords,
		const std::vector<unsigned short>& faceIndices
	) {
		std::vector<glm::vec3> tangents(positions.size(), glm::vec3(0.0f));

		// The tangent vector of every vertex is calculated as the sum of the
		// tangent vectors of the faces it belongs to
		for (std::size_t i = 0; i < faceIndices.size(); i+=3) {
			const glm::vec3& e1		= positions[faceIndices[i+1]] - positions[faceIndices[i]];
			const glm::vec3& e2		= positions[faceIndices[i+2]] - positions[faceIndices[i]];
			const glm::vec2& dUV1	= texCoords[faceIndices[i+1]] - texCoords[faceIndices[i]];
			const glm::vec2& dUV2	= texCoords[faceIndices[i+2]] - texCoords[faceIndices[i]];

			glm::vec3 tangent(0.0f);
			float invertedDeterminant = 1.0f / (dUV1.x * dUV2.y - dUV2.x * dUV1.y);
			tangent.x = invertedDeterminant * (dUV2.y * e1.x - dUV1.y * e2.x);
			tangent.y = invertedDeterminant * (dUV2.y * e1.y - dUV1.y * e2.y);
			tangent.z = invertedDeterminant * (dUV2.y * e1.z - dUV1.y * e2.z);
			tangents[faceIndices[i]]	+= tangent;
			tangents[faceIndices[i+1]]	+= tangent;
			tangents[faceIndices[i+2]]	+= tangent;
		}

		// Normalize the tangent vector of every vertex
		for (glm::vec3& tangent : tangents) {
			tangent = glm::normalize(tangent);
		}

		return tangents;
	}


	void createInternalRingsMesh(
		RawMesh& rawMesh,
		std::size_t segments, std::size_t rings, float radius,
		glm::vec2 latitude
	) {
		std::size_t currentRingIndex = rawMesh.positions.size();

		// Creates the vertices
		float segmentAngle = glm::two_pi<float>() / segments;
		float ringAngle = (latitude[1] - latitude[0]) / rings;

		for (std::size_t i = 0; i < rings + 1; ++i) {
			float currentRingLatitude = i * ringAngle + latitude[0];
			float currentRingRadius = radius * glm::cos(currentRingLatitude);

			float y = radius * glm::sin(currentRingLatitude);
			for (std::size_t j = 0; j < segments; ++j) {
				float currentSegmentLongitude = j * segmentAngle - glm::pi<float>();
				float x = currentRingRadius * glm::cos(currentSegmentLongitude);
				float z = currentRingRadius * glm::sin(currentSegmentLongitude);
				rawMesh.positions.push_back({ x, y, z });
			}
		}

		// Creates the face indices
		for (std::size_t i = 0; i < rings; ++i) {
			std::size_t previousRingIndex = currentRingIndex;
			currentRingIndex += segments;

			for (std::size_t j = 0; j < segments; ++j) {
				rawMesh.faceIndices.push_back(previousRingIndex + j);
				rawMesh.faceIndices.push_back(currentRingIndex + j);
				rawMesh.faceIndices.push_back((j + 1 < segments)? currentRingIndex + j + 1 : currentRingIndex);
				rawMesh.faceIndices.push_back(previousRingIndex + j);
				rawMesh.faceIndices.push_back((j + 1 < segments)? currentRingIndex + j + 1 : currentRingIndex);
				rawMesh.faceIndices.push_back((j + 1 < segments)? previousRingIndex + j + 1 : previousRingIndex);
			}
		}
	}

}
