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
            } else if (z < 0.5f) {
                Color white = Color(1.f, 1.f, 1.f);
                Color blue = Color(0.f, 0.f, 1.f);
                return lerp(white, blue, z * 2.f);
            } else if (z < 1.f) {
                Color blue = Color(0.f, 0.f, 1.f);
                Color black = Color(0.f, 0.f, 0.f);
                return lerp(blue, black, (z - 0.5f) * 1.5f);
            }
            return Color(0.f, 0.f, 0.f);
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

            // Look for intersection in this direction.
            Real ri = ptrScene->rayIntersection(ray, obj_i, p_i);
            // Nothing was intersected
            if (ri >= 0.0f)
                return background(ray);
            Color res = illumination(ray, obj_i, p_i);
            for (auto l : ptrScene->myLights) {
                Vector3 rayToLight = l->direction(p_i);
                Ray myRay(p_i, rayToLight);
                const Color& light_color = l->color(p_i);  // FIXME Vector3(0, 0, 0) - ...
                Color shadow_color = shadow(myRay, light_color);
                if(shadow_color != light_color && shadow_color.max() > 0.003f)  // FIXME
                    res = res * shadow_color;
            }
            return res;
        }

        /// Calcule le vecteur réfléchi à W selon la normale N.
        Vector3 reflect(const Vector3& W, Vector3 N) const {
            return W - 2 * (W.dot(N)) * N;  // TODO handle W.dot(N) negative (the ray is from inside)
        }

        /// Calcule l'illumination de l'objet \a obj au point \a p, sachant que l'observateur est le rayon \a ray.
        Color illumination(const Ray& ray, GraphicalObject *obj, Point3 p) {
            Material m = obj->getMaterial(p);
            Color c;
            for (auto l : ptrScene->myLights) {
                Vector3 direction = l->direction(p);
                Vector3 n = obj->getNormal(p);
                Vector3 w = reflect(ray.direction, n);
                Real beta = w.dot(direction); // FIXME ? normalize vectors
                if (beta >= 0.f) {
                    // there is a specular color
                    Real k_s = std::pow(beta, m.shinyness);
                    c += l->color(p) * m.specular * k_s;
                }
                Real k_d = direction.dot(n); // FIXME ? normalize vectors
                k_d = std::max(0.f, k_d);
                c += l->color(p) * m.diffuse * k_d;
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
                rayTmp.origin = rayTmp.origin + rayTmp.direction;  // on évite d'intersecter l'objet de départ
                //std::cout << rayTmp.origin << " vs " << ray.origin << " " << rayTmp.direction << std::endl;
                GraphicalObject *obj_i = nullptr;  // pointer to intersected object
                Point3 p_i;   // point of intersection

                if (ptrScene->rayIntersection(rayTmp, obj_i, p_i) < 0.f) {
                    Material m = obj_i->getMaterial(p_i);
                    c = c * m.coef_diffusion * m.coef_refraction;
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
