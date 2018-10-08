#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial/prefix/tree/UnitNRShape.h"

#include  "core/src/java/org/apache/lucene/spatial/prefix/tree/Cell.h"
#include  "core/src/java/org/apache/lucene/spatial/prefix/tree/NRCell.h"
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

using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Rectangle;
using org::locationtech::spatial4j::shape::Shape;
using org::locationtech::spatial4j::shape::SpatialRelation;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * A SpatialPrefixTree for single-dimensional numbers and number ranges of fixed
 * precision values (not floating point). Despite its name, the indexed values
 * (and queries) need not actually be ranges, they can be unit instance/values.
 * <p>
 * Why might you use this instead of Lucene's built-in integer/long support?
 * Here are some reasons with features based on code in this class, <em>or are
 * possible based on this class but require a subclass to fully realize it</em>.
 * <ul>
 *   <li>Index ranges, not just unit instances. This is especially useful when
 * the requirement calls for a multi-valued range.</li> <li>Instead of a fixed
 * "precisionStep", this prefixTree can have a customizable number of child
 * values for any prefix (up to 32768). This allows exact alignment of the
 * prefix-tree with typical/expected values, which results in better
 * performance.  For example in a Date implementation, every month can get its
 * own dedicated prefix, every day, etc., even though months vary in
 * duration.</li> <li>Arbitrary precision, like {@link
 * java.math.BigDecimal}.</li> <li>Standard Lucene integer/long indexing always
 * indexes the full precision of those data types but this one is
 * customizable.</li>
 * </ul>
 *
 * Unlike "normal" spatial components in this module, this special-purpose one
 * only works with {@link Shape}s created by the methods on this class, not from
 * any {@link org.locationtech.spatial4j.context.SpatialContext}.
 *
 * @see org.apache.lucene.spatial.prefix.NumberRangePrefixTreeStrategy
 * @see <a
 * href="https://issues.apache.org/jira/browse/LUCENE-5648">LUCENE-5648</a>
 * @lucene.experimental
 */
class NumberRangePrefixTree : public SpatialPrefixTree
{
  GET_CLASS_NAME(NumberRangePrefixTree)

  //
  //    Dummy SpatialContext
  //

private:
  static const std::shared_ptr<SpatialContext> DUMMY_CTX;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static NumberRangePrefixTree::StaticConstructor staticConstructor;

  /** Base interface for {@link Shape}s this prefix tree supports. It extends
   * {@link Shape} (Spatial4j) for compatibility with the spatial API even
   * though it doesn't intermix with conventional 2D shapes.
   * @lucene.experimental
   */
public:
  class NRShape : public Shape, public Cloneable
  {
    GET_CLASS_NAME(NRShape)
    /** The result should be parseable by {@link #parseShape(std::wstring)}. */
  public:
    virtual std::wstring toString() = 0;

    /** Returns this shape rounded to the target level. If we are already more
     * course than the level then the shape is simply returned.  The result may
     * refer to internal state of the argument so you may want to clone it.
     */
    virtual std::shared_ptr<NRShape> roundToLevel(int targetLevel) = 0;
  };

  //
  //  Factory / Conversions / parsing relating to NRShapes
  //

  /** Converts the value to a unit shape. Doesn't parse strings; see {@link
   * #parseShape(std::wstring)} for that. This is the reverse of {@link
   * #toObject(org.apache.lucene.spatial.prefix.tree.NumberRangePrefixTree.UnitNRShape)}.
   */
public:
  virtual std::shared_ptr<UnitNRShape> toUnitShape(std::any value) = 0;

  /** Returns a shape that represents the continuous range between {@code start}
   * and {@code end}. It will be normalized, and so sometimes a {@link
   * org.apache.lucene.spatial.prefix.tree.NumberRangePrefixTree.UnitNRShape}
   * will be returned, other times a
   * {@link
   * org.apache.lucene.spatial.prefix.tree.NumberRangePrefixTree.SpanUnitsNRShape}
   * will be.
   *
   * @throws IllegalArgumentException if the arguments are in the wrong order,
   * or if either contains the other (yet they aren't equal).
   */
  virtual std::shared_ptr<NRShape>
  toRangeShape(std::shared_ptr<UnitNRShape> startUnit,
               std::shared_ptr<UnitNRShape> endUnit);

