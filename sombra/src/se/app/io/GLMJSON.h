#ifndef GLM_JSON_H
#define GLM_JSON_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <nlohmann/json.hpp>

namespace se::app {

	inline nlohmann::json toJson(const glm::vec2& v)
	{
		return { v.x, v.y };
	}


	inline bool toVec2(const nlohmann::json& json, glm::vec2& v)
	{
		if (json.is_array() && (json.size() >= 2)) {
			v = { json[0], json[1] };
			return true;
		}
		return false;
	}


	inline nlohmann::json toJson(const glm::vec3& v)
	{
		return { v.x, v.y, v.z };
	}


	inline bool toVec3(const nlohmann::json& json, glm::vec3& v)
	{
		if (json.is_array() && (json.size() >= 3)) {
			v = { json[0], json[1], json[2] };
			return true;
		}
		return false;
	}


	inline nlohmann::json toJson(const glm::vec4& v)
	{
		return { v.x, v.y, v.z, v.w };
	}


	inline bool toVec4(const nlohmann::json& json, glm::vec4& v)
	{
		if (json.is_array() && (json.size() >= 4)) {
			v = { json[0], json[1], json[2], json[3] };
			return true;
		}
		return false;
	}


	inline nlohmann::json toJson(const glm::quat& q)
	{
		return { q.x, q.y, q.z, q.w };
	}


	inline bool toQuat(const nlohmann::json& json, glm::quat& q)
	{
		if (json.is_array() && (json.size() >= 4)) {
			std::array<float, 4> fArray = json;
			q = *reinterpret_cast<glm::quat*>(fArray.data());
			return true;
		}
		return false;
	}


	inline nlohmann::json toJson(const glm::mat2& m)
	{
		return {
			m[0][0], m[0][1],
			m[1][0], m[1][1]
		};
	}


	inline bool toMat2(const nlohmann::json& json, glm::mat2& m)
	{
		if (json.is_array() && (json.size() >= 4)) {
			std::array<float, 4> fArray = json;
			m = *reinterpret_cast<glm::mat2*>(fArray.data());
			return true;
		}
		return false;
	}


	inline nlohmann::json toJson(const glm::mat3& m)
	{
		return {
			m[0][0], m[0][1], m[0][2],
			m[1][0], m[1][1], m[1][2],
			m[2][0], m[2][1], m[2][2]
		};
	}


	inline bool toMat3(const nlohmann::json& json, glm::mat3& m)
	{
		if (json.is_array() && (json.size() >= 9)) {
			std::array<float, 9> fArray = json;
			m = *reinterpret_cast<glm::mat3*>(fArray.data());
			return true;
		}
		return false;
	}


	inline nlohmann::json toJson(const glm::mat4& m)
	{
		return {
			m[0][0], m[0][1], m[0][2], m[0][3],
			m[1][0], m[1][1], m[1][2], m[1][3],
			m[2][0], m[2][1], m[2][2], m[2][3],
			m[3][0], m[3][1], m[3][2], m[3][3]
		};
	}


	inline bool toMat4(const nlohmann::json& json, glm::mat4& m)
	{
		if (json.is_array() && (json.size() >= 16)) {
			std::array<float, 16> fArray = json;
			m = *reinterpret_cast<glm::mat4*>(fArray.data());
			return true;
		}
		return false;
	}

}

#endif		// GLM_JSON_H
