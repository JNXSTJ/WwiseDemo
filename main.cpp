//***************************************************************************************
// SkinnedMeshApp.cpp by Frank Luna (C) 2015 All Rights Reserved.
//***************************************************************************************

#include "Common/d3dApp.h"
#include "Common/MathHelper.h"
#include "Common/UploadBuffer.h"
#include "Common/GeometryGenerator.h"
#include "Common/Camera.h"
#include "FrameResource.h"
#include "ShadowMap.h"
#include "Ssao.h"
#include "SkinnedData.h"
#include "LoadM3d.h"
#include <iostream>
#include <tracy/Tracy.hpp>
#include <vector>
#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Exporter.hpp>
#include <DirectXMath.h>
#include "audio.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX::PackedVector;
using namespace DirectX;

namespace myengine
{
    namespace assimp
    {
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

    }
}




struct SkinnedModelInstance
{
    SkinnedData* SkinnedInfo = nullptr;
    std::vector<DirectX::XMFLOAT4X4> FinalTransforms;
    std::string ClipName;
    float TimePos = 0.0f;

    // Called every frame and increments the time position, interpolates the 
    // animations for each bone based on the current animation clip, and 
    // generates the final transforms which are ultimately set to the effect
    // for processing in the vertex shader.
    void UpdateSkinnedAnimation(float dt)
    {
        TimePos += dt;

        // Loop animation
        if (TimePos > SkinnedInfo->GetClipEndTime(ClipName))
            TimePos = 0.0f;

        // Compute the final transforms for this time position.
        SkinnedInfo->GetFinalTransforms(ClipName, TimePos, FinalTransforms);
    }
};

// Lightweight structure stores parameters to draw a shape.  This will
// vary from app-to-app.
struct RenderItem
{
    RenderItem() = default;
    RenderItem(const RenderItem& rhs) = delete;

    // World matrix of the shape that describes the object's local space
    // relative to the world space, which defines the position, orientation,
    // and scale of the object in the world.
    XMFLOAT4X4 World = MathHelper::Identity4x4();

    XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();

    // Dirty flag indicating the object data has changed and we need to update the constant buffer.
    // Because we have an object cbuffer for each FrameResource, we have to apply the
    // update to each FrameResource.  Thus, when we modify obect data we should set 
    // NumFramesDirty = gNumFrameResources so that each frame resource gets the update.
    int NumFramesDirty = gNumFrameResources;

    // Index into GPU constant buffer corresponding to the ObjectCB for this render item.
    UINT ObjCBIndex = -1;

    Material* Mat = nullptr;
    MeshGeometry* Geo = nullptr;

    // Primitive topology.
    D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // DrawIndexedInstanced parameters.
    UINT IndexCount = 0;
    UINT StartIndexLocation = 0;
    int BaseVertexLocation = 0;

    // Only applicable to skinned render-items.
    UINT SkinnedCBIndex = -1;

    // nullptr if this render-item is not animated by skinned mesh.
    SkinnedModelInstance* SkinnedModelInst = nullptr;
};

enum class RenderLayer : int
{
    Opaque = 0,
    SkinnedOpaque,
    Debug,
    Sky,
    Count
};

class SkinnedMeshApp : public D3DApp
{
public:
    SkinnedMeshApp(HINSTANCE hInstance);
    SkinnedMeshApp(const SkinnedMeshApp& rhs) = delete;
    SkinnedMeshApp& operator=(const SkinnedMeshApp& rhs) = delete;
    ~SkinnedMeshApp();

    virtual bool Initialize()override;

private:
    virtual void CreateRtvAndDsvDescriptorHeaps()override;
    virtual void OnResize()override;
    virtual void Update(const GameTimer& gt)override;
    virtual void Draw(const GameTimer& gt)override;

    virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
    virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
    virtual void OnMouseMove(WPARAM btnState, int x, int y)override;

    void OnKeyboardInput(const GameTimer& gt);
    void AnimateMaterials(const GameTimer& gt);
    void UpdateObjectCBs(const GameTimer& gt);
    void UpdateSkinnedCBs(const GameTimer& gt);
    void UpdateMaterialBuffer(const GameTimer& gt);
    void UpdateShadowTransform(const GameTimer& gt);
    void UpdateMainPassCB(const GameTimer& gt);
    void UpdateShadowPassCB(const GameTimer& gt);
    void UpdateSsaoCB(const GameTimer& gt);

    void LoadTextures();
    void BuildRootSignature();
    void BuildSsaoRootSignature();
    void BuildDescriptorHeaps();
    void BuildShadersAndInputLayout();
    void BuildShapeGeometry();
    void LoadSkinnedModel();
    void BuildPSOs();
    void BuildFrameResources();
    void BuildMaterials();
    void BuildRenderItems();
    void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);
    void DrawSceneToShadowMap();
    void DrawNormalsAndDepth();

    CD3DX12_CPU_DESCRIPTOR_HANDLE GetCpuSrv(int index)const;
    CD3DX12_GPU_DESCRIPTOR_HANDLE GetGpuSrv(int index)const;
    CD3DX12_CPU_DESCRIPTOR_HANDLE GetDsv(int index)const;
    CD3DX12_CPU_DESCRIPTOR_HANDLE GetRtv(int index)const;

    std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7> GetStaticSamplers();

