#ifndef _IMAGE2DREADER_HPP_
#define _IMAGE2DREADER_HPP_
#include <iostream>
#include <string>
#include "Color.h"
#include "Image2D.h"
#include <sstream>

namespace rt {
template <typename TValue>
class Image2DReader{
public:
    typedef TValue Value;
    typedef Image2D<Value> Image;
    static bool read(Image & img, std::istream & input, bool ascii)
    {
        std::cerr << "[Image2DReader<TValue>::write] NOT IMPLEMENTED." << std::endl;
        return false;
    }
};
/// Specialization for gray-level images.
template <>
class Image2DReader<unsigned char> {
public:
    typedef unsigned char Value;
    typedef Image2D<Value> Image;
    static bool read(Image & img, std::istream & input, bool ascii)
    {
        // test si tout va bien
        if (!input.good()) {
            std::cerr << "Probleme !" << std::endl;
            return false;
        }

        unsigned char g;
        std::string str;
        std::getline(input, str);
        bool asciimode = false;
        if (str == "P2") {
            asciimode = true;
        }

        std::getline(input, str); // hauteur et largeur de l'image
        if (str[0] == '#')
            std::getline(input, str);  // on saute l'eventuel commentaire

        std::istringstream ss(str);
        int w, h;
        ss >> w >> h;
        img = Image(w, h);

        std::getline(input, str); // 255

        if (asciimode) {
            input >> std::skipws;
        }
        else {
            input >> std::noskipws;
        }

        for (auto it = img.begin(), itE = img.end(); it != itE; ++it) {
            if (asciimode) {
                int i;
                input >> i;
                *it = static_cast<Value>(i);
            }
            else input >> *it;
        }

        return true;
    }
};
/// Specialization for color images.
template <>
class Image2DReader<Color> {
public:
    typedef Color Value;
    typedef Image2D<Value> Image;
    typedef Image2D<Color> ColorImage2D;
    typedef ColorImage2D::ConstIterator ConstIterator;
    static bool read(Image & img, std::istream & input, bool ascii)
    {
        // test si tout va bien
        if (!input.good()) {
            std::cerr << "Probleme !" << std::endl;
            return false;
        }

        unsigned char g;
        std::string str;
        std::getline(input, str);       

        bool asciimode = false;
        if (str == "P2") {
            asciimode = true;
        }

        std::getline(input, str);

        if (str[0] == '#')
            std::getline(input, str);

        std::istringstream ss(str);      
        int w, h;
        ss >> w >> h;
        img = Image2D<Color>(w, h);

        std::getline(input, str); // 255

        if (asciimode) {
            input >> std::skipws;
        }
        else {
            input >> std::noskipws;
        }

        for (auto it = img.begin(), itE = img.end(); it != itE; ++it) {

            unsigned char r, g, b;
            input >> r >> g >> b;
            Color c(static_cast<float>(r)/255.0f, static_cast<float>(g)/255.0f, static_cast<float>(b)/255.0f);
            *it = c;
        }

        return true;
    }
};
}
#endif
