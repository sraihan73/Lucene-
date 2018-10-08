#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial/prefix/tree/SpatialPrefixTree.h"

#include  "core/src/java/org/apache/lucene/spatial/prefix/CellToBytesRefIterator.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/spatial/query/SpatialArgs.h"

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

using Query = org::apache::lucene::search::Query;
using SpatialPrefixTree =
    org::apache::lucene::spatial::prefix::tree::SpatialPrefixTree;
using SpatialArgs = org::apache::lucene::spatial::query::SpatialArgs;

/**
 * A basic implementation of {@link PrefixTreeStrategy} using a large
 * {@link TermInSetQuery} of all the cells from
 * {@link
 * SpatialPrefixTree#getTreeCellIterator(org.locationtech.spatial4j.shape.Shape,
 * int)}. It only supports the search of indexed Point shapes. <p> The precision
 * of query shapes (distErrPct) is an important factor in using this Strategy.
 * If the precision is too precise then it will result in many terms which will
 * amount to a slower query.
 *
 * @lucene.experimental
 */
class TermQueryPrefixTreeStrategy : public PrefixTreeStrategy
{
  GET_CLASS_NAME(TermQueryPrefixTreeStrategy)

protected:
  bool simplifyIndexedCells = false;

public:
  TermQueryPrefixTreeStrategy(std::shared_ptr<SpatialPrefixTree> grid,
                              const std::wstring &fieldName);

protected:
  std::shared_ptr<CellToBytesRefIterator> newCellToBytesRefIterator() override;

private:
  class CellToBytesRefIteratorAnonymousInnerClass
      : public CellToBytesRefIterator
  {
    GET_CLASS_NAME(CellToBytesRefIteratorAnonymousInnerClass)
  private:
    std::shared_ptr<TermQueryPrefixTreeStrategy> outerInstance;

  public:
    CellToBytesRefIteratorAnonymousInnerClass(
        std::shared_ptr<TermQueryPrefixTreeStrategy> outerInstance);

    std::shared_ptr<BytesRef> next() override;

  protected:
    std::shared_ptr<CellToBytesRefIteratorAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          CellToBytesRefIteratorAnonymousInnerClass>(
          CellToBytesRefIterator::shared_from_this());
    }
  };

public:
  std::shared_ptr<Query> makeQuery(std::shared_ptr<SpatialArgs> args) override;

protected:
  std::shared_ptr<TermQueryPrefixTreeStrategy> shared_from_this()
  {
    return std::static_pointer_cast<TermQueryPrefixTreeStrategy>(
        PrefixTreeStrategy::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial/prefix/