private:

    std::vector<std::unique_ptr<FrameResource>> mFrameResources;
    FrameResource* mCurrFrameResource = nullptr;
    int mCurrFrameResourceIndex = 0;

    ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
    ComPtr<ID3D12RootSignature> mSsaoRootSignature = nullptr;

    ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;

    std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
    std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;
    std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures;
    std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
    std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;

    std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
    std::vector<D3D12_INPUT_ELEMENT_DESC> mSkinnedInputLayout;

    // List of all the render items.
    std::vector<std::unique_ptr<RenderItem>> mAllRitems;

    // Render items divided by PSO.
    std::vector<RenderItem*> mRitemLayer[(int)RenderLayer::Count];

    UINT mSkyTexHeapIndex = 0;
    UINT mShadowMapHeapIndex = 0;
    UINT mSsaoHeapIndexStart = 0;
    UINT mSsaoAmbientMapIndex = 0;

    UINT mNullCubeSrvIndex = 0;
    UINT mNullTexSrvIndex1 = 0;
    UINT mNullTexSrvIndex2 = 0;

    CD3DX12_GPU_DESCRIPTOR_HANDLE mNullSrv;

    PassConstants mMainPassCB;  // index 0 of pass cbuffer.
    PassConstants mShadowPassCB;// index 1 of pass cbuffer.

    UINT mSkinnedSrvHeapStart = 0;
    std::string mSkinnedModelFilename = "Models\\soldier.m3d";
    std::unique_ptr<SkinnedModelInstance> mSkinnedModelInst;
    SkinnedData mSkinnedInfo;
    std::vector<M3DLoader::Subset> mSkinnedSubsets;
    std::vector<M3DLoader::M3dMaterial> mSkinnedMats;
    std::vector<std::string> mSkinnedTextureNames;

    Camera mCamera;

    std::unique_ptr<ShadowMap> mShadowMap;

    std::unique_ptr<Ssao> mSsao;

    DirectX::BoundingSphere mSceneBounds;

    float mLightNearZ = 0.0f;
    float mLightFarZ = 0.0f;
    XMFLOAT3 mLightPosW;
    XMFLOAT4X4 mLightView = MathHelper::Identity4x4();
    XMFLOAT4X4 mLightProj = MathHelper::Identity4x4();
    XMFLOAT4X4 mShadowTransform = MathHelper::Identity4x4();

    float mLightRotationAngle = 0.0f;
    XMFLOAT3 mBaseLightDirections[3] = {
        XMFLOAT3(0.57735f, -0.57735f, 0.57735f),
        XMFLOAT3(-0.57735f, -0.57735f, 0.57735f),
        XMFLOAT3(0.0f, -0.707f, -0.707f)
    };
    XMFLOAT3 mRotatedLightDirections[3];

    POINT mLastMousePos;
};
 

void PrintWorkdir() {
    // 定义一个缓冲区来存储当前工作路径
    wchar_t buffer[MAX_PATH];

    // 获取当前工作路径
    DWORD dwRet = GetCurrentDirectory(MAX_PATH, buffer);

    // 检查是否成功获取路径
    if (dwRet == 0) {
        // 获取失败，显示错误消息
        MessageBox(NULL, L"Failed to get current directory.", L"Error", MB_OK | MB_ICONERROR);
        return; // 退出程序
    }


    // 显示当前工作路径
    MessageBox(NULL, buffer, L"Current Directory", MB_OK | MB_ICONINFORMATION);

    // ... 其他代码 ...

    return; // 正常退出程序
}
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
    PSTR cmdLine, int showCmd)
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	//myengine::assimp::Model ElysiaBody("C:/Users/taojian/Desktop/wwise_demo/asset/rain_restaurant.obj");
    PrintWorkdir();


	//ZoneScoped;
	//myengine::audio::Audio& instance = myengine::audio::Audio::Instance();
    //instance.LoadBnk("Init.bnk");
	//bool flag = false;
	//while (true)
	//{
	//	::Sleep(1000);
	//	if (flag == false)
	//	{
	//		flag = true;
	//		instance.LoadBnk("Reflect.bnk");
	//		auto id = instance.RegisterGameObject("taojian");
	//		instance.PostEvent(id, "Play_Reflect_Emitter");
	//		AK::SoundEngine::RenderAudio();
	//	}
	//}

	auto inFile = "C:\\Users\\taojian\\Desktop\\wwise_demo\\asset\\rain_restaurant.obj";

    try
    {
        SkinnedMeshApp theApp(hInstance);
        if (!theApp.Initialize())
            return 0;

        return theApp.Run();
    }
    catch (DxException& e)
    {
        MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
        return 0;
    }
}