  /** From lv.getLevel on up, it returns the first Level seen with val != 0. It
   * doesn't check past endLevel. */
private:
  int truncateStartVals(std::shared_ptr<UnitNRShape> lv, int endLevel);

  int truncateEndVals(std::shared_ptr<UnitNRShape> lv, int endLevel);

  /** Converts a UnitNRShape shape to the corresponding type supported by this
class, such as a Calendar/BigDecimal.
   * This is the reverse of {@link #toUnitShape(Object)}.
GET_CLASS_NAME(,)
   */
public:
  virtual std::any toObject(std::shared_ptr<UnitNRShape> shape) = 0;

  /** A string representation of the UnitNRShape that is parse-able by {@link
   * #parseUnitShape(std::wstring)}. */
protected:
  virtual std::wstring toString(std::shared_ptr<UnitNRShape> lv) = 0;

  static std::wstring toStringUnitRaw(std::shared_ptr<UnitNRShape> lv);

  /** Detects a range pattern and parses it, otherwise it's parsed as one shape
   * via
   * {@link #parseUnitShape(std::wstring)}.  The range pattern looks like this BNF:
   * <pre>
   *   '[' + parseShapeLV + ' TO ' + parseShapeLV + ']'
   * </pre>
   * It's the same thing as the toString() of the range shape, notwithstanding
   * range optimization.
   *
   * @param str not null or empty
   * @return not null
   * @throws java.text.ParseException If there is a problem
   */
public:
  virtual std::shared_ptr<NRShape>
  parseShape(const std::wstring &str) ;

  /** Parse a std::wstring to a UnitNRShape. "*" should be the full-range (level 0
   * shape). */
protected:
  virtual std::shared_ptr<UnitNRShape>
  parseUnitShape(const std::wstring &str) = 0;

  //
  //    UnitNRShape
  //

  /**
   * A unit value Shape implemented as a stack of numbers, one for each level in
   * the prefix tree. It directly corresponds to a {@link Cell}.  Spatially
   * speaking, it's analogous to a Point but 1D and has some precision width.
   * @lucene.experimental
   */
public:
  class UnitNRShape : public NRShape,
                      public Comparable<std::shared_ptr<UnitNRShape>>
  {
    GET_CLASS_NAME(UnitNRShape)
    // note: formerly known as LevelledValue; thus some variables still use 'lv'

    /** Get the prefix tree level, the higher the more precise. 0 means the
     * world (universe). */
  public:
    virtual int getLevel() = 0;
    /** Gets the value at the specified level of this unit. level must be &gt;=
     * 0 and &lt;= getLevel(). */
    virtual int getValAtLevel(int level) = 0;
    /** Gets an ancestor at the specified level. It shares state, so you may
     * want to clone() it. */
    virtual std::shared_ptr<UnitNRShape> getShapeAtLevel(int level) = 0;
    std::shared_ptr<UnitNRShape> roundToLevel(int targetLevel);
    override = 0;

    /** Deep clone */
    virtual std::shared_ptr<UnitNRShape> clone() = 0;
  };

  /** Compares a to b, returning less than 0, 0, or greater than 0, if a is less
   * than, equal to, or greater than b, respectively, up to their common prefix
   * (i.e. only min(a.levels,b.levels) are compared).
   * @lucene.internal */
protected:
  static int comparePrefix(std::shared_ptr<UnitNRShape> a,
                           std::shared_ptr<UnitNRShape> b);

  //
  //    SpanUnitsNRShape
  //

