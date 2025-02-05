#include "TestBed.h"

#include "AssetManager.h"
#include "RefEngine.h"
#include "MaterialManager.h"

#include "component/CameraComponent.h"
#include "component/DebugUIComponent_GLFW.h"
#include "component/FlyInputComponent.h"
#include "component/LightComponent.h"
#include "component/RenderableComponent.h"
#include "component/SpinComponent.h"
#include "component/TransformComponent.h"

#include "entity/Entity.h"

#include "file/FBXFile.h"

#include "graphics/Material.h"
#include "graphics/Mesh.h"
#include "graphics/Prims.h"

#include <cmath>

#include <memory>
#include <vector>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>



using namespace reng;

static const std::vector<VertexAttribute> UVVertexAttributes{
    VertexAttribute::Create<float>(0, 3), // position, 3 float
    VertexAttribute::Create<float>(0, 2)  // uv, 2 floats
};

static const std::vector<VertexAttribute> FBXVertexAttributes {
    VertexAttribute::Create<float>(offsetof(FBXVertex, position), 4),
    VertexAttribute::Create<float>(offsetof(FBXVertex, colour), 4),
    VertexAttribute::Create<float>(offsetof(FBXVertex, normal), 4),
    VertexAttribute::Create<float>(offsetof(FBXVertex, tangent), 4),
    VertexAttribute::Create<float>(offsetof(FBXVertex, binormal), 4),
    VertexAttribute::Create<float>(offsetof(FBXVertex, indices), 4),
    VertexAttribute::Create<float>(offsetof(FBXVertex, weights), 4),
    VertexAttribute::Create<float>(offsetof(FBXVertex, texCoord1), 2),
    VertexAttribute::Create<float>(offsetof(FBXVertex, texCoord2), 2)
};


class VertexColorComponent : public UpdateComponent {
public:
    VertexColorComponent(const Primitive& a_colorPrim) :
        colorPrim(a_colorPrim),
        colors(a_colorPrim.accessor.count*4),
        accumTime(0)
    {
        colorPrim.accessor = BufferAccessor(colors, 1);
    }

    void Update(double deltaTime, ComponentDatabase& database ) override
    {
        accumTime += deltaTime;
        size_t colorsSize = colorPrim.accessor.count;
        float* colorBuffer = (float*)colorPrim.accessor.buffer.data;
        for (uint i = 0; i < colorsSize; i++)
        {
            float mult = 1 + (i / 3.0f); // 1, 2 or 3
            mult /= 3.0f;
            float time = (float)accumTime + mult;
            float normTime = (time - (int)time); // fractional part
            colorBuffer[i] = sin( normTime );
        }
        colorPrim.UpdateBuffer();
    }

private:
    Primitive colorPrim;
    std::vector<float> colors;
    double accumTime;
};


TestBed::TestBed() :
    m_pWindow(std::make_unique<RefEngWindow>())
{}

