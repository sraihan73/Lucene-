#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::document
{
class LatLonBoundingBox;
}

namespace org::apache::lucene::document
{
class Document;
}
namespace org::apache::lucene::search
{
class Query;
}

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
namespace org::apache::lucene::search
{

using Document = org::apache::lucene::document::Document;
using LatLonBoundingBox = org::apache::lucene::document::LatLonBoundingBox;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoEncodingUtils.decodeLatitude;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoEncodingUtils.decodeLongitude;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLatitude;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.geo.GeoEncodingUtils.encodeLongitude;

/** Random testing for GeoBoundingBoxField type. */
class TestLatLonBoundingBoxQueries : public BaseRangeFieldQueryTestCase
{
  GET_CLASS_NAME(TestLatLonBoundingBoxQueries)
private:
  static const std::wstring FIELD_NAME;

protected:
  std::shared_ptr<LatLonBoundingBox>
  newRangeField(std::shared_ptr<Range> r) override;

  void addRange(std::shared_ptr<Document> doc,
                std::shared_ptr<Range> r) override;

  /** Basic test for 2d boxes */
public:
  virtual void testBasics() ;

  virtual void testToString();

protected:
  int dimension() override;

  std::shared_ptr<Range> nextRange(int dimensions) override;

  std::shared_ptr<Query> newIntersectsQuery(std::shared_ptr<Range> r) override;

  std::shared_ptr<Query> newContainsQuery(std::shared_ptr<Range> r) override;

  std::shared_ptr<Query> newWithinQuery(std::shared_ptr<Range> r) override;

  std::shared_ptr<Query> newCrossesQuery(std::shared_ptr<Range> r) override;

protected:
  class GeoBBox : public Range
  {
    GET_CLASS_NAME(GeoBBox)
  protected:
    double minLat = 0, minLon = 0, maxLat = 0, maxLon = 0;
    int dimension = 0;

  public:
    GeoBBox(int dimension);

  protected:
    static double quantizeLat(double lat);

    virtual double quantizeLon(double lon);

    int numDimensions() override;

    std::optional<double> getMin(int dim) override;

    void setMin(int dim, std::any val) override;

  private:
    void setMinLat(double d);

    void setMinLon(double d);

    void setMaxLat(double d);

    void setMaxLon(double d);

  protected:
    std::optional<double> getMax(int dim) override;

    void setMax(int dim, std::any val) override;

    bool isEqual(std::shared_ptr<Range> other) override;

    bool isDisjoint(std::shared_ptr<Range> other) override;

    bool isWithin(std::shared_ptr<Range> other) override;

    bool contains(std::shared_ptr<Range> other) override;

  public:
    virtual std::wstring toString();

  protected:
    std::shared_ptr<GeoBBox> shared_from_this()
    {
      return std::static_pointer_cast<GeoBBox>(Range::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestLatLonBoundingBoxQueries> shared_from_this()
  {
    return std::static_pointer_cast<TestLatLonBoundingBoxQueries>(
        BaseRangeFieldQueryTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
