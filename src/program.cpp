#include "program.h"
#include <iostream>

ProgramUPtr Program::Create(const std::vector<ShaderPtr> &shaders)
{
    // auto program = ProgramUPtr(new Program());
    // if (!program->Link(shaders))
    //     return nullptr;
    // return std::move(program);

    auto program = ProgramUPtr(new Program());
    program->m_program = glCreateProgram();

    for (const auto &shader : shaders)
    {
        glAttachShader(program->m_program, shader->Get());
        //SPDLOG_INFO("✅ Attached shader to program: {}", shader->Get());
    }

    glLinkProgram(program->m_program);

    GLint success = 0;
    glGetProgramiv(program->m_program, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[1024];
        glGetProgramInfoLog(program->m_program, 1024, nullptr, infoLog);
        SPDLOG_ERROR("❌ Program linking error: {}", infoLog);
        return nullptr;
    }

    //SPDLOG_INFO("✅ Shader program linked successfully!");
    return program;
}

ProgramUPtr Program::Create(
    const std::string &vertShaderFilename,  const std::string &fragShaderFilename)
{
    ShaderPtr vs = Shader::CreateFromFile(vertShaderFilename, GL_VERTEX_SHADER);
    ShaderPtr fs = Shader::CreateFromFile(fragShaderFilename, GL_FRAGMENT_SHADER);
    if (!vs || !fs)
        return nullptr;
    return std::move(Create({vs, fs}));
}

Program::~Program()
{
    if (m_program)
    {
        glDeleteProgram(m_program);
    }
}

void Program::Use() const
{
    glUseProgram(m_program);
}

bool Program::Link(const std::vector<ShaderPtr> &shaders)
{
    m_program = glCreateProgram();
    for (auto &shader : shaders)
        glAttachShader(m_program, shader->Get());
    glLinkProgram(m_program);

    int success = 0;
    glGetProgramiv(m_program, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[1024];
        glGetProgramInfoLog(m_program, 1024, nullptr, infoLog);
        SPDLOG_ERROR("failed to link program: {}", infoLog);
        return false;
    }
    return true;
}

void Program::SetUniform(const std::string &name, int value) const
{
    // auto loc = glGetUniformLocation(m_program, name.c_str());
    // glUniform1i(loc, value);

    GLint currentProgram = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram); // 현재 바인딩된 쉐이더 프로그램 가져오기
    //std::cout << "✅ Current Shader Program ID: " << currentProgram << std::endl;

    GLint location = glGetUniformLocation(m_program, name.c_str());
    if (location == -1)
    {
        //std::cout << "❌ Uniform " << name << " not found in shader program " << m_program << std::endl;
        return;
    }
    glUniform1i(location, value);
    //std::cout << "✅ Set Uniform " << name << " = " << value << std::endl;
}

void Program::SetUniform(const std::string &name, const glm::mat4 &value) const
{
    auto loc = glGetUniformLocation(m_program, name.c_str());
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));
}

void Program::SetUniform(const std::string &name, float value) const
{
    auto loc = glGetUniformLocation(m_program, name.c_str());
    glUniform1f(loc, value);
}

void Program::SetUniform(const std::string &name, const glm::vec2 &value) const
{
    auto loc = glGetUniformLocation(m_program, name.c_str());
    glUniform2fv(loc, 1, glm::value_ptr(value));
}

void Program::SetUniform(const std::string &name, const glm::vec3 &value) const
{
    auto loc = glGetUniformLocation(m_program, name.c_str());
    glUniform3fv(loc, 1, glm::value_ptr(value));
}

void Program::SetUniform(const std::string &name, const glm::vec4 &value) const
{
    auto loc = glGetUniformLocation(m_program, name.c_str());
    glUniform4fv(loc, 1, glm::value_ptr(value));
}