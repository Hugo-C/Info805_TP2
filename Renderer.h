/**
@file Renderer.h
@author JOL
*/
#pragma once
#ifndef _RENDERER_H_
#define _RENDERER_H_

#include <cmath>
#include "Color.h"
#include "Image2D.h"
#include "Ray.h"
#include "Viewer.h"
#include "GraphicalObject.h"
#include "PointVector.h"
#include "Scene.h"
#include <iostream>
#include <string>



/// Namespace RayTracer
namespace rt {

    inline void progressBar(std::ostream& output,
                            const double currentValue, const double maximumValue) {
        static const int PROGRESSBARWIDTH = 60;
        static int myProgressBarRotation = 0;
        static int myProgressBarCurrent = 0;
        // how wide you want the progress meter to be
        double fraction = currentValue / maximumValue;

        // part of the progressmeter that's already "full"
        int dotz = static_cast<int>(floor(fraction * PROGRESSBARWIDTH));
        if (dotz > PROGRESSBARWIDTH) dotz = PROGRESSBARWIDTH;

        // if the fullness hasn't changed skip display
        if (dotz == myProgressBarCurrent) return;
        myProgressBarCurrent = dotz;
        myProgressBarRotation++;

        // create the "meter"
        int ii = 0;
        output << "[";
        // part  that's full already
        for (; ii < dotz; ii++) output << "#";
        // remaining part (spaces)
        for (; ii < PROGRESSBARWIDTH; ii++) output << " ";
        static const char *rotation_string = "|\\-/";
        myProgressBarRotation %= 4;
        output << "] " << rotation_string[myProgressBarRotation]
               << " " << (int) (fraction * 100) << "/100\r";
        output.flush();
    }

    struct Background {
        virtual Color backgroundColor(const Ray& ray) = 0;
    };

    struct MyBackground : public Background {
        Image2D<Color> img;
        MyBackground(Image2D<Color> img) : img(img){}
        Color backgroundColor(const Ray& ray) override {
            Real z = ray.direction.at(2);
            if (z < 0.f) {
                Real x = -0.5f * ray.direction[0] / ray.direction[2];
                Real y = -0.5f * ray.direction[1] / ray.direction[2];
                Real d = sqrt(x * x + y * y);
                Real t = std::min(d, 30.0f) / 30.0f;
                x -= floor(x);
                y -= floor(y);
                if (((x >= 0.5f) && (y >= 0.5f)) || ((x < 0.5f) && (y < 0.5f)))
                    return lerp(Color(0.2f, 0.2f, 0.2f), Color(1.0f, 1.0f, 1.0f), t);
                else
                    return lerp(Color(0.4f, 0.4f, 0.4f), Color(1.0f, 1.0f, 1.0f), t);
            } else{
                int x = static_cast<int>((0.5f + (ray.direction.at(0) / 2)) * img.w());
                int y = static_cast<int>((0.5f + (ray.direction.at(1) / 2)) * img.h());
                return img.at(x, y);
            }
        }
    };



    /// This structure takes care of rendering a scene.
    struct Renderer {

        /// The scene to render
        Scene *ptrScene;
        /// The origin of the camera in space.
        Point3 myOrigin;
        /// (myOrigin, myOrigin+myDirUL) forms a ray going through the upper-left
        /// corner pixel of the viewport, i.e. pixel (0,0)
        Vector3 myDirUL;
        /// (myOrigin, myOrigin+myDirUR) forms a ray going through the upper-right
        /// corner pixel of the viewport, i.e. pixel (width,0)
        Vector3 myDirUR;
        /// (myOrigin, myOrigin+myDirLL) forms a ray going through the lower-left
        /// corner pixel of the viewport, i.e. pixel (0,height)
        Vector3 myDirLL;
        /// (myOrigin, myOrigin+myDirLR) forms a ray going through the lower-right
        /// corner pixel of the viewport, i.e. pixel (width,height)
        Vector3 myDirLR;

        int myWidth;
        int myHeight;

        // On rajoute un pointeur vers un objet Background
        Background *ptrBackground;

        Renderer() : ptrScene(0) {}

        Renderer(Scene& scene, Background *background) : ptrScene(&scene), ptrBackground(background) {}

        void setScene(rt::Scene& aScene) { ptrScene = &aScene; }

        void setViewBox(Point3 origin,
                        Vector3 dirUL, Vector3 dirUR, Vector3 dirLL, Vector3 dirLR) {
            myOrigin = origin;
            myDirUL = dirUL;
            myDirUR = dirUR;
            myDirLL = dirLL;
            myDirLR = dirLR;
        }

        void setResolution(int width, int height) {
            myWidth = width;
            myHeight = height;
        }

        // Affiche les sources de lumières avant d'appeler la fonction qui
        // donne la couleur de fond.
        Color background(const Ray& ray) {
            Color result = Color(0.0, 0.0, 0.0);
            for (Light *light : ptrScene->myLights) {
                Real cos_a = light->direction(ray.origin).dot(ray.direction);
                if (cos_a > 0.99f) {
                    Real a = acos(cos_a) * 360.0 / M_PI / 8.0;
                    a = std::max(1.0f - a, 0.0f);
                    result += light->color(ray.origin) * a * a;
                }
            }
            if (ptrBackground != 0)
                result += ptrBackground->backgroundColor(ray);
            return result;
        }

