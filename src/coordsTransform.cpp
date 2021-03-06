/*
 *    Copyright 2016 Semyon Yakimov
 *
 *    This file is part of Osm2xmap.
 *
 *    Osm2xmap is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    Osm2xmap is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with Osm2xmap.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cmath>
#include "coordsTransform.h"

Georeferencing::Georeferencing(XmlElement& root, const Coords& geographic_ref_point) {
    XmlElement georeferencingNode = root.getChild("georeferencing");
    mapScale = 1000.0 / georeferencingNode.getAttribute<double>("scale");

    double lat = geographic_ref_point.Y();
    double lon = geographic_ref_point.X();
    /*
    unsigned zone = std::floor(geographic_ref_point.X() / 6) + 1;
    if (zone > 99) {
        throw Error("invalid zone " + std::to_string(zone));
    }
    parameter = 28400 + zone;
    */
    // UTM zone
    unsigned zone = std::floor((lon + 180) / 6) + 1;

    if( lat >= 56.0 && lat < 64.0 && lon >= 3.0 && lon < 12.0 ) {
        zone = 32;
    }
    // Special zones for Svalbard
    if( lat >= 72.0 && lat < 84.0 ) {
        if  ( lon >= 0.0  && lon < 9.0 ) {
            zone = 31;
        }
        else if( lon >= 9.0  && lon < 21.0 ) {
            zone = 33;
        }
        else if(lon >= 21.0 && lon < 33.0 ) {
            zone = 35;
        }
        else if(lon >= 33.0 && lon < 42.0 ) {
            zone = 37;
        }
    }

    if (lat >= 0) {
        parameter = std::to_string(zone) + " N";
    }
    else {
        parameter = std::to_string(zone) + " E";
    }

    // Pulkovo 42 datum (EPSG:284xx)
    //projectedCrsDesc = "+init=epsg:" + std::to_string(parameter);

    // UTM
    projectedCrsDesc = "+proj=utm +datum=WGS84 +zone=" + std::to_string(zone);
    geographicCrsDesc = "+proj=latlong +datum=WGS84";

    if (!(projected_crs = pj_init_plus(projectedCrsDesc.c_str())) ) {
        throw Error("projected coordinate system init failed!");
    }
    if (!(geographic_crs = pj_init_plus(geographicCrsDesc.c_str())) ) {
        throw Error("geographic coordinate system init failed!");
    }
    projectedRefPoint = geographic_ref_point;
    projectedRefPoint = geographicToProj(projectedRefPoint);
    geographicRefPoint = geographic_ref_point;

    info("Using georeferencing:");
    info("   mapScale           " + std::to_string(mapScale));
    info("   declination        " + std::to_string(declination));
    info("   grivation          " + std::to_string(grivation));
    info("   mapRefPoint        " + mapRefPoint.getAsString());
    info("   projectedRefPoint  " + projectedRefPoint.getAsString());
    info("   geographicRefPoint " + geographicRefPoint.getAsString());
    info("   projectedCrsDesc   '" + projectedCrsDesc + "'");
    info("   geographicCrsDesc  '" + geographicCrsDesc + "'");
}

Coords&
Georeferencing::geographicToProj(Coords& coords) {
    coords *= DEG_TO_RAD;
    double x = coords.X();
    double y = coords.Y();
    if (pj_transform(geographic_crs, projected_crs, 1, 1, &x, &y, nullptr)) {
        throw Error("pj_transform failed");
    }
    coords = Coords(x,y);
    return coords; 
}

void Linear::translate(const Coords& delta) {
    x -= delta.X();
    y -= delta.Y();
}

void Linear::rotate(double angle) { ///< angle in radians!!!
    Coords tmp(x * cos(angle) - y * sin(angle),
               x * sin(angle) + y * cos(angle));
    x = tmp.X();
    y = tmp.Y();
}

void Linear::scale(double scaleX, double scaleY) {
    x *= scaleX;
    y *= scaleY;
}

Coords&
CoordsTransform::projToMap(Coords& coords) {
    Linear linear(coords);
    Coords delta = projectedRefPoint;
    linear.translate(delta);
    linear.rotate(grivation * DEG_TO_RAD);
    linear.scale(mapScale, -mapScale);
    Coords mapDelta = mapRefPoint * -1;
    linear.translate(mapDelta);
    linear.scale(1000,1000);
    return coords = linear;
}

Coords&
CoordsTransform::geographicToMap(Coords& coords) {
    if (!isInited()) {
        throw Error("CoordsTransform not inited!");
    }
    coords = geographicToProj(coords);
    coords = projToMap(coords);
    coords = Coords(round(coords.X()), round(coords.Y()));
    return coords;
}

