/**
 *  @file    DG_Physics.cpp
 *  @author  Faaux (github.com/Faaux)
 *  @date    03 February 2018
 */

#include "DG_Physics.h"
#include "DG_GraphicsSystem.h"
#include "DG_ResourceManager.h"
#include "PxPhysicsAPI.h"
#include "main.h"

namespace DG
{
using namespace physx;

class PhysicsMeshManager : public ResourceManager<PxTriangleMesh*>
{
   public:
    void LoadOrGet(StringId id, PxTriangleMesh* p) { Register(id, p); }
};

PhysicsMeshManager gPhysicsMeshManager;

PxDefaultAllocator gAllocator;
PxDefaultErrorCallback gErrorCallback;

PxFoundation* gFoundation = nullptr;
PxPhysics* gPhysics = nullptr;
PxCooking* gCooking;

PxDefaultCpuDispatcher* gDispatcher = nullptr;
PxScene* gScene = nullptr;

PxPvd* gPvd = nullptr;
PxMaterial* gMaterial = nullptr;

PxRigidDynamic* lastDynamic;

static PxQuat ToPxQuat(const glm::quat& q)
{
    PxQuat quat;
    quat.w = q.w;
    quat.x = q.x;
    quat.y = q.y;
    quat.z = q.z;
    return quat;
}

static PxVec3 ToPxVec3(const vec3& v)
{
    PxVec3 result(v.x, v.y, v.z);
    return result;
}

static glm::quat ToQuat(const PxQuat& q)
{
    glm::quat quat;
    quat.w = q.w;
    quat.x = q.x;
    quat.y = q.y;
    quat.z = q.z;
    return quat;
}

static vec3 ToVec3(const PxVec3& v)
{
    vec3 result(v.x, v.y, v.z);
    return result;
}

bool PhysicsWorld::Init()
{
    gFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, gAllocator, gErrorCallback);

    gCooking =
        PxCreateCooking(PX_PHYSICS_VERSION, *gFoundation, PxCookingParams(PxTolerancesScale()));

    // PhysX Visual Debugger
    gPvd = PxCreatePvd(*gFoundation);
    PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
    gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

    // Init phsyics
    gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);

    // Create a physics scene
    PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
    gDispatcher = PxDefaultCpuDispatcherCreate(2);
    sceneDesc.cpuDispatcher = gDispatcher;
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;
    sceneDesc.flags |= PxSceneFlag::eENABLE_ACTIVE_ACTORS;
    sceneDesc.flags |= PxSceneFlag::eEXCLUDE_KINEMATICS_FROM_ACTIVE_ACTORS;
    gScene = gPhysics->createScene(sceneDesc);

    // Attach to PhysX Visual Debugger
    PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
    if (pvdClient)
    {
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
    }

    gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);

    PxRigidStatic* groundPlane = PxCreatePlane(*gPhysics, PxPlane(0, 1, 0, 0), *gMaterial);
    gScene->addActor(*groundPlane);

    return true;
}

void PhysicsWorld::ToggleDebugVisualization()
{
    if (_outputDebugLines)
    {
        gScene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 0.0f);
    }
    else
    {
        gScene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
        gScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 2.0f);
    }
    _outputDebugLines = !_outputDebugLines;
}

void PhysicsWorld::Update(f32 timeStep)
{
    gScene->simulate(1.0f / 60.0f);
    gScene->fetchResults(true);

    // This should only ever return dynamic actors. Statics and Kinematics are user controlled
    // retrieve array of actors that moved
    PxU32 nbActiveActors;
    PxActor** activeActors = gScene->getActiveActors(nbActiveActors);

    // update each render object with the new transform
    for (PxU32 i = 0; i < nbActiveActors; ++i)
    {
        auto rigidActor = activeActors[i]->is<PxRigidActor>();
        if (rigidActor)
        {
            GameObject* gameObject = static_cast<GameObject*>(rigidActor->userData);
            PxTransform transform = rigidActor->getGlobalPose();

            gameObject->GetTransform().SetTransform(ToVec3(transform.p), ToQuat(transform.q));
        }
    }

    if (_outputDebugLines)
    {
        const PxRenderBuffer& rb = gScene->getRenderBuffer();
        for (PxU32 i = 0; i < rb.getNbLines(); i++)
        {
            const PxDebugLine& line = rb.getLines()[i];
            graphics::AddDebugLine(vec3(line.pos0.x, line.pos0.y, line.pos0.z),
                                   vec3(line.pos1.x, line.pos1.y, line.pos1.z),
                                   Color(line.color0 >> 16 & 0xFF, line.color0 >> 8 & 0xFF,
                                         line.color0 >> 0 & 0xFF, line.color0 >> 24 & 0xFF));
            // render the line
        }
    }
}

