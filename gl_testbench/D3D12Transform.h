#pragma once

#include "Transform.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class D3D12Transform : public Transform {
public:
	D3D12Transform();
	~D3D12Transform();
	void translate(float x, float y, float z);
	void rotate(float radians, float x, float y, float z);
	glm::mat4 transform;
};