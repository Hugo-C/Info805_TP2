
#ifndef TP2_PERIODICPLANE_H
#define TP2_PERIODICPLANE_H

/// Namespace RayTracer
#include "GraphicalObject.h"

namespace rt {
    struct PeriodicPlane : public GraphicalObject {


        /// The origin of the plane
        Point3 c;

        /// Vecteurs orthogonaux tangents
        Vector3 u;
        Vector3 v;

        Real band_width;  // width where material_band will be used
        Material material_band;
        Material material_main;

        /// Creates a periodic infinite plane passing through \a c and
        /// tangent to \a u and \a v. Then \a w defines the width of the
        /// band around (0,0) and its period to put material \a band_m,
        /// otherwise \a main_m is used.
        PeriodicPlane(Point3 _c, Vector3 _u, Vector3 _v,
                      Material _main_m, Material _band_m, Real w);

        /// Etant donné un point \a p dans le plan, retourne ses coordonnées \a x et \a y selon \a u et \a v.
        void coordinates(Point3 p, Real& x, Real& y);

        // ---------------- GraphicalObject services ----------------------------

        /// This method is called by Scene::init() at the beginning of the
        /// display in the OpenGL window. May be useful for some
        /// precomputations.
        void init(Viewer& /* viewer */ ) override {}

        /// This method is called by Scene::draw() at each frame to
        /// redisplay objects in the OpenGL window.
        void draw(Viewer& viewer) override;


        /// @return the normal vector at point \a p on the sphere (\a p
        /// should be on or close to the sphere).
        Vector3 getNormal(Point3 p) override;

        /// @return the material associated to this part of the object
        Material getMaterial(Point3 p) override;

        /// @param[in] ray the incoming ray
        /// @param[out] returns the point of intersection with the object
        /// (if any), or the closest point to it.
        ///
        /// @return either a real < 0.0 if there is an intersection, or a
        /// kind of distance to the closest point of intersection.
        Real rayIntersection(const Ray& ray, Point3& p) override;
    };
}

#endif //TP2_PERIODICPLANE_H
