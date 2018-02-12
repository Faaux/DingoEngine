/**
 *  @file    Physics.cpp
 *  @author  Faaux (github.com/Faaux)
 *  @date    03 February 2018
 */

#include "Physics.h"
#include <PxPhysicsAPI.h>
#include <unordered_map>
#include "platform/ResourceManager.h"

namespace DG
{
struct PhysXScene
{
    physx::PxDefaultCpuDispatcher* Dispatcher = nullptr;
    physx::PxScene* Scene = nullptr;
};

class PhysicsMeshManager : public ResourceManager<physx::PxTriangleMesh*>
{
   public:
    void LoadOrGet(StringId id, physx::PxTriangleMesh* p) { Register(id, p); }
};

static std::unordered_map<PhysicsWorld*, PhysXScene> WorldToPhysX;

static PhysicsMeshManager gPhysicsMeshManager;

physx::PxDefaultAllocator gAllocator;
physx::PxDefaultErrorCallback gErrorCallback;
physx::PxFoundation* gFoundation = nullptr;
physx::PxPhysics* gPhysics = nullptr;
physx::PxCooking* gCooking;
physx::PxPvd* gPvd = nullptr;
physx::PxMaterial* gMaterial = nullptr;

static physx::PxQuat ToPxQuat(const glm::quat& q)
{
    physx::PxQuat quat;
    quat.w = q.w;
    quat.x = q.x;
    quat.y = q.y;
    quat.z = q.z;
    return quat;
}

static physx::PxVec3 ToPxVec3(const vec3& v)
{
    physx::PxVec3 result(v.x, v.y, v.z);
    return result;
}

static glm::quat ToQuat(const physx::PxQuat& q)
{
    glm::quat quat;
    quat.w = q.w;
    quat.x = q.x;
    quat.y = q.y;
    quat.z = q.z;
    return quat;
}

static vec3 ToVec3(const physx::PxVec3& v)
{
    vec3 result(v.x, v.y, v.z);
    return result;
}

bool PhysicsWorld::Init(const Clock& clock)
{
    _clock = &clock;

    // Create a physics scene
    PhysXScene& newScene = WorldToPhysX[this];
    physx::PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
    sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
    newScene.Dispatcher = physx::PxDefaultCpuDispatcherCreate(2);
    sceneDesc.cpuDispatcher = newScene.Dispatcher;
    sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
    sceneDesc.flags |= physx::PxSceneFlag::eENABLE_ACTIVE_ACTORS;
    sceneDesc.flags |= physx::PxSceneFlag::eEXCLUDE_KINEMATICS_FROM_ACTIVE_ACTORS;
    newScene.Scene = gPhysics->createScene(sceneDesc);

    // Attach to PhysX Visual Debugger
    physx::PxPvdSceneClient* pvdClient = newScene.Scene->getScenePvdClient();
    if (pvdClient)
    {
        pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
        pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
        pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
    }

    // ToDo: Remove, no actor should be added to the scene, let the user (world edit) take care of
    // it
    physx::PxRigidStatic* groundPlane =
        PxCreatePlane(*gPhysics, physx::PxPlane(0, 1, 0, 0), *gMaterial);
    newScene.Scene->addActor(*groundPlane);

    return true;
}

void PhysicsWorld::ToggleDebugVisualization()
{
    auto scene = WorldToPhysX[this].Scene;
    if (_outputDebugLines)
    {
        scene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE, 0.0f);
    }
    else
    {
        scene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE, 1.0f);
        scene->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_SHAPES, 2.0f);
    }
    _outputDebugLines = !_outputDebugLines;
}

