# Ceci est un fichier de configuration pour une application Qt
# Il faut peut-etre legerement l adapter pour votre ordinateur.

# nom de votre executable
TARGET  = ray-tracer
# config de l executable
CONFIG *= qt opengl release
CONFIG += c++11
# config de Qt
QT     *= opengl xml
QMAKE_CXXFLAGS += -std=c++11

# Noms de vos fichiers entete
HEADERS = Viewer.h PointVector.h Color.h Sphere.h GraphicalObject.h Light.h \
          Material.h PointLight.h Image2D.h Image2DWriter.h Renderer.h Ray.h \
          Scene.h
          
# Noms de vos fichiers source
SOURCES = Viewer.cpp ray-tracer.cpp Sphere.cpp 

###########################################################
# Commentez/decommentez selon votre config/systeme
# (Une config windows est possible)
###########################################################

# Exemple de configuration Linux de Qt et libQGLViewer
## INCLUDEPATH *= /usr/include
## LIBS *= -L/usr/lib/x86_64-linux-gnu -lqglviewer-qt4

# Exemple de configuration MacOSX avec macports
#INCLUDEPATH *= /opt/local/include
#LIBS *= -L/opt/local/lib -lQGLViewer

# Exemple de configuration MacOSX avec frameworks
##  INCLUDEPATH *= /Users/login/libQGLViewer-2.6.4
##  LIBS *= -F/Users/login/Library/Frameworks -framework QGLViewer

#Windows :
LIBS *= -lopengl32 -lglu32
INCLUDEPATH *= D:\Cours\Info805\libQGLViewer-2.7.1
LIBS *= -LD:\Cours\Info805\libQGLViewer-2.7.1\QGLViewer -lQGLViewer2
