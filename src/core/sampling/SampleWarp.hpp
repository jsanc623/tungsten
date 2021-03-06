#ifndef SAMPLE_HPP_
#define SAMPLE_HPP_

#include "math/MathUtil.hpp"
#include "math/Angle.hpp"
#include "math/Vec.hpp"

#include <algorithm>
#include <cmath>

namespace Tungsten {

namespace SampleWarp {

static inline Vec3f uniformHemisphere(const Vec2f &uv)
{
    float phi  = TWO_PI*uv.x();
    float r = std::sqrt(max(1.0f - uv.y()*uv.y(), 0.0f));
    return Vec3f(std::cos(phi)*r, std::sin(phi)*r, uv.y());
}

static inline float uniformHemispherePdf(const Vec3f &/*p*/)
{
    return INV_TWO_PI;
}

static inline Vec3f cosineHemisphere(const Vec2f &uv)
{
    float phi = uv.x()*TWO_PI;
    float r = std::sqrt(uv.y());

    return Vec3f(
            std::cos(phi)*r,
            std::sin(phi)*r,
            std::sqrt(max(1.0f - uv.y(), 0.0f))
    );
}

static inline float cosineHemispherePdf(const Vec3f &p)
{
    return p.z()*INV_PI;
}

static inline Vec3f uniformDisk(const Vec2f &uv)
{
    float phi = uv.x()*TWO_PI;
    float r = std::sqrt(uv.y());
    return Vec3f(std::cos(phi)*r, std::sin(phi)*r, 0.0f);
}

static inline float uniformDiskPdf()
{
    return INV_PI;
}

static inline Vec3f uniformCylinder(Vec2f &uv)
{
    float phi = uv.x()*TWO_PI;
    return Vec3f(
        std::sin(phi),
        std::cos(phi),
        uv.y()*2.0f - 1.0f
    );
}

static inline float uniformCylinderPdf()
{
    return INV_PI;
}

static inline Vec3f uniformSphere(const Vec2f &uv)
{
    float phi = uv.x()*TWO_PI;
    float z = uv.y()*2.0f - 1.0f;
    float r = std::sqrt(max(1.0f - z*z, 0.0f));

    return Vec3f(
        std::cos(phi)*r,
        std::sin(phi)*r,
        z
    );
}

static inline float uniformSpherePdf()
{
    return INV_FOUR_PI;
}

static inline Vec3f uniformSphericalCap(const Vec2f &uv, float cosThetaMax)
{
    float phi = uv.x()*TWO_PI;
    float z = uv.y()*(1.0f - cosThetaMax) + cosThetaMax;
    float r = std::sqrt(max(1.0f - z*z, 0.0f));
    return Vec3f(
        std::sin(phi)*r,
        std::cos(phi)*r,
        z
    );
}

static inline float uniformSphericalCapPdf(float cosThetaMax)
{
    return INV_TWO_PI/(1.0f - cosThetaMax);
}

static inline Vec3f phongHemisphere(const Vec2f &uv, int n)
{
    float phi = uv.x()*TWO_PI;
    float cosTheta = std::pow(uv.y(), 1.0f/(n + 1.0f));
    float r = std::sqrt(std::max(1.0f - cosTheta*cosTheta, 0.0f));
    return Vec3f(std::sin(phi)*r, std::cos(phi)*r, cosTheta);
}

static inline float phongHemispherePdf(const Vec3f &v, int n)
{
    return INV_TWO_PI*(n + 1.0f)*std::pow(v.z(), n);
}

static inline Vec3f uniformTriangle(const Vec2f &uv, const Vec3f& a, const Vec3f& b, const Vec3f& c)
{
    float uSqrt = std::sqrt(uv.x());
    float alpha = 1.0f - uSqrt;
    float beta = (1.0f - uv.y())*uSqrt;

    return a*alpha + b*beta + c*(1.0f - alpha - beta);
}

static inline float uniformTrianglePdf(const Vec3f& a, const Vec3f& b, const Vec3f& c)
{
    return 2.0f/((b - a).cross(c - a).length());
}

static inline float powerHeuristic(float pdf0, float pdf1)
{
    return (pdf0*pdf0)/(pdf0*pdf0 + pdf1*pdf1);
}

}

}

#endif /* SAMPLE_HPP_ */