bool TestBed::Init()
{
    if( !m_pWindow->Init() ) return false;
    m_pEngine = m_pWindow->GetEngine();

	m_pEngine->RegisterUpdateComponent<CameraComponent>();
	m_pEngine->RegisterUpdateComponent<FlyInputComponent>();
    m_pEngine->RegisterUpdateComponent<SpinComponent>();
    m_pEngine->RegisterUpdateComponent<VertexColorComponent>();

	glm::vec2 windowSize = m_pWindow->GetSize();
    
    Entity& debugUIEnt = m_pEngine->EmplaceEntity("DebugUI");
    debugUIEnt.EmplaceComponent<DebugUIComponent>( m_pEngine, m_pWindow->GetWindow() );

	// Add a default camera
	std::shared_ptr<Camera> pCamera = std::make_shared<Camera>(45.f, windowSize.x/windowSize.y, 0.1f, 100.f);
	Entity& cameraEntity = m_pEngine->EmplaceEntity("Camera");
	Transform cameraTransform = Transform(glm::vec3(0, 20, 20));
	cameraTransform.LookAt(glm::vec3(0));
	auto cameraTransformHandle = cameraEntity.EmplaceComponent<TransformComponent>(cameraTransform);
	cameraEntity.EmplaceComponent<CameraComponent>(cameraTransformHandle, pCamera);

    Entity& flyEntity = m_pEngine->EmplaceEntity("FlyInput");
    flyEntity.EmplaceComponent<FlyInputComponent>(cameraTransformHandle, m_pWindow->GetWindow(), 5.f, 0.2f);

    Entity& lightEntity = m_pEngine->EmplaceEntity("Light");
    
    Transform lightTransform(glm::vec3(-50,50,50));
	lightTransform.LookAt(glm::vec3(0));
    auto lightTransformHandle = lightEntity.EmplaceComponent<TransformComponent>(lightTransform);
    lightEntity.EmplaceComponent<LightComponent>(lightTransformHandle);

    AddTexturedQuad(glm::vec3(0, 0, 0));


    AddSpinningTri(glm::vec3(-2, 1, 0));
    AddSpinningTri(glm::vec3(2, 1, 0));

    //AddVertexColoredCube(glm::vec3(3, 2, 3));

    AddLitCube(glm::vec3(0, 0, 0));

    //AddFbxModel(glm::vec3(0, 0, 3), "assets/models/cube/cube.fbx");
    AddFbxModel(glm::vec3(2, -2, 3), "assets/models/ruinedtank/tank.fbx");

    return true;
}

void TestBed::Run()
{
    m_pWindow->Run();
}

void TestBed::AddTexturedQuad(glm::vec3 pos)
{
    MaterialDefinition texturedMatDef(
        "assets/shaders/TexCoord.vert",
        "assets/shaders/Textured.frag",
        "assets/textures/Big_pebbles_pxr128.png"
        );

    auto texturedMat = MaterialManager::LoadMaterial(m_pEngine->GetAssetManager(), texturedMatDef);
    auto accessor = BufferAccessor(Prims::Quad_VerticesAndUVs, 5);
    auto pQuadMesh = Mesh::Create<float>(accessor, UVVertexAttributes, BufferAccessor(Prims::Quad_Indices, 1));

    Entity& entTexQuad = m_pEngine->EmplaceEntity("Textured Quad");
    auto texQuadTrans = entTexQuad.EmplaceComponent<TransformComponent>(Transform(pos, glm::quat(glm::vec3(glm::half_pi<float>(), 0, 0)), glm::vec3(3)));
    entTexQuad.EmplaceComponent<RenderableComponent>(texQuadTrans, pQuadMesh, texturedMat);
}

void TestBed::AddSpinningTri(glm::vec3 pos)
{
    MaterialDefinition redMatDef(
        "assets/shaders/Default.vert",
        "assets/shaders/Red.frag"
        );
    const auto& pRedMaterial = MaterialManager::LoadMaterial(m_pEngine->GetAssetManager(), redMatDef);

    // Non-indexed tri
    // TODO: Meshes in AssetManager so they're cached.
    std::shared_ptr<Mesh> pTriMesh = Mesh::Create<float>(BufferAccessor(Prims::Triangle_Vertices, 3));
    Entity& entSpin1 = m_pEngine->EmplaceEntity("Spinning Tri");
    auto spinTrans1 = entSpin1.EmplaceComponent<TransformComponent>(pos);
    entSpin1.EmplaceComponent<RenderableComponent>(spinTrans1, pTriMesh, pRedMaterial);
    entSpin1.EmplaceComponent<SpinComponent>(spinTrans1, 10.0f);
}

void TestBed::AddVertexColoredCube(glm::vec3 pos)
{
    std::vector<Primitive> cubeBuffers;
    cubeBuffers.emplace_back(BufferAccessor(Prims::Cube_Vertices, 3), Primitive::Vec3VertexAttribute);
    cubeBuffers.emplace_back(BufferAccessor(Prims::Cube_Colors, 4), Primitive::Vec4VertexAttribute, false);
    std::shared_ptr<Mesh> pColoredCubeMesh = Mesh::Create(cubeBuffers, BufferAccessor(Prims::Cube_Indices, 1));

    MaterialDefinition vertColoredMatDef(
        "assets/shaders/Color.vert",
        "assets/shaders/VertexColor.frag"
        );
    const auto& pVertexColorMaterial = MaterialManager::LoadMaterial(m_pEngine->GetAssetManager(), vertColoredMatDef);

    Entity& colorCubeEnt = m_pEngine->EmplaceEntity("Vert Colored Cube");
    auto colorCubeTrans = colorCubeEnt.EmplaceComponent<TransformComponent>(pos);
    colorCubeEnt.EmplaceComponent<RenderableComponent>(colorCubeTrans, pColoredCubeMesh, pVertexColorMaterial);
    colorCubeEnt.EmplaceComponent<VertexColorComponent>(cubeBuffers[1]);
}

