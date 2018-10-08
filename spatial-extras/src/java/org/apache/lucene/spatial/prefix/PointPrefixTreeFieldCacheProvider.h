#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::spatial::prefix::tree
{
class SpatialPrefixTree;
}

namespace org::apache::lucene::spatial::prefix::tree
{
class Cell;
}
namespace org::apache::lucene::util
{
class BytesRef;
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
namespace org::apache::lucene::spatial::prefix
{

using org::locationtech::spatial4j::shape::Point;
using Cell = org::apache::lucene::spatial::prefix::tree::Cell;
using SpatialPrefixTree =
    org::apache::lucene::spatial::prefix::tree::SpatialPrefixTree;
using ShapeFieldCacheProvider =
    org::apache::lucene::spatial::util::ShapeFieldCacheProvider;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * Implementation of {@link ShapeFieldCacheProvider} designed for {@link
 * PrefixTreeStrategy}s that index points (AND ONLY POINTS!).
 *
 * @lucene.internal
 */
class PointPrefixTreeFieldCacheProvider
    : public ShapeFieldCacheProvider<std::shared_ptr<Point>>
{
  GET_CLASS_NAME(PointPrefixTreeFieldCacheProvider)

private:
  const std::shared_ptr<SpatialPrefixTree> grid;
  std::shared_ptr<Cell> scanCell; // re-used in readShape to save GC

public:
  PointPrefixTreeFieldCacheProvider(std::shared_ptr<SpatialPrefixTree> grid,
                                    const std::wstring &shapeField,
                                    int defaultSize);

protected:
  std::shared_ptr<Point> readShape(std::shared_ptr<BytesRef> term) override;

protected:
  std::shared_ptr<PointPrefixTreeFieldCacheProvider> shared_from_this()
  {
    return std::static_pointer_cast<PointPrefixTreeFieldCacheProvider>(
        org.apache.lucene.spatial.util.ShapeFieldCacheProvider<
            org.locationtech.spatial4j.shape.Point>::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial::prefix