bool PhysicsWorld::RayCast(vec3 origin, vec3 unitDir)
{
    PxRaycastBuffer hitInfo;
    PxU32 maxHits = 1;
    PxHitFlags hitFlags = PxHitFlag::eDEFAULT;

    bool status = gScene->raycast(ToPxVec3(origin), ToPxVec3(unitDir), 1e6, hitInfo);

    if (status)
        SDL_Log("HIT!");
    else
        SDL_Log("NO HIT");

    return status;
}

void PhysicsWorld::CookModel(graphics::Model* model)
{
    PxTriangleMesh** cachedMesh = gPhysicsMeshManager.Exists(model->id);
    if (cachedMesh)
        return;

    PxTolerancesScale scale;
    PxCookingParams params(scale);
    // disable mesh cleaning - perform mesh validation on development configurations
    params.meshPreprocessParams |= PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH;
    // disable edge precompute, edges are set for each triangle, slows contact generation
    params.meshPreprocessParams |= PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE;

    gCooking->setParams(params);

    // ToDo: Not only one mesh!
    auto& mesh = model->meshes[0];
    // Copy over data and indices

    PxTriangleMeshDesc meshDesc;
    meshDesc.flags |= PxMeshFlag::e16_BIT_INDICES;

    meshDesc.points.count = (u32)mesh.vertexCount;
    meshDesc.points.data = mesh.data;
    meshDesc.points.stride = (u32)mesh.stride;
    meshDesc.triangles.count = (u32)mesh.count / 3;
    meshDesc.triangles.data = mesh.indices;
    meshDesc.triangles.stride = (u32)3 * sizeof(PxU16);

    Assert(meshDesc.isValid());

    PxTriangleMesh* aTriangleMesh =
        gCooking->createTriangleMesh(meshDesc, gPhysics->getPhysicsInsertionCallback());

    gPhysicsMeshManager.LoadOrGet(model->id, aTriangleMesh);
}

void PhysicsWorld::Shutdown()
{
    gScene->release();
    gDispatcher->release();
    gPhysics->release();
    gCooking->release();
    PxPvdTransport* transport = gPvd->getTransport();
    gPvd->release();
    transport->release();

    gFoundation->release();
}

void PhysicsWorld::AddModel(GameObject& obj)
{
    auto triangleMesh = gPhysicsMeshManager.Exists(obj.GetModelId());
    Assert(triangleMesh);

    auto model = gManagers->ModelManager->Exists(obj.GetModelId());
    Assert(model);
    mat4 worldMatrix = obj.GetTransform().GetModelMatrix() * model->meshes[0].localTransform;

    vec3 scale, translation, skew;
    vec4 perspective;
    glm::quat orientation;
    glm::decompose(worldMatrix, scale, orientation, translation, skew, perspective);
    orientation = glm::conjugate(orientation);
    PxTriangleMeshGeometry geom(*triangleMesh, PxMeshScale(ToPxVec3(scale)));

    auto shape = gPhysics->createShape(geom, *gMaterial);

    /*auto capsule = ;
    PxRigidDynamic* dynamic =
        PxCreateDynamic(*gPhysics, PxTransform(ToPxVec3(translation), ToPxQuat(orientation)),
                        capsule, *gMaterial, 10.f);
    dynamic->userData = &obj;*/

    PxRigidDynamic* dynamic =
        gPhysics->createRigidDynamic(PxTransform(ToPxVec3(translation), ToPxQuat(orientation)));
    PxTransform relativePose(PxVec3(0, 0.5f, 0));
    PxShape* aCapsuleShape =
        PxRigidActorExt::createExclusiveShape(*dynamic, PxBoxGeometry(0.7f, 0.4f, 0.3f), *gMaterial);
    aCapsuleShape->setLocalPose(relativePose);
    PxRigidBodyExt::updateMassAndInertia(*dynamic, 1);
    dynamic->userData = &obj;
    gScene->addActor(*dynamic);
    lastDynamic = dynamic;

    // Static code

    /*PxRigidDynamic* duckMesh =
        gPhysics->createRigidStatic(PxTransform(ToPxVec3(translation), ToPxQuat(orientation)));
    Assert(duckMesh);
    duckMesh->userData = &obj;
    duckMesh->attachShape(*shape);

    shape->release();
    gScene->addActor(*duckMesh);*/
}

void PhysicsWorld::AddForce(vec3 dir, float strength)
{
    lastDynamic->addForce(ToPxVec3(dir) * strength);
}
}  // namespace DG
