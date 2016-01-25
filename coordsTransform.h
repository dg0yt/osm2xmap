#ifndef COORDS_TRANSFORM_H_INCLUDED
#define COORDS_TRANSFORM_H_INCLUDED

#ifndef __cplusplus
#error Need C++-compile to process this file
#endif

extern "C" {
#include <proj_api.h>
}
#include "xmap.h"
#include "common.h"

class CoordsTransform;

class Georeferencing {
protected:
    Coords mapRefPoint;
    Coords projectedRefPoint;
    double mapScale;
    double grivation; ///< deg
    std::string geographicCrsDesc;
    std::string projectedCrsDesc;

    projPJ projected_crs;
    projPJ geographic_crs;

    friend class CoordsTransform;
public:
    Georeferencing(XmlElement& root, const Coords& geographic_ref_point);
    Georeferencing() {};
};

class Linear
: public Coords {
public:
    Linear(Coords& coords) : Coords(coords) {};
    void translate(const Coords& delta);
    void rotate(double angle); ///< angle in radians!!!
    void scale(double scaleX, double scaleY);
};

class CoordsTransform
: public Georeferencing, TrueInit {
    Coords& projToMap(Coords& coords);
public:
    CoordsTransform() {};
    CoordsTransform(const Georeferencing& georef) : Georeferencing(georef), TrueInit(true) {};
    Coords& geographicToMap(Coords& coords);
};

#endif // COORDS_TRANSFORM_H_INCLUDED

