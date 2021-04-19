#ifndef GLM_JSON_H
#define GLM_JSON_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <nlohmann/json.hpp>

namespace se::app {

	template <int l, typename T, glm::qualifier Q>
	inline nlohmann::json toJson(const glm::vec<l, T , Q>& v)
	{
		return *reinterpret_cast<const std::array<T, l>*>( glm::value_ptr(v) );
	}

	template <int l, typename T, glm::qualifier Q>
	inline bool toVec(const nlohmann::json& json, glm::vec<l, T, Q>& v)
	{
		if (json.is_array() && (json.size() >= l)) {
			std::array<T, l> vArray = json;
			v = *reinterpret_cast<glm::vec<l, T, Q>*>(vArray.data());
			return true;
		}
		return false;
	}


	template <typename T, glm::qualifier Q>
	inline nlohmann::json toJson(const glm::qua<T, Q>& q)
	{
		return { q.x, q.y, q.z, q.w };
	}

	template <typename T, glm::qualifier Q>
	inline bool toQuat(const nlohmann::json& json, glm::qua<T, Q>& q)
	{
		if (json.is_array() && (json.size() >= 4)) {
			std::array<T, 4> qArray = json;
			q = *reinterpret_cast<glm::quat*>(qArray.data());
			return true;
		}
		return false;
	}


	template <int c, int r, typename T, glm::qualifier Q>
	inline nlohmann::json toJson(const glm::mat<c, r, T , Q>& m)
	{
		return *reinterpret_cast<const std::array<T, c * r>*>( glm::value_ptr(m) );
	}

	template <int c, int r, typename T, glm::qualifier Q>
	inline bool toMat(const nlohmann::json& json, glm::mat<c, r, T, Q>& m)
	{
		if (json.is_array() && (json.size() >= c * r)) {
			std::array<T, c * r> mArray = json;
			m = *reinterpret_cast<glm::mat<c, r, T, Q>*>(mArray.data());
			return true;
		}
		return false;
	}

}

#endif		// GLM_JSON_H
