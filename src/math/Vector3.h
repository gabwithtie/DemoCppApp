#pragma once

#include <string>
#include <cmath>
#include <vector>

#include <sstream>
#include <glm/vec3.hpp>
#include <glm/trigonometric.hpp>

namespace gbe{
	struct Vector3 : public glm::vec3 {
		Vector3();
		Vector3(float, float, float);
		Vector3(float);
		Vector3(const glm::vec3& glmvec);

		inline const static Vector3 Up() {
			return Vector3(0, 1, 0);
		}

		inline bool isfinite() {
			return std::isfinite(x) && std::isfinite(y) && std::isfinite(z);
		}

		float SqrMagnitude() const;
		Vector3 Cross(Vector3) const;
		Vector3 Normalize() const;
		float Dot(Vector3 b) const;
		float Magnitude() const;

		Vector3& operator+=(Vector3 const& v);
		Vector3& operator-=(Vector3 const& v);
		operator glm::vec3() const;

		const float* Get_Ptr();

		static const Vector3 zero;
		inline static Vector3 Lerp(const Vector3& a, const Vector3& b, float t) {
			auto d = b - a;
			d *= t;
			auto tv = a + d;
			return tv;
		}
		inline static const Vector3 RandomWithin(Vector3& a, Vector3& b) {
			gbe::Vector3 d = b - a;
			auto rand_01 = []() {
				return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
				};

			return a + gbe::Vector3(d.x * rand_01(), d.y * rand_01(), d.z * rand_01());
		}
		inline static const Vector3 Mid(const Vector3& a, const Vector3& b) {
			return Lerp(a, b, 0.5f);
		}
		inline std::string ToString() {
			std::stringstream sstream;
			sstream << this->x << "," << this->y << "," << this->z;

			return sstream.str();
		}
		static Vector3 GetClosestPointOnLineGivenLine(const Vector3& a, const Vector3& adir, const Vector3& b, const Vector3& bdir);

		inline static float AngleBetween(const Vector3& a, const Vector3& b) {
			// Calculate the dot product of the two vectors
			float dotProduct = a.Dot(b);

			// Calculate the product of their magnitudes
			float magnitudeProduct = a.Magnitude() * b.Magnitude();

			// Check for a zero magnitude to avoid division by zero
			if (magnitudeProduct == 0.0f) {
				return 0.0f;
			}

			// Clamp the value to the range [-1, 1] to prevent `acos` from returning
			// `NaN` due to floating point inaccuracies
			float cosTheta = dotProduct / magnitudeProduct;
			cosTheta = std::max(-1.0f, std::min(1.0f, cosTheta));

			// Return the angle in radians
			return glm::degrees(std::acos(cosTheta));
		}
	};
}