void TestBed::AddLitCube(glm::vec3 pos)
{
    MaterialDefinition litMatDef(
        "assets/shaders/Normal.vert",
        "assets/shaders/VertexLit.frag"
        );
    const auto& litMaterial = MaterialManager::LoadMaterial(m_pEngine->GetAssetManager(), litMatDef);
    litMaterial->SetSpecularPower(2.0f);

    std::shared_ptr<Mesh> pCubeMesh = Mesh::Create<float>(BufferAccessor(Prims::Cube_VerticesAndNormals, 6), Primitive::VertexPositionAndNormalsAttribute, BufferAccessor(Prims::Cube_Indices, 1));
    Entity& indexedCubeEnt = m_pEngine->EmplaceEntity("Lit Cube");
    auto indexedCubeTrans = indexedCubeEnt.EmplaceComponent<TransformComponent>(pos);
    indexedCubeEnt.EmplaceComponent<RenderableComponent>(indexedCubeTrans, pCubeMesh, litMaterial);
}

void LoadAndSetTexture(AssetManager* pAssetManager, Material* pMaterial, FBXMaterial* pFbxMaterial, FBXMaterial::TextureTypes textureType, const char* location, int textureUnit)
{
    if( FBXTexture* pFbxTexture = pFbxMaterial->textures[textureType] ) {
        auto texture = pAssetManager->LoadTexture(pFbxTexture->path.c_str(), textureUnit);
        pMaterial->SetSampler(location, texture.textureId, textureUnit);

    }
}

void TestBed::AddFbxModel(glm::vec3 pos, const char* fbxFilename)
{
    MaterialDefinition litMatDef(
        "assets/shaders/TexCoordNormalTangent.vert",
        "assets/shaders/TexturedNormalLit.frag"
        );
    const auto& litMaterial = MaterialManager::LoadMaterial(m_pEngine->GetAssetManager(), litMatDef);

    Entity& fbxEnt = m_pEngine->EmplaceEntity("Fbx Model");
    auto transformComponentHandle = fbxEnt.EmplaceComponent<TransformComponent>(Transform(pos));

    auto fbx = std::shared_ptr<FBXFile>(new FBXFile());
    fbx->load(fbxFilename);

    auto pAssetManager = m_pEngine->GetAssetManager();

    for (uint i = 0; i < fbx->getMeshCount(); i++) {
        FBXMeshNode* pMesh = fbx->getMeshByIndex(i);
        if (pMesh->m_vertices.size() >  0) {
            auto pMaterial = std::make_shared<Material>(*litMaterial); // Take a copy

            // Create one mesh instanace for each material
            for( auto pFbxMaterial : pMesh->m_materials )
            {
                LoadAndSetTexture(pAssetManager, pMaterial.get(), pFbxMaterial, FBXMaterial::DiffuseTexture, WellKnownLocation::DiffuseSampler, (int)WellKnownTextureUnit::Diffuse);

                LoadAndSetTexture(pAssetManager, pMaterial.get(), pFbxMaterial, FBXMaterial::NormalTexture, WellKnownLocation::NormalSampler, (int)WellKnownTextureUnit::Normal);

                const BufferAccessor accessor(pMesh->m_vertices, 1);
                auto pFbxMesh = Mesh::Create<FBXVertex>(accessor, FBXVertexAttributes, BufferAccessor(pMesh->m_indices, 1));
                fbxEnt.EmplaceComponent<RenderableComponent>(transformComponentHandle, pFbxMesh, pMaterial);
            }
        }
    }
}
