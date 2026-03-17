#include "Vector3.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/closest_point.hpp>

gbe::Vector3::Vector3() : glm::vec3(0)
{
}

gbe::Vector3::Vector3(float x, float y, float z) : glm::vec3(x, y, z)
{

}

gbe::Vector3::Vector3(float u) : glm::vec3(u)
{

}

gbe::Vector3::Vector3(const glm::vec3& glmvec) : glm::vec3(glmvec)
{
	
}

float gbe::Vector3::SqrMagnitude() const {
	return (this->x * this->x) + (this->y * this->y) + (this->z * this->z);
}

gbe::Vector3 gbe::Vector3::Cross(gbe::Vector3 b) const {
	return Vector3((this->y * b.z) - (this->z * b.y), (this->x * b.z) - (this->z * b.x), (this->x * b.y) - (this->y * b.x));
}

gbe::Vector3 gbe::Vector3::Normalize() const {
	return (glm::vec3)*this * (1.0f / this->Magnitude());
}

float gbe::Vector3::Dot(Vector3 b) const
{
	return (this->x * b.x) + (this->y * b.y) + (this->z * b.z);
}

float gbe::Vector3::Magnitude() const
{
	return sqrt(this->SqrMagnitude());
}

gbe::Vector3& gbe::Vector3::operator+=(const gbe::Vector3& b) {
	glm::vec3::operator+=((glm::vec3)b);
	return *this;
}

gbe::Vector3& gbe::Vector3::operator-=(const gbe::Vector3& b) {
	glm::vec3::operator-=((glm::vec3)b);
	return *this;
}

gbe::Vector3::operator glm::vec3() const
{
	return glm::vec3(this->x, this->y, this->z);
}

const float* gbe::Vector3::Get_Ptr() {
	return &this->x;
}


const gbe::Vector3 gbe::Vector3::zero = gbe::Vector3(0, 0, 0);

gbe::Vector3 gbe::Vector3::GetClosestPointOnLineGivenLine(const Vector3& a, const Vector3& adir, const Vector3& b, const Vector3& bdir) {
    Vector3 cdir = a - b;

    // Check for parallelism to prevent division by zero
    auto bottom = (adir.Dot(adir) * bdir.Dot(bdir)) - (adir.Dot(bdir) * adir.Dot(bdir));
    const float epsilon = 1e-6f; // A small tolerance for floating-point comparison

    if (std::abs(bottom) < epsilon) {
        auto t_parallel = cdir.Dot(adir) / adir.Dot(adir);
        return a + (adir * t_parallel);
    }

    auto top = -(adir.Dot(bdir) * bdir.Dot(cdir)) + (adir.Dot(cdir) * bdir.Dot(bdir));
    auto t = -top / bottom;

    return a + (adir * t);
}