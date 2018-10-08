#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial/prefix/tree/SpatialPrefixTree.h"

#include  "core/src/java/org/apache/lucene/spatial/prefix/tree/Cell.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/spatial/prefix/tree/CellIterator.h"

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

using BytesRef = org::apache::lucene::util::BytesRef;
using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Rectangle;
using org::locationtech::spatial4j::shape::Shape;
using org::locationtech::spatial4j::shape::SpatialRelation;

/**
 * Uses a compact binary representation of 8 bytes to encode a spatial quad
 * trie.
 *
 * The binary representation is as follows:
 * <pre>
 * CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCDDDDDL
 *
 * Where C = Cell bits (2 per quad)
 *       D = Depth bits (5 with max of 29 levels)
 *       L = isLeaf bit
 * </pre>
 *
 * It includes a built-in "pruneLeafyBranches" setting (true by default) similar
 * to
 * {@link
 * org.apache.lucene.spatial.prefix.RecursivePrefixTreeStrategy#setPruneLeafyBranches(bool)}
 * although this one only prunes at the target detail level (where it has the
 * most effect).  Usually you should disable RPT's prune, since it is very
 * memory in-efficient.
 *
 * @lucene.experimental
 */
class PackedQuadPrefixTree : public QuadPrefixTree
{
  GET_CLASS_NAME(PackedQuadPrefixTree)
public:
  static constexpr int MAX_LEVELS_POSSIBLE = 29;

protected:
  static std::deque<char> const QUAD;

  bool leafyPrune = true;

  /**
   * Factory for creating {@link PackedQuadPrefixTree} instances with useful
   * defaults.
   */
public:
  class Factory : public QuadPrefixTree::Factory
  {
    GET_CLASS_NAME(Factory)
  protected:
    std::shared_ptr<SpatialPrefixTree> newSPT() override;

  protected:
    std::shared_ptr<Factory> shared_from_this()
    {
      return std::static_pointer_cast<Factory>(
          QuadPrefixTree.Factory::shared_from_this());
    }
  };

public:
  PackedQuadPrefixTree(std::shared_ptr<SpatialContext> ctx, int maxLevels);

  virtual std::wstring toString();

  std::shared_ptr<Cell> getWorldCell() override;

  std::shared_ptr<Cell> getCell(std::shared_ptr<Point> p, int level) override;

protected:
  virtual void build(double x, double y, int level,
                     std::deque<std::shared_ptr<Cell>> &matches,
                     int64_t term, std::shared_ptr<Shape> shape,
                     int maxLevel);

  virtual void checkBattenberg(char quad, double cx, double cy, int level,
                               std::deque<std::shared_ptr<Cell>> &matches,
                               int64_t term, std::shared_ptr<Shape> shape,
                               int maxLevel);

public:
  std::shared_ptr<Cell> readCell(std::shared_ptr<BytesRef> term,
                                 std::shared_ptr<Cell> scratch) override;

  std::shared_ptr<CellIterator>
  getTreeCellIterator(std::shared_ptr<Shape> shape, int detailLevel) override;

  virtual bool isPruneLeafyBranches();

  /** Like {@link
   * org.apache.lucene.spatial.prefix.RecursivePrefixTreeStrategy#setPruneLeafyBranches(bool)}
   * but more memory efficient and only applies to the detailLevel, where it has
   * the most effect. */
  virtual void setPruneLeafyBranches(bool pruneLeafyBranches);

  /** See binary representation in the javadocs of {@link PackedQuadPrefixTree}.
   */
protected:
  class PackedQuadCell : public QuadCell
  {
    GET_CLASS_NAME(PackedQuadCell)
  private:
    std::shared_ptr<PackedQuadPrefixTree> outerInstance;

    int64_t term = 0;

  public:
    PackedQuadCell(std::shared_ptr<PackedQuadPrefixTree> outerInstance,
                   int64_t term);

    PackedQuadCell(std::shared_ptr<PackedQuadPrefixTree> outerInstance,
                   int64_t term, std::shared_ptr<SpatialRelation> shapeRel);

  protected:
    void readCell(std::shared_ptr<BytesRef> bytes) override;

  private:
    int getShiftForLevel(int const level);

  public:
    virtual bool isEnd(int const level, int const shift);

    /**
     * Get the next cell in the tree without using recursion. descend parameter
     * requests traversal to the child nodes, setting this to false will step to
     * the next sibling. Note: This complies with lexicographical ordering, once
     * you've moved to the next sibling there is no backtracking.
     */
    virtual std::shared_ptr<PackedQuadCell> nextCell(bool descend);

  protected:
    void readLeafAdjust() override;

  public:
    std::shared_ptr<BytesRef>
    getTokenBytesWithLeaf(std::shared_ptr<BytesRef> result) override;

    std::shared_ptr<BytesRef>
    getTokenBytesNoLeaf(std::shared_ptr<BytesRef> result) override;

    int compareToNoLeaf(std::shared_ptr<Cell> fromCell) override;

    int getLevel() override;

  protected:
    std::shared_ptr<std::deque<std::shared_ptr<Cell>>> getSubCells() override;

    std::shared_ptr<QuadCell> getSubCell(std::shared_ptr<Point> p) override;

  public:
    bool isPrefixOf(std::shared_ptr<Cell> c) override;

  protected:
    virtual bool isInternalPrefix(std::shared_ptr<PackedQuadCell> c);

    virtual int64_t concat(char postfix);

    /**
     * Constructs a bounding box shape out of the encoded cell
     */
    std::shared_ptr<Rectangle> makeShape() override;

  private:
    int64_t fromBytes(char b1, char b2, char b3, char b4, char b5, char b6,
                        char b7, char b8);

    std::deque<char> longToByteArray(int64_t value,
                                      std::deque<char> &result);

    int64_t longFromByteArray(std::deque<char> &bytes, int ofs);

    /**
     * Used for debugging, this will print the bits of the cell
     */
  public:
    virtual std::wstring toString();

  protected:
    std::shared_ptr<PackedQuadCell> shared_from_this()
    {
      return std::static_pointer_cast<PackedQuadCell>(
          QuadCell::shared_from_this());
    }
  }; // PackedQuadCell

  /** This is a streamlined version of TreeCellIterator, with built-in support
   * to prune at detailLevel (but not recursively upwards). */
protected:
  class PrefixTreeIterator : public CellIterator
  {
    GET_CLASS_NAME(PrefixTreeIterator)
  private:
    std::shared_ptr<PackedQuadPrefixTree> outerInstance;

    std::shared_ptr<Shape> shape;
    std::shared_ptr<PackedQuadCell> thisCell_;
    std::shared_ptr<PackedQuadCell> nextCell;

    short level = 0;
    const short detailLevel;
    std::shared_ptr<CellIterator> pruneIter;

  public:
    PrefixTreeIterator(std::shared_ptr<PackedQuadPrefixTree> outerInstance,
                       std::shared_ptr<Shape> shape, short detailLevel);

    bool hasNext() override;

  private:
    bool pruned(std::shared_ptr<SpatialRelation> rel);

  public:
    std::shared_ptr<Cell> next() override;

    void remove() override;

  protected:
    std::shared_ptr<PrefixTreeIterator> shared_from_this()
    {
      return std::static_pointer_cast<PrefixTreeIterator>(
          CellIterator::shared_from_this());
    }
  };

protected:
  std::shared_ptr<PackedQuadPrefixTree> shared_from_this()
  {
    return std::static_pointer_cast<PackedQuadPrefixTree>(
        QuadPrefixTree::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial/prefix/tree/
