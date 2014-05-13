#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable
__kernel void applyGravity(__global float* particles,
                           __global float* forces,
                           float3 gravity)
{
    size_t gId = get_global_id(0);
    size_t pId = gId * 7;
    size_t fId = gId * 3;
    float3 f = (float3)(gravity.x, gravity.y, gravity.z);

    f *= particles[pId];

    forces[fId] = f.x;
    forces[fId + 1] = f.y;
    forces[fId + 2] = f.z;
}

__kernel void computeSpringDamper(__global float* particles,
                                  __global int* SPIndexes,
                                  __global float* SPs,
                                  __global float* forces)
{
    size_t gId = get_global_id(0);
    size_t SPId = gId * 3;
    size_t id1 = SPIndexes[gId * 2];
    size_t id2 = SPIndexes[gId * 2 + 1];
    size_t pId = id1 * 7;
    size_t idPart2 = id2 * 7;
    size_t idForce1 = id1 * 3;
    size_t idForce2 = id2 * 3;

    float3 p1 = (float3)(particles[pId + 1], particles[pId + 2], particles[pId + 3]);
    float3 v1 = (float3)(particles[pId + 4], particles[pId + 5], particles[pId + 6]);
    float3 p2 = (float3)(particles[idPart2 + 1], particles[idPart2 + 2], particles[idPart2 + 3]);
    float3 v2 = (float3)(particles[idPart2 + 4], particles[idPart2 + 5], particles[idPart2 + 6]);
    float restLength = SPs[SPId + 0];
    float dampingFactor = SPs[SPId + 1];
    float springConstant = SPs[SPId + 2];
    float3 e = p2 - p1;
    float l = length(e);
    e = normalize(e);
    float vd1 = dot(v1, e);
    float vd2 = dot(v2, e);
    float3 f = (-springConstant * (restLength - l) - dampingFactor * (vd1 - vd2)) * e;
    forces[idForce1] = forces[idForce1] + f.x;
    forces[idForce1 + 1] = forces[idForce1 + 1] + f.y;
    forces[idForce1 + 2] = forces[idForce1 + 2] + f.z;
    forces[idForce2] = forces[idForce2] - f.x;
    forces[idForce2 + 1] = forces[idForce2 + 1] - f.y;
    forces[idForce2 + 2] = forces[idForce2 + 2] - f.z;
}

__kernel void computeAerodynamicForces(__global int* faces, __global float* particles, __global float* forces, float3 wind, float density, float dragCoef) {
    size_t gId = get_global_id(0);
    size_t faceId = gId * 3;
    size_t id1 = faces[faceId];
    size_t id2 = faces[faceId + 1];
    size_t id3 = faces[faceId + 2];
    size_t pId = id1 * 7;
    size_t idPart2 = id2 * 7;
    size_t idPart3 = id3 * 7;
    size_t idForce1 = id1 * 3;
    size_t idForce2 = id2 * 3;
    size_t idForce3 = id3 * 3;
    float3 p1 = (float3)(particles[pId + 1], particles[pId + 2], particles[pId + 3]);
    float3 p2 = (float3)(particles[idPart2 + 1], particles[idPart2 + 2], particles[idPart2 + 3]);
    float3 p3 = (float3)(particles[idPart3 + 1], particles[idPart3 + 2], particles[idPart3 + 3]);
    float3 v1 = (float3)(particles[pId + 4], particles[pId + 5], particles[pId + 6]);
    float3 v2 = (float3)(particles[idPart2 + 4], particles[idPart2 + 5], particles[idPart2 + 6]);
    float3 v3 = (float3)(particles[idPart3 + 4], particles[idPart3 + 5], particles[idPart3 + 6]);

    float3 ns = cross(p2 - p1, p3 - p1);
    float3 v = (v1 + v2 + v3) / 3.0f;
    v -= wind;
    float3 f = -density * dragCoef * (length(v) * dot(v, ns) / (12 * length(ns))) * ns;
    forces[idForce1] += f.x;
    forces[idForce1 + 1] += f.y;
    forces[idForce1 + 2] += f.z;
    forces[idForce2] += f.x;
    forces[idForce2 + 1] += f.y;
    forces[idForce2 + 2] += f.z;
    forces[idForce3] += f.x;
    forces[idForce3 + 1] += f.y;
    forces[idForce3 + 2] += f.z;
}

__kernel void integrate(__global float* particles, __global float* forces, float dt)
{
    size_t gId = get_global_id(0);
    size_t pId = gId * 7;
    size_t fId = gId * 3;
    float m = particles[pId];
    float3 p = (float3)(particles[pId + 1], particles[pId + 2], particles[pId + 3]);
    float3 v = (float3)(particles[pId + 4], particles[pId + 5], particles[pId + 6]);
    float3 f = (float3)(forces[fId], forces[fId + 1], forces[fId + 2]);

    if (m > 0) {
        float3 a = f / m;

        v = v + a * dt;
        p = p + v * dt;
        particles[pId + 1] = p.x;
        particles[pId + 2] = p.y;
        particles[pId + 3] = p.z;
        particles[pId + 4] = v.x;
        particles[pId + 5] = v.y;
        particles[pId + 6] = v.z;
    }
}

__kernel void intersect(__global float* particles, float friction, float elasticity, float planeY, float3 sphereCenter, float3 sphereAxe, float sphereRadius, float sphereSpeed) {
    size_t gId = get_global_id(0);
    size_t pId = gId * 7;
    float3 p = (float3)(particles[pId + 1], particles[pId + 2], particles[pId + 3]);
    float3 v = (float3)(particles[pId + 4], particles[pId + 5], particles[pId + 6]);

    if (p.y < planeY) {
        p.y = planeY;
        v.y = -elasticity * v.y;
        v.x = (1.0f - friction) * v.x;
        v.z = (1.0f - friction) * v.z;
    }
    float3 vec = p - sphereCenter;
    if (length(vec) < sphereRadius) {
        float3 n = normalize(vec);
        vec = n * sphereRadius;
        float3 axe = normalize(sphereAxe) * sphereSpeed;
        float3 vs = cross(vec, axe);
        v -= vs;
        float vc = dot(n, v);
        float3 j = -(1.0f + elasticity) * vc * n;
        float3 e = normalize(n * dot(n, v) - v);
        float3 jd = friction * length(j) * e;
        v += j + jd + vs;
        p = sphereCenter + vec;
    }
    particles[pId + 1] = p.x;
    particles[pId + 2] = p.y;
    particles[pId + 3] = p.z;
    particles[pId + 4] = v.x;
    particles[pId + 5] = v.y;
    particles[pId + 6] = v.z;
}
