#pragma once

#include <glm/mat3x3.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace gbe {
	struct Matrix3 : public glm::mat3x3 {
		inline Matrix3() : glm::mat3x3(1.0f)
		{

		}

		inline Matrix3(const glm::mat3x3& from) : glm::mat3x3(from)
		{

		}

		inline const float* Get_Ptr() {
			return &((*this)[0].x);
		}

		inline Matrix3 Inverted() {
			return glm::inverse(*this);
		}

		inline bool isfinite() const {
			for (int i = 0; i < 3; i++)
				for (int j = 0; j < 3; j++)
					if (!std::isfinite((*this)[i][j]))
						return false;
			return true;
		}

	};
}