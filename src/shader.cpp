#include "shader.h"

ShaderUPtr Shader::CreateFromFile(const std::string &filename, GLenum shaderType)
{
    auto shader = ShaderUPtr(new Shader());

    //SPDLOG_INFO("🔍 Trying to load shader: {}", filename);


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

    // 🔥 쉐이더 소스 코드 출력 (디버깅용)
    //SPDLOG_INFO("✅ Successfully loaded shader file: {}", filename);

    const char *codePtr = code.c_str();
    int32_t codeLength = static_cast<int32_t>(code.length());

    // 셰이더 생성 및 컴파일
    m_shader = glCreateShader(shaderType);
    glShaderSource(m_shader, 1, &codePtr, &codeLength);
    glCompileShader(m_shader);

    // 컴파일 에러 체크
    int success = 0;
    glGetShaderiv(m_shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[1024];
        glGetShaderInfoLog(m_shader, 1024, nullptr, infoLog);
        SPDLOG_ERROR("Failed to compile shader: {}", filename);
        SPDLOG_ERROR("Compile error: {}", infoLog);
        return false;
    }
    //SPDLOG_INFO("✅ Shader compiled successfully: {}", filename);
    return true;
}