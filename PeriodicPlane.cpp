#include <cmath>

#include "PeriodicPlane.h"
#include <cmath>
#include <limits>

rt::PeriodicPlane::PeriodicPlane(rt::Point3 _c, rt::Vector3 _u, rt::Vector3 _v, rt::Material _main_m, rt::Material _band_m,
                                 rt::Real w) : c(_c), u(_u), v(_v), band_width(w),
                                               material_band(_band_m), material_main(_main_m){}

void rt::PeriodicPlane::coordinates(rt::Point3 p, rt::Real& x, rt::Real& y) {
    auto uNormalized = u / u.norm();
    auto vNormalized = v / v.norm();
    x = uNormalized.dot(p);
    y = vNormalized.dot(p);
}

void rt::PeriodicPlane::draw(rt::Viewer& /* viewer */) {
    float big = 200.f;
    Vector3 bigU = u * big;
    Vector3 bigV = v * big;

    glBegin(GL_QUADS);
    glColor4fv( material_main.ambient );
    glMaterialfv(GL_FRONT, GL_DIFFUSE, material_main.diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, material_main.specular);
    glMaterialf(GL_FRONT, GL_SHININESS, material_main.shinyness );
    glVertex3f(bigU[0] + bigV[0] + c[0], bigU[1] + bigV[1] + c[1], bigU[2] + bigV[2] + c[2]);
    glVertex3f(bigU[0] - bigV[0] + c[0], bigU[1] - bigV[1] + c[1], bigU[2] - bigV[2] + c[2]);
    glVertex3f(-bigU[0] - bigV[0] + c[0], -bigU[1] - bigV[1] + c[1], -bigU[2] - bigV[2] + c[2]);
    glVertex3f(-bigU[0] + bigV[0] + c[0], -bigU[1] + bigV[1] + c[1], -bigU[2] + bigV[2] + c[2]);

    glEnd();
}

rt::Vector3 rt::PeriodicPlane::getNormal(rt::Point3 /* p */) {
    auto n = u.cross(v);
    return n / n.norm();
}

rt::Material rt::PeriodicPlane::getMaterial(rt::Point3 p) {
    Real x, y;
    this->coordinates(p, x, y);

    // on récupère les entiers les plus proches et on fais la différence
    int closest_x = static_cast<int>(round(x));
    float d_x = std::fabs(closest_x - x);
    int closest_y = static_cast<int>(round(y));
    float d_y = std::fabs(closest_y - y);

    // si le point est proche de la grille de valeur entière on renvoie le matériaux correspondant
    if(d_x < band_width || d_y < band_width)
        return material_band;
    return material_main;
}

rt::Real rt::PeriodicPlane::rayIntersection(const rt::Ray& ray, rt::Point3& p) {
    float epsilon = std::numeric_limits<float>::epsilon();
    Vector3 n = getNormal(Point3());
    Real c = n.dot(ray.direction);
    Real d = (this->c - ray.origin).dot(n);

    if(fabs(c) <= epsilon){
        if(fabs(d) <= epsilon){
            p = ray.origin;
            return -1.0f;
        }
        return 1.f;  // no intersection
    }

    Real gamma = d / c;

    if(gamma <= epsilon)
        return 1.f; // no intersection, the ray is pointing the other way

    p = ray.origin + gamma * ray.direction;
    return -1.f;
}
