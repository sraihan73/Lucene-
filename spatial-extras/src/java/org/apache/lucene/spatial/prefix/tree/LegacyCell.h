#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

#include  "core/src/java/org/apache/lucene/spatial/prefix/tree/SpatialPrefixTree.h"
#include  "core/src/java/org/apache/lucene/spatial/prefix/tree/CellIterator.h"
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

using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Shape;
using org::locationtech::spatial4j::shape::SpatialRelation;
using BytesRef = org::apache::lucene::util::BytesRef;

/** The base for the original two SPT's: Geohash and Quad. Don't subclass this
 * for new SPTs.
 * @lucene.internal */
// public for RPT pruneLeafyBranches code
class LegacyCell : public std::enable_shared_from_this<LegacyCell>,
                   public CellCanPrune
{
  GET_CLASS_NAME(LegacyCell)

  // Important: A LegacyCell doesn't share state for getNextLevelCells(), and
  //  LegacySpatialPrefixTree assumes this in its simplify tree logic.

private:
  static constexpr char LEAF_BYTE =
      L'+'; // NOTE: must sort before letters & numbers

  // Arguably we could simply use a BytesRef, using an extra Object.
protected:
  std::deque<char> bytes; // generally bigger to potentially hold a leaf
  int b_off = 0;
  int b_len = 0; // doesn't reflect leaf; same as getLevel()

  // C++ NOTE: Fields cannot have the same name as methods:
  bool isLeaf_ = false;

  /**
   * When set via getSubCells(filter), it is the relationship between this cell
   * and the given shape filter. Doesn't participate in shape equality.
   */
  std::shared_ptr<SpatialRelation> shapeRel;

  std::shared_ptr<Shape> shape; // cached

  /** Warning: Refers to the same bytes (no copy). If {@link #setLeaf()} is
   * subsequently called then it may modify bytes. */
  LegacyCell(std::deque<char> &bytes, int off, int len);

  virtual void readCell(std::shared_ptr<BytesRef> bytes);

  virtual void readLeafAdjust();

  virtual std::shared_ptr<SpatialPrefixTree> getGrid() = 0;

  virtual int getMaxLevels() = 0;

public:
  std::shared_ptr<SpatialRelation> getShapeRel() override;

  void setShapeRel(std::shared_ptr<SpatialRelation> rel) override;

  bool isLeaf() override;

  void setLeaf() override;

  std::shared_ptr<BytesRef>
  getTokenBytesWithLeaf(std::shared_ptr<BytesRef> result) override;

  std::shared_ptr<BytesRef>
  getTokenBytesNoLeaf(std::shared_ptr<BytesRef> result) override;

  int getLevel() override;

  std::shared_ptr<CellIterator>
  getNextLevelCells(std::shared_ptr<Shape> shapeFilter) override;

  /**
   * Performant implementations are expected to implement this efficiently by
   * considering the current cell's boundary.
   * <p>
   * Precondition: Never called when getLevel() == maxLevel.
   * Precondition: this.getShape().relate(p) != DISJOINT.
   */
protected:
  virtual std::shared_ptr<LegacyCell> getSubCell(std::shared_ptr<Point> p) = 0;

  /**
   * Gets the cells at the next grid cell level that covers this cell.
   * Precondition: Never called when getLevel() == maxLevel.
   *
   * @return A set of cells (no dups), sorted, modifiable, not empty, not null.
   */
  virtual std::shared_ptr<std::deque<std::shared_ptr<Cell>>> getSubCells() = 0;

public:
  bool isPrefixOf(std::shared_ptr<Cell> c) override;

  /** Copied from {@link
   * org.apache.lucene.util.StringHelper#startsWith(BytesRef, BytesRef)} which
   * calls this. This is to avoid creating a BytesRef.  */
private:
  static bool sliceEquals(std::deque<char> &sliceToTest_bytes,
                          int sliceToTest_offset, int sliceToTest_length,
                          std::deque<char> &other_bytes, int other_offset,
                          int other_length);

public:
  int compareToNoLeaf(std::shared_ptr<Cell> fromCell) override;

  /** Copied from {@link BytesRef#compareTo(BytesRef)}.
   * This is to avoid creating a BytesRef. */
protected:
  static int compare(std::deque<char> &aBytes, int aUpto, int a_length,
                     std::deque<char> &bBytes, int bUpto, int b_length);

public:
  bool equals(std::any obj) override;

  virtual int hashCode();

  virtual std::wstring toString();
};

} // #include  "core/src/java/org/apache/lucene/spatial/prefix/tree/
