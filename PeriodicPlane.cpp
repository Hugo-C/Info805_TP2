//
// Created by hugo on 19/03/19.
//

#include "PeriodicPlane.h"
#include <cmath>
#include <limits>

rt::PeriodicPlane::PeriodicPlane(rt::Point3 c, rt::Vector3 u, rt::Vector3 v, rt::Material main_m, rt::Material band_m,
                                 rt::Real w) : GraphicalObject(), c(c), u(u), v(v), band_width(w),
                                               material_band(band_m), material_main(main_m){
    float big = 2.f;
    Vector3 bigU = u * big;
    Vector3 bigV = v * big;

    std::cout << -bigU[0] + bigV[0] + c[0] << ", " <<  -bigU[1] + bigV[1] + c[1] << ", " <<  -bigU[2] + bigV[2] + c[2] << std::endl;

}

void rt::PeriodicPlane::coordinates(rt::Point3 p, rt::Real& x, rt::Real& y) {
    x = u.dot(p);
    y = v.dot(p);
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
    return u.cross(v).norm();
}

rt::Material rt::PeriodicPlane::getMaterial(rt::Point3 p) {
    Real x, y;
    this->coordinates(p, x, y);

    float d_x = static_cast<float>(fmod(x, 1.f));
    if(d_x > 0.5f)
        d_x -= 0.5f;
    float d_y = static_cast<float>(fmod(y, 1.f));
    if(d_y > 0.5f)
        d_y -= 0.5f;

    if(d_x < band_width && d_y < band_width)
        return material_band;
    return material_main;
}

rt::Real rt::PeriodicPlane::rayIntersection(const rt::Ray& ray, rt::Point3& p) {
    float epsilon = 0.0001f; //std::numeric_limits<float>::epsilon();
    Vector3 n = getNormal(Point3());
    Real c = n.dot(ray.direction);
    Real d = (this->c - ray.origin).dot(n);

    if(abs(c) <= epsilon){
        if(abs(d) <= epsilon){
            p = ray.origin;
            return - 0.1f;
        }
        return 1.f;  // no intersection
    }

    Real gamma = d / c;
    if(gamma <= epsilon)
        return 1.f;  // no intersection, the ray is pointing the other way

    p = ray.origin + gamma * ray.direction;
    return -1.f;
}