        /// The main rendering routine
        void render(Image2D<Color>& image, int max_depth) {
            std::cout << "Rendering into image ... might take a while." << std::endl;
            image = Image2D<Color>(myWidth, myHeight);
            for (int y = 0; y < myHeight; ++y) {
                Real ty = (Real) y / (Real) (myHeight - 1);
                progressBar(std::cout, ty, 1.0);
                Vector3 dirL = (1.0f - ty) * myDirUL + ty * myDirLL;
                Vector3 dirR = (1.0f - ty) * myDirUR + ty * myDirLR;
                dirL /= dirL.norm();
                dirR /= dirR.norm();
                for (int x = 0; x < myWidth; ++x) {
                    Real tx = (Real) x / (Real) (myWidth - 1);
                    Vector3 dir = (1.0f - tx) * dirL + tx * dirR;
                    Ray eye_ray = Ray(myOrigin, dir, max_depth);
                    Color result = trace(eye_ray);
                    image.at(x, y) = result.clamp();
                }
            }
            std::cout << "Done." << std::endl;
        }


        /// The rendering routine for one ray.
        /// @return the color for the given ray.
        Color trace(const Ray& ray) {
            assert(ptrScene != nullptr);
            GraphicalObject *obj_i = nullptr; // pointer to intersected object
            Point3 p_i;       // point of intersection
            Color res(0, 0, 0);

            // Look for intersection in this direction.
            Real ri = ptrScene->rayIntersection(ray, obj_i, p_i);
            // Nothing was intersected
            if (ri >= 0.0f)
                return background(ray);
            
            // gestion de la réflexion et de la refraction
            const Material& m = obj_i->getMaterial(p_i);
            if(ray.depth > 0){
                if(m.coef_reflexion != 0){
                    Vector3 direction_refl = reflect(ray.direction, obj_i->getNormal(p_i));
                    Ray ray_refl(p_i + direction_refl * 0.001f, direction_refl, ray.depth - 1);
                    Color C_refl = trace(ray_refl);
                    res += C_refl * m.specular * m.coef_reflexion;
                }
                if(m.coef_refraction != 0){
                    Ray ray_refraction = refractionRay(ray, p_i, obj_i->getNormal(p_i), m);
                    if(ray_refraction.depth > 0){
                        Color C_refraction = trace(ray_refraction);
                        res += C_refraction * m.diffuse * m.coef_refraction;
                    }
                }
            }

            res += illumination(ray, obj_i, p_i);
            return res;
        }

        /// Calcule le vecteur réfléchi à W selon la normale N.
        Vector3 reflect(const Vector3& W, Vector3 N) const {
            return W - 2 * (W.dot(N)) * N;
        }

        Ray refractionRay( const Ray& aRay, const Point3& p, Vector3 N, const Material& m ){
            Vector3 V = aRay.direction;
            Real r;
            bool isFromOutside = V.dot(N) < 0;
            if(isFromOutside){
                r = m.out_refractive_index / m.in_refractive_index;
            } else {
                r = m.in_refractive_index / m.out_refractive_index;
                N = -N;
            }
            Real c = (-N).dot(V);

            Real x = 1 - r * r * (1 - c * c);
            if(x < 0)
                return Ray(Point3(), Vector3(), -1);  // no refraction ray

            Vector3 v_refract = r * V + ((Real) (r * c - (sqrt(x)))) * N;
            v_refract = v_refract / v_refract.norm();
            return Ray(p + v_refract * 0.01f, v_refract, aRay.depth - 1);
        }

        /// Calcule l'illumination de l'objet \a obj au point \a p, sachant que l'observateur est le rayon \a ray.
        Color illumination(const Ray& ray, GraphicalObject *obj, Point3 p) {
            Material m = obj->getMaterial(p);
            Color c;
            for (auto l : ptrScene->myLights) {
                Vector3 direction = l->direction(p);
                Vector3 n = obj->getNormal(p);
                Vector3 w = reflect(ray.direction, n);

                //handle shadows
                Color light_color = l->color(p);
                light_color = shadow(Ray(p, direction), light_color);

                Real beta = w.dot(direction); // FIXME ? normalize vectors
                if (beta >= 0.f) {
                    // there is a specular color
                    Real k_s = std::pow(beta, m.shinyness);
                    c += light_color * m.specular * m.coef_reflexion * k_s;
                }
                Real k_d = direction.dot(n); // FIXME ? normalize vectors
                k_d = std::max(0.f, k_d);
                if(ray.depth == 0){
                    c += light_color * m.diffuse * k_d;
                } else {
                    c += light_color * m.diffuse * m.coef_diffusion * k_d;
                }
            }
            c += m.ambient;
            return c;
        }

        /// Calcule la couleur de la lumière (donnée par light_color) dans la
        /// direction donnée par le rayon. Si aucun objet n'est traversé,
        /// retourne light_color, sinon si un des objets traversés est opaque,
        /// retourne du noir, et enfin si les objets traversés sont
        /// transparents, attenue la couleur.
        Color shadow(const Ray& ray, Color light_color) {
            Ray rayTmp = ray;
            Color c = light_color;
            while (c.max() > 0.003f) {  // tant que la couleur n'est pas noire
                rayTmp.origin = rayTmp.origin + 0.0001f * rayTmp.direction;  // on évite d'intersecter l'objet de départ
                GraphicalObject *obj_i = nullptr;  // pointer to intersected object
                Point3 p_i;   // point of intersection

                if (ptrScene->rayIntersection(rayTmp, obj_i, p_i) < 0.f) {
                    Material m = obj_i->getMaterial(p_i);
                    c = c * m.diffuse * m.coef_refraction;
                    rayTmp.origin = p_i;
                } else {
                    return c; // le rayon n'intersecte pas avec un objet
                }
            }
            return c;
        }

    };

} // namespace rt

#endif // #define _RENDERER_H_
