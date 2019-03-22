#include "WaterPlane.h"
namespace rt {
    WaveData::WaveData(float r, float a, float l, float phi) : r(r), a(a), l(l), phi(phi) { }

    WaterPlane::WaterPlane(Point3 _c1, Vector3 _u1, Vector3 _v1, Material _main_m1) : PeriodicPlane(_c1, _u1, _v1, _main_m1, Material::black_plastic(), 0.f) {
        myWaves.emplace_back(0.1f, 3.2f, 2.4f, 0.0f);
        myWaves.emplace_back(0.2f, 2.4f, 0.8f, 0.0f);
        myWaves.emplace_back(0.23f, 1.1f, 1.31f, 0.0f);
        myWaves.emplace_back(0.03f, 0.54f, 0.52f, 0.0f);
        myWaves.emplace_back(0.3f, 1.69f, 1.6f, 0.0f);
    }

    Vector3 WaterPlane::getNormal(Point3 p) {
        Real x, y;
        this->coordinates(p, x, y);
        Vector3 n = PeriodicPlane::getNormal(p);
        Real distortion = 0.f;
        for(auto aWave : myWaves){
            Real t = x * cos(aWave.a) + y * sin(aWave.a);
            distortion += aWave.r * cos(2 * M_PI * t / aWave.l + aWave.phi);
        }
        n[2] = distortion;
        return n;
    }

    Material WaterPlane::getMaterial(Point3 /* p */) {
        return material_main;
    }
}