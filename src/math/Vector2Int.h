#pragma once

#include <cmath>
#include <vector>

#include <glm/vec2.hpp>
#include "Vector2.h"
#include <string>

namespace gbe {
	struct Vector2Int : public glm::ivec2 {
		Vector2Int();
		Vector2Int(int, int);
		Vector2Int(glm::ivec2 glmvec);

		inline Vector2Int& operator+=(const Vector2Int& b) {
			this->x += b.x;
			this->y += b.y;

			return *this;
		}

		inline Vector2Int& operator-=(const Vector2Int& b) {
			this->x -= b.x;
			this->y -= b.y;

			return *this;
		}

		static const Vector2Int zero;

		std::string Hash() {
			return std::to_string(this->x) + "," + std::to_string(this->y);
		}
	};
}