#pragma once
#include "DG_Include.h"
#include "DG_Transform.h"
#include "gl/glew.h"

namespace DG
{
class GraphicsSystem
{
   public:
    GraphicsSystem();
};

class DebugDrawManager
{
   public:
    void AddLine(const vec3 &fromPosition, const vec3 &toPosition, Color color,
                 r32 lineWidth = 1.0f, float durationSeconds = 0.0f, bool depthEnabled = true);

    void AddCross(const vec3 &position, Color color, r32 size = 1.0f, float durationSeconds = 0.0f,
                  bool depthEnabled = true);

    void AddSphere(const vec3 &centerPosition, Color color, r32 radius = 1.0f,
                   float durationSeconds = 0.0f, bool depthEnabled = true);

    void AddCircle(const vec3 &centerPosition, const vec3 &planeNormal, Color color,
                   r32 radius = 1.0f, float durationSeconds = 0.0f, bool depthEnabled = true);

    void AddAxes(const Transform &transform, Color color, r32 size = 1.0f,
                 float durationSeconds = 0.0f, bool depthEnabled = true);

    void AddTriangle(const vec3 &vertex0, const vec3 &vertex1, const vec3 &vertex2, Color color,
                     r32 lineWidth = 1.0f, float durationSeconds = 0.0f, bool depthEnabled = true);

    void AddAABB(const vec3 &minCoords, const vec3 &maxCoords, Color color, r32 lineWidth = 1.0f,
                 float durationSeconds = 0.0f, bool depthEnabled = true);

    void AddString(const vec3 &position, const char *text, Color color,
                   float durationSeconds = 0.0f, bool depthEnabled = true);
};

extern DebugDrawManager g_DebugDrawManager;
}  // namespace DG
