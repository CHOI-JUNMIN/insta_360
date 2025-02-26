#pragma once
#include <memory>
#include <string>
#include <optional>
#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <spdlog/spdlog.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

std::optional<std::string> LoadTextFile(const std::string &filename);
glm::vec3 GetAttenuationCoeff(float distance);

#define CLASS_PTR(klassName)                            \
    class klassName;                                    \
    using klassName##UPtr = std::unique_ptr<klassName>; \
    using klassName##Ptr = std::shared_ptr<klassName>;  \
    using klassName##WPtr = std::weak_ptr<klassName>;