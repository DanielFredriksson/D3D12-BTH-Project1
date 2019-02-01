#include "D3D12Transform.h"

D3D12Transform::D3D12Transform() 
{

}

D3D12Transform::~D3D12Transform()
{
}

void D3D12Transform::translate(float x, float y, float z)
{
	transform[3][0] += x;
	transform[3][1] += y;
	transform[3][2] += z;
}

void D3D12Transform::rotate(float radians, float x, float y, float z)
{
	transform = glm::rotate(transform, radians, glm::vec3(x, y, z));
}

