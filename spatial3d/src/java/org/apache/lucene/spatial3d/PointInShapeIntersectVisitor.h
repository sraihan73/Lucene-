#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class DocIdSetBuilder;
}

namespace org::apache::lucene::spatial3d::geom
{
class GeoShape;
}
namespace org::apache::lucene::util
{
class BulkAdder;
}
namespace org::apache::lucene::spatial3d::geom
{
class XYZBounds;
}
namespace org::apache::lucene::index
{
class PointValues;
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

namespace org::apache::lucene::spatial3d
{

using IntersectVisitor =
    org::apache::lucene::index::PointValues::IntersectVisitor;
using Relation = org::apache::lucene::index::PointValues::Relation;
using GeoShape = org::apache::lucene::spatial3d::geom::GeoShape;
using XYZBounds = org::apache::lucene::spatial3d::geom::XYZBounds;
using DocIdSetBuilder = org::apache::lucene::util::DocIdSetBuilder;

class PointInShapeIntersectVisitor
    : public std::enable_shared_from_this<PointInShapeIntersectVisitor>,
      public IntersectVisitor
{
  GET_CLASS_NAME(PointInShapeIntersectVisitor)
private:
  const std::shared_ptr<DocIdSetBuilder> hits;
  const std::shared_ptr<GeoShape> shape;
  const double minimumX;
  const double maximumX;
  const double minimumY;
  const double maximumY;
  const double minimumZ;
  const double maximumZ;
  std::shared_ptr<DocIdSetBuilder::BulkAdder> adder;

public:
  PointInShapeIntersectVisitor(std::shared_ptr<DocIdSetBuilder> hits,
                               std::shared_ptr<GeoShape> shape,
                               std::shared_ptr<XYZBounds> bounds);

  void grow(int count) override;

  void visit(int docID) override;

  void visit(int docID, std::deque<char> &packedValue) override;

  Relation compare(std::deque<char> &minPackedValue,
                   std::deque<char> &maxPackedValue) override;
};

} // namespace org::apache::lucene::spatial3d
