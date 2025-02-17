#include "model.h"
#include <iostream>

ModelUPtr Model::Load(const std::string &filename) // 모델로드 함수
{
    auto model = ModelUPtr(new Model());
    if (!model->LoadByAssimp(filename))
        return nullptr;
    return std::move(model);
}

bool Model::LoadByAssimp(const std::string &filename) // Assimp라이브러리 사용 모델로드
{
    Assimp::Importer importer;
    auto scene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals); // 모델 파일로드
    //                                          모든메쉬 삼각형 변환    UV좌표 상하 반전
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    { // 씬이 제대로 로드되지 않았거나, 불완전하거나, 루트 노드가 없는 경우
        //SPDLOG_ERROR("failed to load model: {}", filename);
        return false;
    }

    auto dirname = filename.substr(0, filename.find_last_of("/")); // 파일 이름에서 디렉토리 이름을 추출

    auto LoadTexture = [&](aiMaterial *material, aiTextureType type, const std::string &dirname) -> TextureUPtr
    {
        if (material->GetTextureCount(type) <= 0)
            return nullptr; // 텍스처가 없으면 nullptr 반환

        aiString filepath;
        material->GetTexture(type, 0, &filepath);
        std::string fullPath = fmt::format("{}/{}", dirname, filepath.C_Str());

        std::cout << "Loading texture: " << fullPath << std::endl;

        auto image = Image::Load(fullPath);
        if (!image)
        {
            std::cout << "❌ Failed to load image: " << fullPath << std::endl;
            return nullptr;
        }

        // 🔥 OpenGL 텍스처 생성 (Texture.cpp에서 정의된 함수 사용)
        auto texture = Texture::CreateFromImage(image.get());
        if (!texture)
        {
            std::cout << "❌ Failed to create OpenGL texture from image: " << fullPath << std::endl;
            return nullptr;
        }

        std::cout << "✅ OpenGL texture created successfully: " << fullPath << std::endl;
        return texture;
    };

    for (uint32_t i = 0; i < scene->mNumMaterials; i++)
    {
        auto material = scene->mMaterials[i];
        auto glMaterial = Material::Create();

        glMaterial->diffuse = LoadTexture(material, aiTextureType_DIFFUSE, dirname);
        glMaterial->specular = LoadTexture(material, aiTextureType_SPECULAR, dirname);

        if (!glMaterial->diffuse)
            std::cout << "Material " << i << " has no diffuse texture." << std::endl;
        if (!glMaterial->specular)
            //std::cout << "Material " << i << " has no specular texture." << std::endl;

        m_materials.push_back(std::move(glMaterial));
    }

    ProcessNode(scene->mRootNode, scene); // 루트 노드부터 시작해 씬을 재귀적으로 처리하는 ProcessNode 함수를 호출);
    // ShutdownAssimpLogger();
    return true;
}

void Model::ProcessNode(aiNode *node, const aiScene *scene) // 현재 노드에 포함된 메쉬를 반복하면서 처리
{
    aiMatrix4x4 transformation = node->mTransformation;
    aiVector3D scaling;
    aiQuaternion rotation;
    aiVector3D position;

    transformation.Decompose(scaling, rotation, position);

    position.x *= 0.001;
    position.y *= 0.001;
    position.z *= 0.001;

    // if (node->mParent)
    // {
    //     SPDLOG_INFO("Node name: {}, Parent name: {}, Node position: x={}, y={}, z={}", node->mName.C_Str(), node->mParent->mName.C_Str(), position.x, position.y, position.z);
    // }
    // else
    // {
    //     SPDLOG_INFO("Node name: {}, Parent name: None (Root Node), Node position: x={}, y={}, z={}", node->mName.C_Str(), position.x, position.y, position.z);
    // }

    for (uint32_t i = 0; i < node->mNumMeshes; i++)
    {

        auto meshIndex = node->mMeshes[i];
        auto mesh = scene->mMeshes[meshIndex];

        auto glMesh = ProcessMesh(mesh, scene);
        glMesh->SetNodeName(node->mName.C_Str()); // 현재 노드의 이름을 메쉬에 설정합니다.

        // 처리한 메쉬를 m_meshes 벡터에 추가합니다.
        m_meshes.push_back(std::move(glMesh));

        // ProcessMesh(mesh, scene);
    }

    for (uint32_t i = 0; i < node->mNumChildren; i++) // 자식 노드가 있으면 똑같이 ㄱㄱ
    {
        aiNode *childNode = node->mChildren[i];
        nodeHierarchy[node->mName.C_Str()].push_back(childNode->mName.C_Str());
        ProcessNode(node->mChildren[i], scene);
    }
}

MeshUPtr Model::ProcessMesh(aiMesh *mesh, const aiScene *scene) // 메쉬 데이터 처리
{
    // SPDLOG_INFO("process mesh: {}, #vert: {}, #face: {}", mesh->mName.C_Str(), mesh->mNumVertices, mesh->mNumFaces);

    std::vector<Vertex> vertices; // 정점(vertex) 데이터 처리
    vertices.resize(mesh->mNumVertices);

    for (uint32_t i = 0; i < mesh->mNumVertices; i++)
    {
        auto &v = vertices[i];
        v.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        v.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

        // 🔥 주석 해제하여 UV 좌표 사용 가능하게 함
        if (mesh->mTextureCoords[0]) // UV 좌표가 존재하는지 확인
        {
            v.texCoord = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        }
    }

    // for (uint32_t i = 0; i < mesh->mNumVertices; i++)
    // {
    //     auto &v = vertices[i];
    //     v.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
    //     v.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
    //     v.texCoord = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
    // }

    std::vector<uint32_t> indices; // 메쉬의 인덱스 데이터를 처리합니다. 각 면(Face)에서 세 개의 인덱스를 가져와 삼각형을 구성
    indices.resize(mesh->mNumFaces * 3);

    for (uint32_t i = 0; i < mesh->mNumFaces; i++)
    {
        indices[3 * i] = mesh->mFaces[i].mIndices[0];
        indices[3 * i + 1] = mesh->mFaces[i].mIndices[1];
        indices[3 * i + 2] = mesh->mFaces[i].mIndices[2];
    }

    auto glMesh = Mesh::Create(vertices, indices, GL_TRIANGLES); // 정점(Vertex)와 인덱스(Index) 데이터를 사용해 OpenGL 메쉬 객체를 생성

    if (mesh->mMaterialIndex >= 0) // 메쉬에 재질이 연결되어 있으면, 해당 재질을 메쉬에 설정
    {
        glMesh->SetMaterial(m_materials[mesh->mMaterialIndex]);
    }
    return glMesh;
    // m_meshes.push_back(std::move(glMesh));
}
