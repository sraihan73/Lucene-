#pragma once
#include "stringhelper.h"
#define _USE_MATH_DEFINES
#include <cmath>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial3d/geom/PlanetModel.h"

#include  "core/src/java/org/apache/lucene/spatial3d/geom/Vector.h"

/*
 * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * sraihan.com licenses this file to You under GPLv3 License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
namespace org::apache::lucene::spatial3d::geom
{

/**
 * This class represents a point on the surface of a sphere or ellipsoid.
 *
 * @lucene.experimental
 */
class GeoPoint : public Vector, public SerializableObject
{
  GET_CLASS_NAME(GeoPoint)

  // By making lazily-evaluated variables be "volatile", we guarantee atomicity
  // when they are updated.  This is necessary if we are using these classes in
  // a multi-thread fashion, because we don't try to synchronize for the lazy
  // computation.

  /** This is the lazily-evaluated magnitude.  Some constructors include it, but
   * others don't, and we try not to create extra computation by always
   * computing it.  Does not need to be
   * synchronized for thread safety, because depends wholly on immutable
   * variables of this class. */
protected:
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: protected volatile double magnitude =
  // -std::numeric_limits<double>::infinity(); C++ NOTE: Fields cannot have the
  // same name as methods:
  double magnitude_ = -std::numeric_limits<double>::infinity();
  /** Lazily-evaluated latitude.  Does not need to be
   * synchronized for thread safety, because depends wholly on immutable
   * variables of this class.  */
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: protected volatile double latitude =
  // -std::numeric_limits<double>::infinity();
  double latitude = -std::numeric_limits<double>::infinity();
  /** Lazily-evaluated longitude.   Does not need to be
   * synchronized for thread safety, because depends wholly on immutable
   * variables of this class.  */
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: protected volatile double longitude =
  // -std::numeric_limits<double>::infinity();
  double longitude = -std::numeric_limits<double>::infinity();

  /** Construct a GeoPoint from the trig functions of a lat and lon pair.
   * @param planetModel is the planetModel to put the point on.
   * @param sinLat is the sin of the latitude.
   * @param sinLon is the sin of the longitude.
   * @param cosLat is the cos of the latitude.
   * @param cosLon is the cos of the longitude.
   * @param lat is the latitude.
   * @param lon is the longitude.
   */
public:
  GeoPoint(std::shared_ptr<PlanetModel> planetModel, double const sinLat,
           double const sinLon, double const cosLat, double const cosLon,
           double const lat, double const lon);

  /** Construct a GeoPoint from the trig functions of a lat and lon pair.
   * @param planetModel is the planetModel to put the point on.
   * @param sinLat is the sin of the latitude.
   * @param sinLon is the sin of the longitude.
   * @param cosLat is the cos of the latitude.
   * @param cosLon is the cos of the longitude.
   */
  GeoPoint(std::shared_ptr<PlanetModel> planetModel, double const sinLat,
           double const sinLon, double const cosLat, double const cosLon);

  /** Construct a GeoPoint from a latitude/longitude pair.
   * @param planetModel is the planetModel to put the point on.
   * @param lat is the latitude.
   * @param lon is the longitude.
   */
  GeoPoint(std::shared_ptr<PlanetModel> planetModel, double const lat,
           double const lon);

  /** Construct a GeoPoint from a planet model and an input stream.
   */
  GeoPoint(std::shared_ptr<PlanetModel> planetModel,
           std::shared_ptr<InputStream> inputStream) ;

  /** Construct a GeoPoint from a unit (x,y,z) deque and a magnitude.
   * @param magnitude is the desired magnitude, provided to put the point on the
   * ellipsoid.
   * @param x is the unit x value.
   * @param y is the unit y value.
   * @param z is the unit z value.
   * @param lat is the latitude.
   * @param lon is the longitude.
   */
  GeoPoint(double const magnitude, double const x, double const y,
           double const z, double lat, double lon);

  /** Construct a GeoPoint from a unit (x,y,z) deque and a magnitude.
   * @param magnitude is the desired magnitude, provided to put the point on the
   * ellipsoid.
   * @param x is the unit x value.
   * @param y is the unit y value.
   * @param z is the unit z value.
   */
  GeoPoint(double const magnitude, double const x, double const y,
           double const z);

  /** Construct a GeoPoint from an (x,y,z) value.
   * The (x,y,z) tuple must be on the desired ellipsoid.
   * @param x is the ellipsoid point x value.
   * @param y is the ellipsoid point y value.
   * @param z is the ellipsoid point z value.
   */
  GeoPoint(double const x, double const y, double const z);

  void
  write(std::shared_ptr<OutputStream> outputStream)  override;

  /** Compute an arc distance between two points.
   * Note: this is an angular distance, and not a surface distance, and is
   * therefore independent of planet model. For surface distance, see {@link
   * PlanetModel#surfaceDistance(GeoPoint, GeoPoint)}
   * @param v is the second point.
   * @return the angle, in radians, between the two points.
   */
  virtual double arcDistance(std::shared_ptr<Vector> v);

  /** Compute an arc distance between two points.
   * @param x is the x part of the second point.
   * @param y is the y part of the second point.
   * @param z is the z part of the second point.
   * @return the angle, in radians, between the two points.
   */
  virtual double arcDistance(double const x, double const y, double const z);

  /** Compute the latitude for the point.
   * @return the latitude.
   */
  virtual double getLatitude();

  /** Compute the longitude for the point.
   * @return the longitude value.  Uses 0.0 if there is no computable longitude.
   */
  virtual double getLongitude();

  /** Compute the linear magnitude of the point.
   * @return the magnitude.
   */
  double magnitude() override;

  /** Compute whether point matches another.
   *@param p is the other point.
   *@return true if the same.
   */
  virtual bool isIdentical(std::shared_ptr<GeoPoint> p);

  /** Compute whether point matches another.
   *@param x is the x value
   *@param y is the y value
   *@param z is the z value
   *@return true if the same.
   */
  virtual bool isIdentical(double const x, double const y, double const z);

  virtual std::wstring toString();

protected:
  std::shared_ptr<GeoPoint> shared_from_this()
  {
    return std::static_pointer_cast<GeoPoint>(Vector::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial3d/geom/
