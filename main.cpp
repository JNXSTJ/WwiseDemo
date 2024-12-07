#include <iostream>
#include <tracy/Tracy.hpp>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Exporter.hpp>
#include <DirectXMath.h>
#include "audio.h"
using namespace DirectX;

struct Vertex
{
	Vertex() = default;
	Vertex(const Vertex& rhs)
	{
		this->position = rhs.position;
		this->normal = rhs.normal;
		this->tangent = rhs.tangent;
		this->texCoord = rhs.texCoord;
	}
	Vertex& operator= (Vertex& rhs)
	{
		return rhs;
	}

	Vertex(Vertex&& rhs) = default;

	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT3 tangent;
	XMFLOAT2 texCoord;
};

class Model
{
public:

	struct Mesh
	{
		Mesh() = default;
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		Mesh(std::vector<Vertex>& vertices, std::vector<UINT>& indices)
		{
			this->vertices = vertices;
			this->indices = indices;
		}
	};

	// load scene for assimp
	Model(const std::string& path);

	// Traverse and process the nodes in assimp in turn
	void TraverseNode(const aiScene* scene, aiNode* node);

	// load mesh, which includes vertex, index, normal, tangent, texture, material information
	Mesh LoadMesh(const aiScene* scene, aiMesh* mesh);

	std::vector<Vertex> GetVertices();

	std::vector<uint32_t> GetIndices();

private:
	std::string directory;
	std::vector<Mesh> m_meshs;
};


Model::Model(const std::string& path)
{
	Assimp::Importer localImporter;

	const aiScene* pLocalScene = localImporter.ReadFile(
		path,
		// Triangulates all faces of all meshes
		aiProcess_Triangulate |
		// Supersedes the aiProcess_MakeLeftHanded and aiProcess_FlipUVs and aiProcess_FlipWindingOrder flags
		aiProcess_ConvertToLeftHanded |
		// This preset enables almost every optimization step to achieve perfectly optimized data. In D3D, need combine with aiProcess_ConvertToLeftHanded
		aiProcessPreset_TargetRealtime_MaxQuality |
		// Calculates the tangents and bitangents for the imported meshes
		aiProcess_CalcTangentSpace |
		// Splits large meshes into smaller sub-meshes
		// This is quite useful for real-time rendering, 
		// where the number of triangles which can be maximally processed in a single draw - call is limited by the video driver / hardware
		aiProcess_SplitLargeMeshes |
		// A postprocessing step to reduce the number of meshes
		aiProcess_OptimizeMeshes |
		// A postprocessing step to optimize the scene hierarchy
		aiProcess_OptimizeGraph);

	// "localScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE" is used to check whether value data returned is incomplete
	if (pLocalScene == nullptr || pLocalScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || pLocalScene->mRootNode == nullptr)
	{
		std::cout << "ERROR::ASSIMP::" << localImporter.GetErrorString() << std::endl;
	}

	directory = path.substr(0, path.find_last_of('/'));

	TraverseNode(pLocalScene, pLocalScene->mRootNode);
}

void Model::TraverseNode(const aiScene* scene, aiNode* node)
{
	// load mesh
	for (UINT i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh* pLocalMesh = scene->mMeshes[node->mMeshes[i]];
		m_meshs.push_back(LoadMesh(scene, pLocalMesh));
	}

	// traverse child node
	for (UINT i = 0; i < node->mNumChildren; ++i)
	{
		TraverseNode(scene, node->mChildren[i]);
	}
}

Model::Mesh Model::LoadMesh(const aiScene* scene, aiMesh* mesh)
{
	std::vector<Vertex> localVertices;
	std::vector<uint32_t> localIndices;

	// process vertex position, normal, tangent, texture coordinates
	for (UINT i = 0; i < mesh->mNumVertices; ++i)
	{
		Vertex localVertex;

		localVertex.position.x = mesh->mVertices[i].x;
		localVertex.position.y = mesh->mVertices[i].y;
		localVertex.position.z = mesh->mVertices[i].z;

		localVertex.normal.x = mesh->mNormals[i].x;
		localVertex.normal.y = mesh->mNormals[i].y;
		localVertex.normal.z = mesh->mNormals[i].z;

		if (mesh->mTangents != nullptr)
		{
			localVertex.tangent.x = mesh->mTangents[i].x;
			localVertex.tangent.y = mesh->mTangents[i].y;
			localVertex.tangent.z = mesh->mTangents[i].z;
		}

		// assimp allow one model have 8 different texture coordinates in one vertex, but we just care first texture coordinates because we will not use so many
		if (mesh->mTextureCoords[0])
		{
			localVertex.texCoord.x = mesh->mTextureCoords[0][i].x;
			localVertex.texCoord.y = mesh->mTextureCoords[0][i].y;
		}
		else
		{
			localVertex.texCoord = XMFLOAT2(0.0f, 0.0f);
		}

		localVertices.push_back(localVertex);
	}

	for (UINT i = 0; i < mesh->mNumFaces; ++i)
	{
		aiFace localFace = mesh->mFaces[i];
		for (UINT j = 0; j < localFace.mNumIndices; ++j)
		{
			localIndices.push_back(localFace.mIndices[j]);
		}
	}

	return Mesh(localVertices, localIndices);
}

std::vector<Vertex> Model::GetVertices()
{
	std::vector<Vertex> localVertices;

	for (auto& m : m_meshs)
	{
		for (auto& v : m.vertices)
		{
			localVertices.push_back(v);
		}
	}

	return localVertices;
}

std::vector<uint32_t> Model::GetIndices()
{
	std::vector<uint32_t> localIndices;

	for (auto& m : m_meshs)
	{
		for (auto& i : m.indices)
		{
			localIndices.push_back(i);
		}
	}

	return localIndices;
}

int main()
{
	Model ElysiaBody("C:/Users/taojian/Desktop/wwise_demo/asset/rain_restaurant.obj");

	ZoneScoped;
	myengine::audio::Audio& instance = myengine::audio::Audio::Instance();
	instance.LoadBnk("Init.bnk");
	bool flag = false;
	while (true)
	{
		::Sleep(1000);
		if (flag == false)
		{
			flag = true;
			instance.LoadBnk("Reflect.bnk");
			auto id = instance.RegisterGameObject("taojian");
			instance.PostEvent(id, "Play_Reflect_Emitter");
			AK::SoundEngine::RenderAudio();
		}
	}

	auto inFile = "C:\\Users\\taojian\\Desktop\\wwise_demo\\asset\\rain_restaurant.obj";

	return 0;
}