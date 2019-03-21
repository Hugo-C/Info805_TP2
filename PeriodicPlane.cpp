//
// Created by hugo on 19/03/19.
//

#include "PeriodicPlane.h"
#include <cmath>

rt::PeriodicPlane::PeriodicPlane(rt::Point3 c, rt::Vector3 u, rt::Vector3 v, rt::Material main_m, rt::Material band_m,
                                 rt::Real w) : GraphicalObject(), c(c), u(u.norm()), v(v.norm()), band_width(w),
                                               material_band(band_m), material_main(main_m){

}

void rt::PeriodicPlane::coordinates(rt::Point3 p, rt::Real& x, rt::Real& y) {
    x = u.dot(p);
    y = v.dot(p);
}

void rt::PeriodicPlane::draw(rt::Viewer& /* viewer */) {
    Vector3 n = getNormal(Point3());
    float big = 10000.f;
    Vector3 cu = (c + u) * big;
    Vector3 cv = (c + v) * big;
    Vector3 cvu = (c + v + u) * big;

    Material m = material_main;
    // Taking care of north pole
    glBegin( GL_TRIANGLE_FAN );
    glColor4fv( m.ambient );
    glMaterialfv(GL_FRONT, GL_DIFFUSE, m.diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, m.specular);
    glMaterialf(GL_FRONT, GL_SHININESS, m.shinyness );

    glNormal3fv(n);
    glVertex3fv(c);
    glVertex3fv(cv);
    glVertex3fv(cu);

    glVertex3fv(cu);
    glVertex3fv(cv);
    glVertex3fv(cvu);

    glEnd();
}

rt::Vector3 rt::PeriodicPlane::getNormal(rt::Point3 /* p */) {
    return u.cross(v);
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
    float epsilon = 0.00001f;
    Vector3 n = getNormal(Point3());
    Real c = n.dot(ray.direction);
    Real d = (ray.origin - this->c).dot(n);
    if(abs(c - epsilon) <= 0.f){
        if(abs(d - epsilon) <= 0.f){
            p = ray.origin;
            return 0.f;
        }
        return 1.f;  // no intersection
    }

    Real gamma = d / c;
    if(gamma <= 0.f)
        return 1.f;  // no intersection, the ray is pointing the other way

    p = this->c + gamma * ray.direction;
    return -1.f;
}
