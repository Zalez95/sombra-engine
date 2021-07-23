#include <array>
#include <glm/gtc/constants.hpp>
#include "se/app/io/MeshLoader.h"
#include "se/graphics/core/VertexBuffer.h"
#include "se/graphics/core/IndexBuffer.h"
#include "se/graphics/core/VertexArray.h"
#include "se/collision/HalfEdgeMeshExt.h"
#include "se/utils/FixedVector.h"

namespace se::app {

	static void createInternalRingsMesh(
		RawMesh& rawMesh,
		std::size_t segments, std::size_t rings, float radius, const glm::vec2& latitude,
		float uvScale = 1.0f, const glm::vec2& uvOffset = glm::vec2(0.0f)
	);


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
			vao.enableAttribute(MeshAttributes::PositionAttribute);
			vao.setVertexAttribute(MeshAttributes::PositionAttribute, TypeId::Float, false, 3, 0);
		}
		if (!rawMesh.normals.empty()) {
			auto& vbo = vbos.emplace_back();
			vbo.resizeAndCopy(rawMesh.normals.data(), rawMesh.normals.size());

			vbo.bind();
			vao.enableAttribute(MeshAttributes::NormalAttribute);
			vao.setVertexAttribute(MeshAttributes::NormalAttribute, TypeId::Float, false, 3, 0);
		}
		if (!rawMesh.tangents.empty()) {
			auto& vbo = vbos.emplace_back();
			vbo.resizeAndCopy(rawMesh.tangents.data(), rawMesh.tangents.size());

			vbo.bind();
			vao.enableAttribute(MeshAttributes::TangentAttribute);
			vao.setVertexAttribute(MeshAttributes::TangentAttribute, TypeId::Float, false, 3, 0);
		}
		if (!rawMesh.texCoords.empty()) {
			auto& vbo = vbos.emplace_back();
			vbo.resizeAndCopy(rawMesh.texCoords.data(), rawMesh.texCoords.size());

			vbo.bind();
			vao.enableAttribute(MeshAttributes::TexCoordAttribute0);
			vao.setVertexAttribute(MeshAttributes::TexCoordAttribute0, TypeId::Float, false, 2, 0);
		}
		if (!rawMesh.jointIndices.empty()) {
			auto& vbo = vbos.emplace_back();
			vbo.resizeAndCopy(rawMesh.jointIndices.data(), rawMesh.jointIndices.size());

			vbo.bind();
			vao.enableAttribute(MeshAttributes::JointIndexAttribute);
			vao.setVertexIntegerAttribute(MeshAttributes::JointIndexAttribute, TypeId::UnsignedShort, 4, 0);
		}
		if (!rawMesh.jointWeights.empty()) {
			auto& vbo = vbos.emplace_back();
			vbo.resizeAndCopy(rawMesh.jointWeights.data(), rawMesh.jointWeights.size());

			vbo.bind();
			vao.enableAttribute(MeshAttributes::JointWeightAttribute);
			vao.setVertexAttribute(MeshAttributes::JointWeightAttribute, TypeId::Float, false, 4, 0);
		}

		IndexBuffer ibo;
		ibo.resizeAndCopy(rawMesh.indices.data(), TypeId::UnsignedShort, rawMesh.indices.size());

		ibo.bind();

		auto [minimum, maximum] = calculateBounds(rawMesh);

		Mesh ret(std::move(vbos), std::move(ibo), std::move(vao));
		ret.setBounds(minimum, maximum);
		return ret;
	}


	RawMesh MeshLoader::createRawMesh(const graphics::Mesh& gMesh)
	{
		RawMesh ret;

		for (const auto& vbo : gMesh.getVBOs()) {
			if (gMesh.getVAO().isAttributeEnabled(MeshAttributes::PositionAttribute)
				&& gMesh.getVAO().checkVertexAttributeVBOBound(MeshAttributes::PositionAttribute, vbo)
			) {
				ret.positions.resize(vbo.size() / sizeof(glm::vec3));
				vbo.read(ret.positions.data(), ret.positions.size());
			}
			else if (gMesh.getVAO().isAttributeEnabled(MeshAttributes::NormalAttribute)
				&& gMesh.getVAO().checkVertexAttributeVBOBound(MeshAttributes::NormalAttribute, vbo)
			) {
				ret.normals.resize(vbo.size() / sizeof(glm::vec3));
				vbo.read(ret.normals.data(), ret.normals.size());
			}
			else if (gMesh.getVAO().isAttributeEnabled(MeshAttributes::TangentAttribute)
				&& gMesh.getVAO().checkVertexAttributeVBOBound(MeshAttributes::TangentAttribute, vbo)
			) {
				ret.tangents.resize(vbo.size() / sizeof(glm::vec3));
				vbo.read(ret.tangents.data(), ret.tangents.size());
			}
			else if (gMesh.getVAO().isAttributeEnabled(MeshAttributes::TexCoordAttribute0)
				&& gMesh.getVAO().checkVertexAttributeVBOBound(MeshAttributes::TexCoordAttribute0, vbo)
			) {
				ret.texCoords.resize(vbo.size() / sizeof(glm::vec2));
				vbo.read(ret.texCoords.data(), ret.texCoords.size());
			}
			else if (gMesh.getVAO().isAttributeEnabled(MeshAttributes::JointIndexAttribute)
				&& gMesh.getVAO().checkVertexAttributeVBOBound(MeshAttributes::JointIndexAttribute, vbo)
			) {
				ret.jointIndices.resize(vbo.size() / sizeof(glm::u16vec4));
				vbo.read(ret.jointIndices.data(), ret.jointIndices.size());
			}
			else if (gMesh.getVAO().isAttributeEnabled(MeshAttributes::JointWeightAttribute)
				&& gMesh.getVAO().checkVertexAttributeVBOBound(MeshAttributes::JointWeightAttribute, vbo)
			) {
				ret.jointWeights.resize(vbo.size() / sizeof(glm::vec4));
				vbo.read(ret.jointWeights.data(), ret.jointWeights.size());
			}
		}

		if (gMesh.getIBO().getIndexType() == graphics::TypeId::UnsignedShort) {
			ret.indices.resize(gMesh.getIBO().getIndexCount());
			gMesh.getIBO().read(ret.indices.data(), ret.indices.size());
		}

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
		for (std::size_t i = 0; i < rawMesh.indices.size(); i += 3) {
			std::array<int, 3> vertexIndices = {
				heVertexIndices[ rawMesh.indices[i] ],
				heVertexIndices[ rawMesh.indices[i+1] ],
				heVertexIndices[ rawMesh.indices[i+2] ]
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
		rawMesh.indices.reserve(3 * heMeshTriangles.faces.size());

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
				rawMesh.indices.push_back(static_cast<unsigned short>(vertexMap[iVertex]));
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
		ret.indices = {
			16, 20, 18,		5, 21, 1,
			2, 23, 19,		0, 7, 4,
			10, 9, 8,		15, 13, 12,
			16, 17, 20,		5, 22, 21,
			2, 6, 23,		0, 3, 7,
			10, 11, 9,		15, 14, 13
		};

		return ret;
	}


	RawMesh MeshLoader::createGridMesh(const std::string& name, std::size_t numSquares, float length)
	{
		se::app::RawMesh ret(name);

		float halfL = 0.5f * length;
		float stepL = length / numSquares;

		ret.positions.reserve((numSquares + 1) * (numSquares + 1));
		ret.indices.reserve(6 * numSquares * numSquares);
		for (unsigned short i = 0; i < static_cast<unsigned short>(numSquares + 1); ++i) {
			for (unsigned short j = 0; j < static_cast<unsigned short>(numSquares + 1); ++j) {
				ret.positions.emplace_back(j * stepL - halfL, 0.0f, i * stepL - halfL);

				if ((i > 0) && (j > 0)) {
					unsigned short topLeft = (i - 1) * (static_cast<unsigned short>(numSquares) + 1) + j - 1;
					unsigned short topRight = (i - 1) * (static_cast<unsigned short>(numSquares) + 1) + j;
					unsigned short bottomLeft = i * (static_cast<unsigned short>(numSquares) + 1) + j - 1;
					unsigned short bottomRight = i * (static_cast<unsigned short>(numSquares) + 1) + j;
					ret.indices.insert(ret.indices.end(), {
						topLeft, bottomLeft, bottomLeft, bottomRight, bottomRight, topRight, topRight, topLeft
					});
				}
			}
		}

		return ret;
	}


	RawMesh MeshLoader::createSphereMesh(
		const std::string& name,
		std::size_t segments, std::size_t rings, float radius
	) {
		RawMesh rawMesh(name);
		rawMesh.positions.reserve(2 * ((rings / 2 + 1) * segments + 1));
		rawMesh.texCoords.reserve(2 * ((rings / 2 + 1) * segments + 1));
		rawMesh.indices.reserve(2 * (6 * (rings / 2) * segments + 3 * segments));

		// Creates the bottom skullcap
		rawMesh.positions.push_back({ 0.0f, -radius, 0.0f });
		rawMesh.texCoords.push_back({ 0.25f, 0.25f });
		for (std::size_t j = 0; j < segments; ++j) {
			rawMesh.indices.push_back(0);
			rawMesh.indices.push_back(static_cast<unsigned short>(j + 1));
			rawMesh.indices.push_back(static_cast<unsigned short>((j + 1) % segments + 1));
		}

		// Creates the internal rings
		float ringAngle = glm::half_pi<float>() / (rings / 2 + 1);
		createInternalRingsMesh(rawMesh, segments, rings/2, radius, { ringAngle - glm::half_pi<float>(), 0.0f }, 0.5f);
		createInternalRingsMesh(rawMesh, segments, rings/2, radius, { 0.0f, glm::half_pi<float>() - ringAngle }, 0.5f, glm::vec2(0.5f, 0.0f));

		// Creates the top skullcap
		rawMesh.positions.push_back({ 0.0f, radius, 0.0f });
		rawMesh.texCoords.push_back({ 0.75f, 0.25f });
		for (std::size_t j = 0; j < segments; ++j) {
			rawMesh.indices.push_back(static_cast<unsigned short>(1 + (2 * (rings / 2) + 1) * segments + j));
			rawMesh.indices.push_back(static_cast<unsigned short>(rawMesh.positions.size() - 1));
			rawMesh.indices.push_back(static_cast<unsigned short>(1 + (2 * (rings / 2) + 1) * segments + (j + 1) % segments));
		}

		return rawMesh;
	}


	RawMesh MeshLoader::createDomeMesh(
		const std::string& name,
		std::size_t segments, std::size_t rings, float radius
	) {
		RawMesh rawMesh(name);
		rawMesh.positions.reserve((rings + 1) * segments + 1);
		rawMesh.texCoords.reserve((rings + 1) * segments + 1);
		rawMesh.indices.reserve(6 * rings * segments + 3 * segments);

		// Creates the internal rings
		float ringAngle = glm::half_pi<float>() / (rings + 1);
		createInternalRingsMesh(rawMesh, segments, rings, radius, { 0.0f, glm::half_pi<float>() - ringAngle });

		// Creates the top skullcap
		rawMesh.positions.push_back({ 0.0f, radius, 0.0f });
		rawMesh.texCoords.push_back({ 0.5f, 0.5f });
		for (std::size_t j = 0; j < segments; ++j) {
			rawMesh.indices.push_back(static_cast<unsigned short>(rings * segments + j));
			rawMesh.indices.push_back(static_cast<unsigned short>(rawMesh.positions.size() - 1));
			rawMesh.indices.push_back(static_cast<unsigned short>(rings * segments + (j + 1) % segments));
		}

		return rawMesh;
	}


	RawMesh MeshLoader::createConeMesh(
		const std::string& name,
		std::size_t segments, float radius, float height
	) {
		RawMesh rawMesh(name);
		rawMesh.positions.reserve(2 * segments + 1);
		rawMesh.texCoords.reserve(2 * segments + 1);
		rawMesh.indices.reserve(6 * (segments - 1));

		float segmentAngle = glm::two_pi<float>() / segments;
		float halfHeight = 0.5f * height;
		for (std::size_t i = 0; i < 2 * segments; ++i) {
			float currentSegmenAngle = i * segmentAngle - glm::pi<float>();
			float c = glm::cos(currentSegmenAngle);
			float s = glm::sin(currentSegmenAngle);
			rawMesh.positions.push_back({ radius * c, -halfHeight, radius * s });
			rawMesh.texCoords.push_back({ 0.25f * c + 0.25f + 0.5f * (i / segments), 0.25f * s + 0.25f });
		}
		rawMesh.positions.push_back({ 0.0f, halfHeight, 0.0f });
		rawMesh.texCoords.push_back({ 0.25f, 0.25f });

		for (std::size_t i = 0; i < segments; ++i) {
			rawMesh.indices.push_back(static_cast<unsigned short>(i));
			rawMesh.indices.push_back(static_cast<unsigned short>(2 * segments));
			rawMesh.indices.push_back(static_cast<unsigned short>((i + 1) % segments));
		}
		for (std::size_t i = 1; i < segments - 1; ++i) {
			rawMesh.indices.push_back(static_cast<unsigned short>(segments));
			rawMesh.indices.push_back(static_cast<unsigned short>(segments + i));
			rawMesh.indices.push_back(static_cast<unsigned short>(segments + (i + 1) % segments));
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


	std::vector<float> MeshLoader::calculateHeights(
		unsigned char* data, std::size_t xSize, std::size_t zSize
	) {
		std::vector<float> heights;
		heights.reserve(xSize * zSize);
		for (std::size_t z = 0; z < zSize; ++z) {
			for (std::size_t x = 0; x < xSize; ++x) {
				unsigned char h = data[z * xSize + x];
				heights.push_back(h / static_cast<float>(255) - 0.5f);
			}
		}
		return heights;
	}


	void createInternalRingsMesh(
		RawMesh& rawMesh,
		std::size_t segments, std::size_t rings, float radius, const glm::vec2& latitude,
		float uvScale, const glm::vec2& uvOffset
	) {
		unsigned short currentRingIndex = static_cast<unsigned short>(rawMesh.positions.size());

		// Creates the vertices
		float segmentAngle = glm::two_pi<float>() / segments;
		float ringAngle = (latitude[1] - latitude[0]) / rings;
		float halfUVScale = 0.5f * uvScale;

		for (std::size_t i = 0; i < rings + 1; ++i) {
			float currentRingLatitude = i * ringAngle + latitude[0];
			float ringScale = glm::cos(currentRingLatitude);
			float currentRingRadius = radius * ringScale;

			float y = radius * glm::sin(currentRingLatitude);
			for (std::size_t j = 0; j < segments; ++j) {
				float currentSegmentLongitude = j * segmentAngle - glm::pi<float>();
				float c = glm::cos(currentSegmentLongitude);
				float s = glm::sin(currentSegmentLongitude);
				rawMesh.positions.push_back({ currentRingRadius * c, y, currentRingRadius * s });
				rawMesh.texCoords.push_back({
					halfUVScale * ringScale * c + halfUVScale + uvOffset.x,
					halfUVScale * ringScale * s + halfUVScale + uvOffset.y
				});
			}
		}

		// Creates the face indices
		for (std::size_t i = 0; i < rings; ++i) {
			unsigned short previousRingIndex = currentRingIndex;
			currentRingIndex += static_cast<unsigned short>(segments);

			for (unsigned short j = 0; j < static_cast<unsigned short>(segments); ++j) {
				unsigned short nextJ = j + 1;
				rawMesh.indices.push_back(previousRingIndex + j);
				rawMesh.indices.push_back(currentRingIndex + j);
				rawMesh.indices.push_back((nextJ < segments)? currentRingIndex + nextJ : currentRingIndex);
				rawMesh.indices.push_back(previousRingIndex + j);
				rawMesh.indices.push_back((nextJ < segments)? currentRingIndex + nextJ : currentRingIndex);
				rawMesh.indices.push_back((nextJ < segments)? previousRingIndex + nextJ : previousRingIndex);
			}
		}
	}

}
