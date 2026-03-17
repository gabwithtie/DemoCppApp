#pragma once

#include <vector>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Vector3.h"
#include "Vector4.h"

namespace gbe {
	struct Matrix4 : public glm::mat4x4 {
		Matrix4();
		Matrix4(const glm::mat4x4& from);

		inline Matrix4 Inverted() {
			return glm::inverse(*this);
		}

		inline Matrix4(float model_mat[16]) : Matrix4(glm::make_mat4(model_mat)) {

		}

		inline bool isfinite() const {
			for (int i = 0; i < 4; i++)
				for (int j = 0; j < 4; j++)
					if (!std::isfinite((*this)[i][j]))
						return false;
			return true;
		}

        inline static std::vector<Vector4> get_frustrum_corners(const glm::mat4& proj, const glm::mat4& view) {
            const auto inv = glm::inverse(proj * view);

            std::vector<Vector4> frustumCorners;
            for (unsigned int x = 0; x < 2; ++x)
            {
                for (unsigned int y = 0; y < 2; ++y)
                {
                    for (unsigned int z = 0; z < 2; ++z)
                    {
                        const glm::vec4 pt =
                            inv * glm::vec4(
                                2.0f * (float)x - 1.0f,
                                2.0f * (float)y - 1.0f,
                                z,
                                1.0f);
                        frustumCorners.push_back(pt / pt.w);
                    }
                }
            }
            return frustumCorners;
        }

        inline static Vector3 get_frustrum_center(std::vector<Vector4>& corners) {
            glm::vec3 center = glm::vec3(0, 0, 0);
            for (const auto& v : corners)
            {
                center += glm::vec3(v);
            }
            center /= corners.size();

            return center;
        }

		const float* Get_Ptr();
	};
}