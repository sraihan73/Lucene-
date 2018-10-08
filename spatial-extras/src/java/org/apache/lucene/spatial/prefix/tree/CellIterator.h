#pragma once
#include "exceptionhelper.h"
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

/**
 * An Iterator of SpatialPrefixTree Cells. The order is always sorted without
 * duplicates.
 *
 * @lucene.experimental
 */
class CellIterator : public std::enable_shared_from_this<CellIterator>,
                     public Iterator<std::shared_ptr<Cell>>
{
  GET_CLASS_NAME(CellIterator)

  // note: nextCell or thisCell can be non-null but neither at the same time.
  // That's
  // because they might return the same instance when re-used!

protected:
  std::shared_ptr<Cell> nextCell; // to be returned by next(), and null'ed after
  // C++ NOTE: Fields cannot have the same name as methods:
  std::shared_ptr<Cell>
      thisCell_; // see next() & thisCell(). Should be cleared in hasNext().

  /** Returns the cell last returned from {@link #next()}. It's cleared by
   * hasNext(). */
public:
  virtual std::shared_ptr<Cell> thisCell();

  // Arguably this belongs here and not on Cell
  // public SpatialRelation getShapeRel()

  /**
   * Gets the next cell that is &gt;= {@code fromCell}, compared using non-leaf
   * bytes. If it returns null then the iterator is exhausted.
   */
  virtual std::shared_ptr<Cell> nextFrom(std::shared_ptr<Cell> fromCell);

  /** This prevents sub-cells (those underneath the current cell) from being
   * iterated to, if applicable, otherwise a NO-OP. */
  void remove() override;

  std::shared_ptr<Cell> next() override;
};

} // #include  "core/src/java/org/apache/lucene/spatial/prefix/tree/
