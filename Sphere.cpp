/**
@file Sphere.cpp
*/
#include <cmath>
#include "Sphere.h"

void
rt::Sphere::draw( Viewer& /* viewer */ )
{
  Material m = material;
  // Taking care of south pole
  glBegin( GL_TRIANGLE_FAN );
  glColor4fv( m.ambient );
  glMaterialfv(GL_FRONT, GL_DIFFUSE, m.diffuse);
  glMaterialfv(GL_FRONT, GL_SPECULAR, m.specular);
  glMaterialf(GL_FRONT, GL_SHININESS, m.shinyness );
  Point3 south_pole = localize( -90, 0 );
  glNormal3fv( getNormal( south_pole ) );
  glVertex3fv( south_pole );
  for ( int x = 0; x <= NLON; ++x )
    {
      Point3 p = localize( -90 + 180/NLAT, x * 360 / NLON );
      glNormal3fv( getNormal( p ) );
      glVertex3fv( p );
    }
  glEnd();
  // Taking care of in-between poles
  for ( int y = 1; y < NLAT - 1; ++y )
    {
      glBegin( GL_QUAD_STRIP);
      glColor4fv( m.ambient );
      glMaterialfv(GL_FRONT, GL_DIFFUSE, m.diffuse);
      glMaterialfv(GL_FRONT, GL_SPECULAR, m.specular);
      glMaterialf(GL_FRONT, GL_SHININESS, m.shinyness );
      for ( int x = 0; x <= NLON; ++x )
        {
          Point3 p = localize( -90 + y*180/NLAT,     x * 360 / NLON );
          Point3 q = localize( -90 + (y+1)*180/NLAT, x * 360 / NLON );
          glNormal3fv( getNormal( p ) );
          glVertex3fv( p );
          glNormal3fv( getNormal( q ) );
          glVertex3fv( q );
        }
      glEnd();
    }
  // Taking care of north pole
  glBegin( GL_TRIANGLE_FAN );
  glColor4fv( m.ambient );
  glMaterialfv(GL_FRONT, GL_DIFFUSE, m.diffuse);
  glMaterialfv(GL_FRONT, GL_SPECULAR, m.specular);
  glMaterialf(GL_FRONT, GL_SHININESS, m.shinyness );
  Point3 north_pole = localize( 90, 0 );
  glNormal3fv( getNormal( north_pole ) );
  glVertex3fv( north_pole );
  for ( int x = NLON; x >= 0; --x )
    {
      Point3 p = localize( -90 + (NLAT-1)*180/NLAT, x * 360 / NLON );
      glNormal3fv( getNormal( p ) );
      glVertex3fv( p );
    }
  glEnd();
}

rt::Point3
rt::Sphere::localize( Real latitude, Real longitude ) const
{
  static const Real conv_deg_rad = 2.0 * M_PI / 360.0;
  latitude  *= conv_deg_rad;
  longitude *= conv_deg_rad;
  return center 
    + radius * Point3( cos( longitude ) * cos( latitude ),
                       sin( longitude ) * cos( latitude ),
                       sin( latitude ) );
}

rt::Vector3
rt::Sphere::getNormal( Point3 p )
{
  Vector3 u = p - center;
  Real   l2 = u.dot( u );
  if ( l2 != 0.0 ) u /= sqrt( l2 );
  return u;
}

rt::Material
rt::Sphere::getMaterial( Point3 /* p */ )
{
  return material; // the material is constant along the sphere.
}

rt::Real
rt::Sphere::rayIntersection( const Ray& ray, Point3& p )
{
    Vector3 pc = Vector3(center[0] - ray.origin[0], center[1] - ray.origin[1], center[2] - ray.origin[2]);
    Vector3 pq = pc.dot(ray.direction) * ray.direction;
    Vector3 qc = pc - pq;
    Real distance2 = qc.dot(qc);
    Real radius2 = radius * radius;
    if (distance2 > radius2)
        return 1.0f;  // no intersect with the sphere

    Real b = 2 * (ray.direction.dot(pc));
    Real discriminant = b * b - 4 * (pc.dot(pc) - radius2) * (pc.dot(pc) - radius2);
    if(discriminant < 0.f){
        std::cout << "discriminant is negative : " << discriminant << " ||pc|| : " << pc.dot(pc) << std::endl;
        //return 1.0f;
    }


    Real disSqrt = static_cast<Real>(sqrt(discriminant));

    Real t1 = (-b - disSqrt) / 2.0f;
    Real t2 = (-b + disSqrt) / 2.0f;
    if (t1 < 0.f && t2 < 0.f)
        return 1.0f;  // the ray start after the sphere

    Real t = t1 > 0 ? t1 : t2;
    p = Point3(ray.origin[0] + ray.direction[0] * t,
               ray.origin[1] + ray.direction[1] * t,
               ray.origin[2] + ray.direction[2] * t);
    return -1.0f;
}
