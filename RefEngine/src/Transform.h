#pragma once

#include "utils/memaligned.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

namespace reng {

class Transform {
public:
	Transform(glm::vec3 position) {
		glm::vec4 pos = glm::vec4(position, 1);
		glm::mat4 trans = glm::mat4(1.0f);
		m_globalTransform[3] = pos;
	}

	Transform( const glm::mat4x4& transform ) :
		m_globalTransform(transform)
	{}

	const glm::mat4x4& GetMartix() const { return m_globalTransform; }
	glm::vec3 GetPosition() const { return glm::vec3(m_globalTransform[3]); };
	glm::vec3 GetRight() const { return glm::vec3(m_globalTransform[0]); }
	glm::vec3 GetUp() const { return glm::vec3(m_globalTransform[1]); }
	glm::vec3 GetForward() const { return glm::vec3(m_globalTransform[2]); }

	void SetMatrix( const glm::mat4x4& transform )  { m_globalTransform = transform; }
	void SetPosition(const glm::vec3& position) { m_globalTransform[3] = glm::vec4(position, 1); }

	ALIGNED_NEW_OP_16
	ALIGNED_DELETE_OP

private:
	glm::mat4x4 m_localTransform;
	glm::mat4x4 m_globalTransform;
};

}