  /** A range Shape; based on a pair of {@link
   * org.apache.lucene.spatial.prefix.tree.NumberRangePrefixTree.UnitNRShape}.
   * Spatially speaking, it's analogous to a Rectangle but 1D. It might have
   * been named with Range in the name but it may be confusing since even the
   * {@link
   * org.apache.lucene.spatial.prefix.tree.NumberRangePrefixTree.UnitNRShape} is
   * in some sense a range.
   * @lucene.experimental */
public:
  class SpanUnitsNRShape
      : public std::enable_shared_from_this<SpanUnitsNRShape>,
        public NRShape
  {
    GET_CLASS_NAME(SpanUnitsNRShape)
  private:
    std::shared_ptr<NumberRangePrefixTree> outerInstance;

    const std::shared_ptr<UnitNRShape> minLV, maxLV;
    const int lastLevelInCommon; // computed; not part of identity

    /** Don't call directly; see
     * {@link
     * #toRangeShape(org.apache.lucene.spatial.prefix.tree.NumberRangePrefixTree.UnitNRShape,
     * org.apache.lucene.spatial.prefix.tree.NumberRangePrefixTree.UnitNRShape)}.
     */
    SpanUnitsNRShape(std::shared_ptr<NumberRangePrefixTree> outerInstance,
                     std::shared_ptr<UnitNRShape> minLV,
                     std::shared_ptr<UnitNRShape> maxLV);

  public:
    std::shared_ptr<SpatialContext> getContext() override;

    virtual std::shared_ptr<UnitNRShape> getMinUnit();

    virtual std::shared_ptr<UnitNRShape> getMaxUnit();

    /** How many levels are in common between minUnit and maxUnit, not including
     * level 0. */
  private:
    int getLevelsInCommon();

  public:
    std::shared_ptr<NRShape> roundToLevel(int targetLevel) override;

    std::shared_ptr<SpatialRelation>
    relate(std::shared_ptr<Shape> shape) override;

    virtual std::shared_ptr<SpatialRelation>
    relate(std::shared_ptr<SpanUnitsNRShape> ext);

    std::shared_ptr<Rectangle> getBoundingBox() override;
    bool hasArea() override;
    double getArea(std::shared_ptr<SpatialContext> spatialContext) override;
    std::shared_ptr<Point> getCenter() override;
    std::shared_ptr<Shape>
    getBuffered(double v,
                std::shared_ptr<SpatialContext> spatialContext) override;
    bool isEmpty() override;
    std::shared_ptr<SpanUnitsNRShape> clone() override;

    virtual std::wstring toString();

    virtual bool equals(std::any o);

    virtual int hashCode();
  }; // class SpanUnitsNRShape

  //
  //    NumberRangePrefixTree
  //

protected:
  std::deque<int> const maxSubCellsByLevel;
  std::deque<int> const termLenByLevel;
  std::deque<int> const levelByTermLen;
  const int maxTermLen; // how long could cell.getToken... (that is a leaf)
                        // possibly be?

  NumberRangePrefixTree(std::deque<int> &maxSubCellsByLevel);

public:
  virtual std::wstring toString();

  int getLevelForDistance(double dist) override;

  double getDistanceForLevel(int level) override;

protected:
  virtual std::shared_ptr<UnitNRShape> toShape(std::deque<int> &valStack,
                                               int len);

public:
  std::shared_ptr<Cell> getWorldCell() override;

protected:
  virtual std::deque<std::shared_ptr<NRCell>> newCellStack(int levels);

public:
  std::shared_ptr<Cell> readCell(std::shared_ptr<BytesRef> term,
                                 std::shared_ptr<Cell> scratch) override;

  /** Returns the number of sub-cells beneath the given UnitNRShape. */
  virtual int getNumSubCells(std::shared_ptr<UnitNRShape> lv);

  //
  //    NRCell
  //

  /** Most of the PrefixTree implementation is in this one class, which is both
   * the Cell, the CellIterator, and the Shape to reduce object allocation. It's
   * implemented as a re-used array/stack of Cells at adjacent levels, that all
   * have a reference back to the cell array to traverse. They also share a
   * common BytesRef for the term.
   * @lucene.internal */
protected:
  class NRCell : public CellIterator, public Cell, public UnitNRShape
  {
    GET_CLASS_NAME(NRCell)
  private:
    std::shared_ptr<NumberRangePrefixTree> outerInstance;

    // Shared: (TODO put this in a new class)
  public:
    std::deque<std::shared_ptr<NRCell>> const cellsByLevel;
    const std::shared_ptr<BytesRef> term; // AKA the token
    std::deque<char> termBuf; // see ensureOwnTermBytes(), only for cell0

