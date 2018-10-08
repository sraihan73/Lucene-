#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial/prefix/tree/Cell.h"

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
namespace org::apache::lucene::spatial::prefix::tree
{

using org::locationtech::spatial4j::shape::Shape;

/**
 * A filtering iterator of Cells. Those not matching the provided shape
 * (disjoint) are skipped. If {@code shapeFilter} is null then all cells are
 * returned.
 *
 * @lucene.internal
 */
class FilterCellIterator : public CellIterator
{
  GET_CLASS_NAME(FilterCellIterator)
public:
  const std::shared_ptr<Iterator<std::shared_ptr<Cell>>> baseIter;
  const std::shared_ptr<Shape> shapeFilter;

  FilterCellIterator(std::shared_ptr<Iterator<std::shared_ptr<Cell>>> baseIter,
                     std::shared_ptr<Shape> shapeFilter);

  bool hasNext() override;

protected:
  std::shared_ptr<FilterCellIterator> shared_from_this()
  {
    return std::static_pointer_cast<FilterCellIterator>(
        CellIterator::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial/prefix/tree/
