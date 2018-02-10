/**
 *  @file    DG_Physics.cpp
 *  @author  Faaux (github.com/Faaux)
 *  @date    03 February 2018
 */

#include "Physics.h"
#include "GraphicsSystem.h"
#include "ResourceManager.h"
#include "PxPhysicsAPI.h"
#include "main.h"

namespace DG
{
using namespace physx;

struct PhysXScene
{
    PxDefaultCpuDispatcher* Dispatcher = nullptr;
    PxScene* Scene = nullptr;
};

class PhysicsMeshManager : public ResourceManager<PxTriangleMesh*>
{
   public:
    void LoadOrGet(StringId id, PxTriangleMesh* p) { Register(id, p); }
};

static std::unordered_map<PhysicsWorld*, PhysXScene> WorldToPhysX;

static PhysicsMeshManager gPhysicsMeshManager;

PxDefaultAllocator gAllocator;
PxDefaultErrorCallback gErrorCallback;
PxFoundation* gFoundation = nullptr;
PxPhysics* gPhysics = nullptr;
PxCooking* gCooking;
PxPvd* gPvd = nullptr;
PxMaterial* gMaterial = nullptr;

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

bool PhysicsWorld::Init(const Clock& clock)
{
    _clock = &clock;

    // Create a physics scene
    PhysXScene& newScene = WorldToPhysX[this];
    PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
    newScene.Dispatcher = PxDefaultCpuDispatcherCreate(2);
    sceneDesc.cpuDispatcher = newScene.Dispatcher;
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;
    sceneDesc.flags |= PxSceneFlag::eENABLE_ACTIVE_ACTORS;
    sceneDesc.flags |= PxSceneFlag::eEXCLUDE_KINEMATICS_FROM_ACTIVE_ACTORS;
    newScene.Scene = gPhysics->createScene(sceneDesc);

    // Attach to PhysX Visual Debugger
    PxPvdSceneClient* pvdClient = newScene.Scene->getScenePvdClient();
    if (pvdClient)
    {
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
    }

    //ToDo: Remove, no actor should be added to the scene, let the user (world edit) take care of it
    PxRigidStatic* groundPlane = PxCreatePlane(*gPhysics, PxPlane(0, 1, 0, 0), *gMaterial);
    newScene.Scene->addActor(*groundPlane);

    return true;
}

void PhysicsWorld::ToggleDebugVisualization()
{
    auto scene = WorldToPhysX[this].Scene;
    if (_outputDebugLines)
    {
        scene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 0.0f);
    }
    else
    {
        scene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
        scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 2.0f);
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
    PxU32 nbActiveActors;
    PxActor** activeActors = scene->getActiveActors(nbActiveActors);

    // update each render object with the new transform
    for (PxU32 i = 0; i < nbActiveActors; ++i)
    {
        auto rigidActor = activeActors[i]->is<PxRigidActor>();
        if (rigidActor)
        {
            /*GameObject* gameObject = static_cast<GameObject*>(rigidActor->userData);
            PxTransform transform = rigidActor->getGlobalPose();

            gameObject->GetTransform().Set(ToVec3(transform.p), ToQuat(transform.q));*/
        }
    }

    if (_outputDebugLines)
    {
        const PxRenderBuffer& rb = scene->getRenderBuffer();
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

void* PhysicsWorld::RayCast(vec3 origin, vec3 unitDir)
{
    auto scene = WorldToPhysX[this].Scene;
    PxRaycastBuffer hitInfo;
    PxU32 maxHits = 1;
    PxHitFlags hitFlags = PxHitFlag::eDEFAULT;
    PxQueryFilterData filterData(PxQueryFlag::eSTATIC);

    bool status =
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
//
//void PhysicsWorld::AddModel(GameObject& obj, bool forEditing)
//{
//    auto scene = WorldToPhysX[this].Scene;
//
//    auto model = gManagers->ModelManager->Exists(obj.Renderable->RenderableId);
//    Assert(model);
//
//    if (forEditing)
//    {
//        // Disassemble local matrix for model
//        mat4 worldMatrix = obj.GetTransform().GetModelMatrix() * model->meshes[0].localTransform;
//
//        vec3 scale, translation, skew;
//        vec4 perspective;
//        glm::quat orientation;
//        glm::decompose(worldMatrix, scale, orientation, translation, skew, perspective);
//        orientation = glm::conjugate(orientation);
//
//        // Get Mesh
//        auto triangleMesh = gPhysicsMeshManager.Exists(obj.Renderable->RenderableId);
//        if (!triangleMesh)
//        {
//            SDL_LogWarn(0, "Runtime cooking for EDITOR Mesh");
//            CookModel(gManagers->ModelManager->Exists(obj.Renderable->RenderableId));
//            triangleMesh = gPhysicsMeshManager.Exists(obj.Renderable->RenderableId);
//        }
//        Assert(triangleMesh);
//
//        // Create Geometry and then Shape
//        PxTriangleMeshGeometry geom(*triangleMesh, PxMeshScale(ToPxVec3(scale)));
//        PxShape* shape = gPhysics->createShape(geom, *gMaterial);
//
//        // Create Rigid Static
//        PxRigidStatic* staticActor =
//            gPhysics->createRigidStatic(PxTransform(ToPxVec3(translation), ToPxQuat(orientation)));
//
//        staticActor->userData = &obj;
//        staticActor->attachShape(*shape);
//        shape->release();
//        scene->addActor(*staticActor);
//
//        // Add to gameobject
//        obj.Physics = new PhysicsComponent();
//        obj.Physics->Data = staticActor;
//        obj.Physics->Type = PhysicsComponent::Type::Static;
//    }
//    else
//    {
//        auto& objectTransform = obj.GetTransform();
//        const auto translation = ToPxVec3(objectTransform.GetPosition());
//        const auto orientation = ToPxQuat(objectTransform.GetOrientation());
//        // Create Actor
//        PxRigidDynamic* dynamic =
//            gPhysics->createRigidDynamic(PxTransform(translation, orientation));
//
//        // Attach Shape
//        PxShape* dynamicShape = PxRigidActorExt::createExclusiveShape(
//            *dynamic, PxBoxGeometry(0.7f, 0.4f, 0.3f), *gMaterial);
//        PxTransform relativePose(PxVec3(0, 0.5f, 0));
//        dynamicShape->setLocalPose(relativePose);
//
//        // Update with new shape
//        PxRigidBodyExt::updateMassAndInertia(*dynamic, 1);
//
//        // Add to scene
//        dynamic->userData = &obj;
//        scene->addActor(*dynamic);
//
//        // Add to gameobject
//        obj.Physics = new PhysicsComponent();
//        obj.Physics->Data = dynamic;
//        obj.Physics->Type = PhysicsComponent::Type::Dynamic;
//    }
//}
//
//void PhysicsWorld::RemoveModel(GameObject& obj)
//{
//    auto scene = WorldToPhysX[this].Scene;
//    PxRigidBody* rigidBody = (PxRigidBody*)obj.Physics->Data;
//    rigidBody->release();
//
//    delete obj.Physics;
//    obj.Physics = nullptr;
//}

bool InitPhysics()
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
    PxPvdTransport* transport = gPvd->getTransport();
    gPvd->release();
    transport->release();

    gFoundation->release();
    return true;
}

void CookModel(graphics::GraphicsModel* model)
{
    Assert(model);
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
}  // namespace DG