    // Cell state...
    const int cellLevel; // assert levelStack[cellLevel] == this
    int cellNumber =
        0; // relative to parent cell. It's unused for level 0. Starts at 0.

    std::shared_ptr<SpatialRelation> cellShapeRel;
    bool cellIsLeaf = false;

    // CellIterator state is defined further below

    NRCell(std::shared_ptr<NumberRangePrefixTree> outerInstance,
           std::deque<std::shared_ptr<NRCell>> &cellsByLevel,
           std::shared_ptr<BytesRef> term, int cellLevel);

    /** Ensure we own term.bytes so that it's safe to modify. We detect via a
     * kluge in which cellsByLevel[0].termBuf is non-null, which is a
     * pre-allocated for use to replace term.bytes. */
    virtual void ensureOwnTermBytes();

  private:
    void reset();

    void resetCellWithCellNum(int cellNumber);

    void ensureDecoded();

    void assertDecoded();

  public:
    int getLevel() override;

    std::shared_ptr<SpatialRelation> getShapeRel() override;

    void setShapeRel(std::shared_ptr<SpatialRelation> rel) override;

    bool isLeaf() override;

    void setLeaf() override;

    std::shared_ptr<UnitNRShape> getShape() override;

    std::shared_ptr<BytesRef>
    getTokenBytesNoLeaf(std::shared_ptr<BytesRef> result) override;

    std::shared_ptr<BytesRef>
    getTokenBytesWithLeaf(std::shared_ptr<BytesRef> result) override;

    bool isPrefixOf(std::shared_ptr<Cell> c) override;

    int compareToNoLeaf(std::shared_ptr<Cell> fromCell) override;

    std::shared_ptr<CellIterator>
    getNextLevelCells(std::shared_ptr<Shape> shapeFilter) override;

    //----------- CellIterator

    std::shared_ptr<Shape> iterFilter; // UnitNRShape or NRShape
    bool iterFirstIsIntersects = false;
    bool iterLastIsIntersects = false;
    int iterFirstCellNumber = 0;
    int iterLastCellNumber = 0;

  private:
    void initIter(std::shared_ptr<Shape> filter);

  public:
    bool hasNext() override;

    // TODO override nextFrom to be more efficient

    //----------- UnitNRShape

    int getValAtLevel(int level) override;

    std::shared_ptr<NRCell> getShapeAtLevel(int level) override;

    std::shared_ptr<UnitNRShape> roundToLevel(int targetLevel) override;

    std::shared_ptr<SpatialRelation>
    relate(std::shared_ptr<Shape> shape) override;

    virtual std::shared_ptr<SpatialRelation>
    relate(std::shared_ptr<UnitNRShape> lv);

    virtual std::shared_ptr<SpatialRelation>
    relate(std::shared_ptr<SpanUnitsNRShape> spanShape);

    std::shared_ptr<UnitNRShape> clone() override;

    int compareTo(std::shared_ptr<UnitNRShape> o) override;

    std::shared_ptr<Rectangle> getBoundingBox() override;

    bool hasArea() override;

    double getArea(std::shared_ptr<SpatialContext> ctx) override;

    std::shared_ptr<Point> getCenter() override;

    std::shared_ptr<Shape>
    getBuffered(double distance, std::shared_ptr<SpatialContext> ctx) override;
    bool isEmpty() override;

    //------- Object

    bool equals(std::any obj) override;

    std::shared_ptr<SpatialContext> getContext() override;

    virtual int hashCode();

    virtual std::wstring toString();

    /** Configure your IDE to use this. */
    virtual std::wstring toStringDebug();

  protected:
    std::shared_ptr<NRCell> shared_from_this()
    {
      return std::static_pointer_cast<NRCell>(CellIterator::shared_from_this());
    }
  }; // END OF NRCell

protected:
  std::shared_ptr<NumberRangePrefixTree> shared_from_this()
  {
    return std::static_pointer_cast<NumberRangePrefixTree>(
        SpatialPrefixTree::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial/prefix/tree/
