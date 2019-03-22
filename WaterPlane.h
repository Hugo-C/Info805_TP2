#ifndef TP2_WATERPLANE_H
#define TP2_WATERPLANE_H

#include "PeriodicPlane.h"
namespace rt {

    struct WaveData {
        WaveData(float r, float a, float l, float phi);

        Real r;
        Real a;
        Real l;
        Real phi;
    };

    struct WaterPlane : public rt::PeriodicPlane {

        std::vector<WaveData> myWaves;

        WaterPlane(rt::Point3 _c, rt::Vector3 _u, rt::Vector3 _v, rt::Material _main_m);

        /// @return the normal vector at point \a p on the sphere (\a p
        /// should be on or close to the sphere).
        Vector3 getNormal(Point3 p) override;

        /// @return the material associated to this part of the object
        Material getMaterial(Point3 p) override;
    };
}


#endif //TP2_WATERPLANE_H