void PhysicsWorld::Update()
{
    auto scene = WorldToPhysX[this].Scene;
    static float timeAccumulator = 0;
    timeAccumulator += _clock->GetLastDtSeconds();

    const float physicsTimeStep = 1.0f / 120.0f;
    while (timeAccumulator > physicsTimeStep)
    {
        timeAccumulator -= physicsTimeStep;
        scene->simulate(physicsTimeStep);
        scene->fetchResults(true);
    }

    // This should only ever return dynamic actors. Statics and Kinematics are user controlled
    // retrieve array of actors that moved
    physx::PxU32 nbActiveActors;
    physx::PxActor** activeActors = scene->getActiveActors(nbActiveActors);

    // update each render object with the new transform
    for (physx::PxU32 i = 0; i < nbActiveActors; ++i)
    {
        auto rigidActor = activeActors[i]->is<physx::PxRigidActor>();
        if (rigidActor)
        {
            /*GameObject* gameObject = static_cast<GameObject*>(rigidActor->userData);
            PxTransform transform = rigidActor->getGlobalPose();

            gameObject->GetTransform().Set(ToVec3(transform.p), ToQuat(transform.q));*/
        }
    }

    if (_outputDebugLines)
    {
        const physx::PxRenderBuffer& rb = scene->getRenderBuffer();
        for (physx::PxU32 i = 0; i < rb.getNbLines(); i++)
        {
            const physx::PxDebugLine& line = rb.getLines()[i];
            graphics::AddDebugLine(vec3(line.pos0.x, line.pos0.y, line.pos0.z),
                                   vec3(line.pos1.x, line.pos1.y, line.pos1.z),
                                   Color(line.color0 >> 16 & 0xFF, line.color0 >> 8 & 0xFF,
                                         line.color0 >> 0 & 0xFF, line.color0 >> 24 & 0xFF));
            // render the line
        }
    }
}

void* PhysicsWorld::RayCast(vec3 origin, vec3 unitDir)
{
    const auto scene = WorldToPhysX[this].Scene;
    physx::PxRaycastBuffer hitInfo;
    physx::PxU32 maxHits = 1;
    physx::PxHitFlags hitFlags = physx::PxHitFlag::eDEFAULT;
    physx::PxQueryFilterData filterData(physx::PxQueryFlag::eSTATIC);

    const bool status =
        scene->raycast(ToPxVec3(origin), ToPxVec3(unitDir), 1e6, hitInfo, hitFlags, filterData);

    if (status)
        return hitInfo.getAnyHit(0).actor;

    return 0;
}

void PhysicsWorld::Shutdown()
{
    auto scene = WorldToPhysX[this];
    scene.Scene->release();
    scene.Dispatcher->release();
    WorldToPhysX.erase(this);
}

bool InitPhysics()
{
    gFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, gAllocator, gErrorCallback);

    gCooking = PxCreateCooking(PX_PHYSICS_VERSION, *gFoundation,
                               physx::PxCookingParams(physx::PxTolerancesScale()));

    // PhysX Visual Debugger
    gPvd = PxCreatePvd(*gFoundation);
    physx::PxPvdTransport* transport =
        physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
    gPvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);

    // Init phsyics
    gPhysics =
        PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, physx::PxTolerancesScale(), true, gPvd);

    gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);
    return true;
}

bool ShutdownPhysics()
{
    Assert(WorldToPhysX.size() <= 1);
    for (auto& pair : WorldToPhysX)
    {
        pair.second.Scene->release();
        pair.second.Dispatcher->release();
    }
    gPhysics->release();
    gCooking->release();
    physx::PxPvdTransport* transport = gPvd->getTransport();
    gPvd->release();
    transport->release();

    gFoundation->release();
    return true;
}

void CookModel(graphics::GraphicsModel* model)
{
    Assert(model);
    physx::PxTriangleMesh** cachedMesh = gPhysicsMeshManager.Exists(model->id);
    if (cachedMesh)
        return;

    physx::PxTolerancesScale scale;
    physx::PxCookingParams params(scale);
    // disable mesh cleaning - perform mesh validation on development configurations
    params.meshPreprocessParams |= physx::PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH;
    // disable edge precompute, edges are set for each triangle, slows contact generation
    params.meshPreprocessParams |= physx::PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE;

    gCooking->setParams(params);

    // ToDo: Not only one mesh!
    auto& mesh = model->meshes[0];
    // Copy over data and indices

    physx::PxTriangleMeshDesc meshDesc;
    meshDesc.flags |= physx::PxMeshFlag::e16_BIT_INDICES;

    meshDesc.points.count = (u32)mesh.vertexCount;
    meshDesc.points.data = mesh.data;
    meshDesc.points.stride = (u32)mesh.stride;
    meshDesc.triangles.count = (u32)mesh.count / 3;
    meshDesc.triangles.data = mesh.indices;
    meshDesc.triangles.stride = (u32)3 * sizeof(physx::PxU16);

    Assert(meshDesc.isValid());

    physx::PxTriangleMesh* aTriangleMesh =
        gCooking->createTriangleMesh(meshDesc, gPhysics->getPhysicsInsertionCallback());

    gPhysicsMeshManager.LoadOrGet(model->id, aTriangleMesh);
}
}  // namespace DG
