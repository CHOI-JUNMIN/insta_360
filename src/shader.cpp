#include "shader.h"

ShaderUPtr Shader::CreateFromFile(const std::string &filename, GLenum shaderType)
{
    auto shader = ShaderUPtr(new Shader());

    if (!shader->LoadFile(filename, shaderType))
    {
        SPDLOG_ERROR("❌ Failed to create shader from file: {}", filename);
        return nullptr;
    }
    return std::move(shader);
}

Shader::~Shader()
{
    if (m_shader)
    {
        glDeleteShader(m_shader);
        m_shader = 0;
    }
}

bool Shader::LoadFile(const std::string &filename, GLenum shaderType)
{
    auto result = LoadTextFile(filename);
    if (!result.has_value())
    {
        SPDLOG_ERROR("Failed to load shader file: {}", filename);
        return false;
    }

    auto &code = result.value();
    if (code.empty())
    {
        SPDLOG_ERROR("Shader file is empty: {}", filename);
        return false;
    }

    const char *codePtr = code.c_str();
    int32_t codeLength = static_cast<int32_t>(code.length());

    // 셰이더 생성 및 컴파일
    m_shader = glCreateShader(shaderType);
    glShaderSource(m_shader, 1, &codePtr, &codeLength);
    glCompileShader(m_shader);

    return true;
}