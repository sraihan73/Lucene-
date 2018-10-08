using namespace std;

#include "EarthDebugger.h"

namespace org::apache::lucene::geo
{
using SloppyMath = org::apache::lucene::util::SloppyMath;

EarthDebugger::EarthDebugger()
{
  b->append(L"<!DOCTYPE HTML>\n");
  b->append(L"<html>\n");
  b->append(L"  <head>\n");
  b->append(
      L"    <script src=\"http://www.webglearth.com/v2/api.js\"></script>\n");
  b->append(L"    <script>\n");
  b->append(L"      function initialize() {\n");
  b->append(L"        var earth = new WE.map_obj('earth_div');\n");
}

EarthDebugger::EarthDebugger(double centerLat, double centerLon,
                             double altitudeMeters)
{
  b->append(L"<!DOCTYPE HTML>\n");
  b->append(L"<html>\n");
  b->append(L"  <head>\n");
  b->append(
      L"    <script src=\"http://www.webglearth.com/v2/api.js\"></script>\n");
  b->append(L"    <script>\n");
  b->append(L"      function initialize() {\n");
  b->append(L"        var earth = new WE.map_obj('earth_div', {center: [" +
            to_wstring(centerLat) + L", " + to_wstring(centerLon) +
            L"], altitude: " + to_wstring(altitudeMeters) + L"});\n");
}

void EarthDebugger::addPolygon(shared_ptr<Polygon> poly)
{
  addPolygon(poly, L"#00ff00");
}

void EarthDebugger::addPolygon(shared_ptr<Polygon> poly, const wstring &color)
{
  wstring name = L"poly" + to_wstring(nextShape);
  nextShape++;

  b->append(L"        var " + name + L" = WE.polygon([\n");
  std::deque<double> polyLats = poly->getPolyLats();
  std::deque<double> polyLons = poly->getPolyLons();
  for (int i = 0; i < polyLats.size(); i++) {
    b->append(L"          [" + to_wstring(polyLats[i]) + L", " +
              to_wstring(polyLons[i]) + L"],\n");
  }
  b->append(L"        ], {color: '" + color +
            L"', fillColor: \"#000000\", fillOpacity: 0.0001});\n");
  b->append(L"        " + name + L".addTo(earth);\n");

  for (auto hole : poly->getHoles()) {
    addPolygon(hole, L"#ffffff");
  }
}

double EarthDebugger::MAX_KM_PER_STEP = 100.0;

int EarthDebugger::getStepCount(double minLat, double maxLat, double minLon,
                                double maxLon)
{
  double distanceMeters =
      SloppyMath::haversinMeters(minLat, minLon, maxLat, maxLon);
  return max(
      1, static_cast<int>(round((distanceMeters / 1000.0) / MAX_KM_PER_STEP)));
}

void EarthDebugger::drawSegment(double minLat, double maxLat, double minLon,
                                double maxLon)
{
  int steps = getStepCount(minLat, maxLat, minLon, maxLon);
  for (int i = 0; i < steps; i++) {
    b->append(L"          [" +
              to_wstring(minLat + (maxLat - minLat) * i / steps) + L", " +
              to_wstring(minLon + (maxLon - minLon) * i / steps) + L"],\n");
  }
}

void EarthDebugger::addRect(double minLat, double maxLat, double minLon,
                            double maxLon)
{
  addRect(minLat, maxLat, minLon, maxLon, L"#ff0000");
}

void EarthDebugger::addRect(double minLat, double maxLat, double minLon,
                            double maxLon, const wstring &color)
{
  wstring name = L"rect" + to_wstring(nextShape);
  nextShape++;

  b->append(L"        // lat: " + to_wstring(minLat) + L" TO " +
            to_wstring(maxLat) + L"; lon: " + to_wstring(minLon) + L" TO " +
            to_wstring(maxLon) + L"\n");
  b->append(L"        var " + name + L" = WE.polygon([\n");

  b->append(L"          // min -> max lat, min lon\n");
  drawSegment(minLat, maxLat, minLon, minLon);

  b->append(L"          // max lat, min -> max lon\n");
  drawSegment(maxLat, maxLat, minLon, maxLon);

  b->append(L"          // max -> min lat, max lon\n");
  drawSegment(maxLat, minLat, maxLon, maxLon);

  b->append(L"          // min lat, max -> min lon\n");
  drawSegment(minLat, minLat, maxLon, minLon);

  b->append(L"          // min lat, min lon\n");
  b->append(L"          [" + to_wstring(minLat) + L", " + to_wstring(minLon) +
            L"]\n");
  b->append(L"        ], {color: \"" + color + L"\", fillColor: \"" + color +
            L"\"});\n");
  b->append(L"        " + name + L".addTo(earth);\n");
}

void EarthDebugger::addLatLine(double lat, double minLon, double maxLon)
{
  wstring name = L"latline" + to_wstring(nextShape);
  nextShape++;

  b->append(L"        var " + name + L" = WE.polygon([\n");
  double lon;
  int steps = getStepCount(lat, minLon, lat, maxLon);
  for (lon = minLon; lon <= maxLon; lon += (maxLon - minLon) / steps) {
    b->append(L"          [" + to_wstring(lat) + L", " + to_wstring(lon) +
              L"],\n");
  }
  b->append(L"          [" + to_wstring(lat) + L", " + to_wstring(maxLon) +
            L"],\n");
  lon -= (maxLon - minLon) / steps;
  for (; lon >= minLon; lon -= (maxLon - minLon) / steps) {
    b->append(L"          [" + to_wstring(lat) + L", " + to_wstring(lon) +
              L"],\n");
  }
  b->append(L"        ], {color: \"#ff0000\", fillColor: \"#ffffff\", opacity: "
            L"1, fillOpacity: 0.0001});\n");
  b->append(L"        " + name + L".addTo(earth);\n");
}

void EarthDebugger::addLonLine(double minLat, double maxLat, double lon)
{
  wstring name = L"lonline" + to_wstring(nextShape);
  nextShape++;

  b->append(L"        var " + name + L" = WE.polygon([\n");
  double lat;
  int steps = getStepCount(minLat, lon, maxLat, lon);
  for (lat = minLat; lat <= maxLat; lat += (maxLat - minLat) / steps) {
    b->append(L"          [" + to_wstring(lat) + L", " + to_wstring(lon) +
              L"],\n");
  }
  b->append(L"          [" + to_wstring(maxLat) + L", " + to_wstring(lon) +
            L"],\n");
  lat -= (maxLat - minLat) / 36;
  for (; lat >= minLat; lat -= (maxLat - minLat) / steps) {
    b->append(L"          [" + to_wstring(lat) + L", " + to_wstring(lon) +
              L"],\n");
  }
  b->append(L"        ], {color: \"#ff0000\", fillColor: \"#ffffff\", opacity: "
            L"1, fillOpacity: 0.0001});\n");
  b->append(L"        " + name + L".addTo(earth);\n");
}

void EarthDebugger::addPoint(double lat, double lon)
{
  b->append(L"        WE.marker([" + to_wstring(lat) + L", " + to_wstring(lon) +
            L"]).addTo(earth);\n");
}

void EarthDebugger::addCircle(double centerLat, double centerLon,
                              double radiusMeters, bool alsoAddBBox)
{
  addPoint(centerLat, centerLon);
  wstring name = L"circle" + to_wstring(nextShape);
  nextShape++;
  b->append(L"        var " + name + L" = WE.polygon([\n");
  inverseHaversin(b, centerLat, centerLon, radiusMeters);
  b->append(L"        ], {color: '#00ff00', fillColor: \"#000000\", "
            L"fillOpacity: 0.0001 });\n");
  b->append(L"        " + name + L".addTo(earth);\n");

  if (alsoAddBBox) {
    shared_ptr<Rectangle> box =
        Rectangle::fromPointDistance(centerLat, centerLon, radiusMeters);
    addRect(box->minLat, box->maxLat, box->minLon, box->maxLon);
    addLatLine(Rectangle::axisLat(centerLat, radiusMeters), box->minLon,
               box->maxLon);
  }
}

wstring EarthDebugger::finish()
{
  if (finished) {
    throw make_shared<IllegalStateException>(L"already finished");
  }
  finished = true;
  b->append(
      L"        "
      L"WE.tileLayer('http://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png',{\n");
  b->append(L"          attribution: '© OpenStreetMap contributors'\n");
  b->append(L"        }).addTo(earth);\n");
  b->append(L"      }\n");
  b->append(L"    </script>\n");
  b->append(L"    <style>\n");
  b->append(L"      html, body{padding: 0; margin: 0;}\n");
  b->append(L"      #earth_div{top: 0; right: 0; bottom: 0; left: 0; position: "
            L"absolute !important;}\n");
  b->append(L"    </style>\n");
  b->append(L"    <title>WebGL Earth API: Hello World</title>\n");
  b->append(L"  </head>\n");
  b->append(L"  <body onload=\"initialize()\">\n");
  b->append(L"    <div id=\"earth_div\"></div>\n");
  b->append(L"  </body>\n");
  b->append(L"</html>\n");

  return b->toString();
}

void EarthDebugger::inverseHaversin(shared_ptr<StringBuilder> b,
                                    double centerLat, double centerLon,
                                    double radiusMeters)
{
  double angle = 0;
  int steps = 100;

  while (angle < 360) {
    double x = cos(SloppyMath::toRadians(angle));
    double y = sin(SloppyMath::toRadians(angle));
    double factor = 2.0;
    double step = 1.0;
    int last = 0;
    double lastDistanceMeters = 0.0;
    // System.out.println("angle " + angle + " slope=" + slope);
    while (true) {
      double lat = wrapLat(centerLat + y * factor);
      double lon = wrapLon(centerLon + x * factor);
      double distanceMeters =
          SloppyMath::haversinMeters(centerLat, centerLon, lat, lon);

      if (last == 1 && distanceMeters < lastDistanceMeters) {
        // For large enough circles, some angles are not possible:
        // System.out.println("  done: give up on angle " + angle);
        angle += 360.0 / steps;
        goto newAngleContinue;
      }
      if (last == -1 && distanceMeters > lastDistanceMeters) {
        // For large enough circles, some angles are not possible:
        // System.out.println("  done: give up on angle " + angle);
        angle += 360.0 / steps;
        goto newAngleContinue;
      }
      lastDistanceMeters = distanceMeters;

      // System.out.println("  iter lat=" + lat + " lon=" + lon + " distance=" +
      // distanceMeters + " vs " + radiusMeters);
      if (abs(distanceMeters - radiusMeters) < 0.1) {
        b->append(L"          [" + to_wstring(lat) + L", " + to_wstring(lon) +
                  L"],\n");
        break;
      }
      if (distanceMeters > radiusMeters) {
        // too big
        // System.out.println("    smaller");
        factor -= step;
        if (last == 1) {
          // System.out.println("      half-step");
          step /= 2.0;
        }
        last = -1;
      } else if (distanceMeters < radiusMeters) {
        // too small
        // System.out.println("    bigger");
        factor += step;
        if (last == -1) {
          // System.out.println("      half-step");
          step /= 2.0;
        }
        last = 1;
      }
    }
    angle += 360.0 / steps;
  newAngleContinue:;
  }
newAngleBreak:;
}

double EarthDebugger::wrapLat(double lat)
{
  // System.out.println("wrapLat " + lat);
  if (lat > 90) {
    // System.out.println("  " + (180 - lat));
    return 180 - lat;
  } else if (lat < -90) {
    // System.out.println("  " + (-180 - lat));
    return -180 - lat;
  } else {
    // System.out.println("  " + lat);
    return lat;
  }
}

double EarthDebugger::wrapLon(double lon)
{
  // System.out.println("wrapLon " + lon);
  if (lon > 180) {
    // System.out.println("  " + (lon - 360));
    return lon - 360;
  } else if (lon < -180) {
    // System.out.println("  " + (lon + 360));
    return lon + 360;
  } else {
    // System.out.println("  " + lon);
    return lon;
  }
}
} // namespace org::apache::lucene::geo