#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::document
{
class Document;
}

namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::geo
{
class Polygon;
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
using BaseGeoPointTestCase = org::apache::lucene::geo::BaseGeoPointTestCase;
using Polygon = org::apache::lucene::geo::Polygon;

class TestLatLonDocValuesQueries : public BaseGeoPointTestCase
{
  GET_CLASS_NAME(TestLatLonDocValuesQueries)

protected:
  bool supportsPolygons() override;

  void addPointToDoc(const std::wstring &field, std::shared_ptr<Document> doc,
                     double lat, double lon) override;

  std::shared_ptr<Query> newRectQuery(const std::wstring &field, double minLat,
                                      double maxLat, double minLon,
                                      double maxLon) override;

  std::shared_ptr<Query> newDistanceQuery(const std::wstring &field,
                                          double centerLat, double centerLon,
                                          double radiusMeters) override;

  std::shared_ptr<Query>
  newPolygonQuery(const std::wstring &field,
                  std::deque<Polygon> &polygons) override;

  double quantizeLat(double latRaw) override;

  double quantizeLon(double lonRaw) override;

protected:
  std::shared_ptr<TestLatLonDocValuesQueries> shared_from_this()
  {
    return std::static_pointer_cast<TestLatLonDocValuesQueries>(
        org.apache.lucene.geo.BaseGeoPointTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
