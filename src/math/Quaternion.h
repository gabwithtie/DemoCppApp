#pragma once

#include <string>
#include <cmath>
#include <vector>

#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Vector3.h"
#include "Matrix3.h"

namespace gbe {
	struct Quaternion : public glm::quat {
	private:
		inline Matrix3 normalize_matrix(Matrix3 frommat) {
			// Normalize the columns to remove any scaling
			frommat[0] = glm::normalize(frommat[0]);
			frommat[1] = glm::normalize(frommat[1]);
			frommat[2] = glm::normalize(frommat[2]);
			return frommat;
		}
	public:
		Quaternion();
		Quaternion(glm::quat glmquat);

		inline Quaternion(Matrix3 frommat) : glm::quat(normalize_matrix(frommat)){
			
		}

		inline Quaternion Inverted() {
			return glm::inverse(*this);
		}

		static Quaternion AngleAxis(Vector3, float deg_angle);
		static Quaternion LookAtRotation(Vector3 forward, Vector3 Up);
		static Quaternion Euler(Vector3);

		inline static Quaternion Lerp(Quaternion a, Quaternion b, float t) {
			return glm::lerp(a, b, t);
		}

		inline Matrix3 ToMatrix() const {
			return glm::toMat3(*this);
		}

		operator glm::quat() const;

		inline const Quaternion& operator *=(const Quaternion& other) {
			glm::quat::operator*=((glm::quat)other);

			return *this;
		}

		Vector3 ToEuler();
	};
}