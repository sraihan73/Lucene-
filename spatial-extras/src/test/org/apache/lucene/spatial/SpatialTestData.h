#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

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
namespace org::apache::lucene::spatial
{

using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::shape::Shape;

// This class is modelled after SpatialTestQuery.
// Before Lucene 4.7, this was a bit different in Spatial4j as SampleData &
// SampleDataReader.

class SpatialTestData : public std::enable_shared_from_this<SpatialTestData>
{
  GET_CLASS_NAME(SpatialTestData)
public:
  std::wstring id;
  std::wstring name;
  std::shared_ptr<Shape> shape;

  /** Reads the stream, consuming a format that is a tab-separated values of 3
   * columns: an "id", a "name" and the "shape".  Empty lines and lines starting
   * with a '#' are skipped. The stream is closed.
   */
  static std::shared_ptr<Iterator<std::shared_ptr<SpatialTestData>>>
  getTestData(std::shared_ptr<InputStream> in_,
              std::shared_ptr<SpatialContext> ctx) ;
};

} // namespace org::apache::lucene::spatial
