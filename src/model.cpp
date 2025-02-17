#include "model.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

void Model::SetPosition(float newPos)
{
    Pos = newPos;
}

void Model::Draw(const Program *program) const
{
    for (const auto &mesh : m_meshes)
    {
        
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        auto material = mesh->GetMaterial(); // 메쉬에 연결된 재질을 가져옴
        glm::vec3 lightPos(0.0f, 1.0f, 1.0f);

        glm::mat NewModelMatrix = glm::translate(modelMatrix, glm::vec3(Pos, 0.0f, 0.0f));

        program->Use(); 

        program->SetUniform("modelMatrix", glm::mat4(1.0f));

        
        program->SetUniform("materialdiffuse", material->diffuseColor);
        program->SetUniform("materialambient", material->ambientColor);
        program->SetUniform("materialspecular", material->specularColor);
        program->SetUniform("materialshininess", material->shininess);

        program->SetUniform("lightDiffuse", glm::vec3(1.0f, 1.0f, 1.0f));  // 확산광 색상
        program->SetUniform("lightSpecular", glm::vec3(0.0f, 0.0f, 0.0f)); // 반사광 색상
        program->SetUniform("lightambient", glm::vec3(0.3f, 0.3f, 0.3f));  // 주변광
        program->SetUniform("lightPosition", lightPos);

        if (material->diffuse) //  Diffuse 텍스처가 있는 경우만 바인딩
        {
            glActiveTexture(GL_TEXTURE0);
            material->diffuse->Bind();
            program->SetUniform("diffuseTexture", 0); //  tex 대신 diffuseTexture 사용
        }

        mesh->Draw(program);

        //  텍스처 바인딩 해제 (다른 모델에 영향 방지)
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}