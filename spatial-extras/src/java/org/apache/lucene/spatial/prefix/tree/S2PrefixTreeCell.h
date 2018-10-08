#pragma once
#include "stringhelper.h"
#include <any>
#include <cmath>
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial/prefix/tree/S2PrefixTree.h"

#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
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

using com::google::common::geometry::S2CellId;
using BytesRef = org::apache::lucene::util::BytesRef;
using org::locationtech::spatial4j::shape::Shape;
using org::locationtech::spatial4j::shape::SpatialRelation;

/**
 * This class represents a S2 pixel in the RPT.
 *
 * @lucene.internal
 */
class S2PrefixTreeCell : public std::enable_shared_from_this<S2PrefixTreeCell>,
                         public CellCanPrune
{
  GET_CLASS_NAME(S2PrefixTreeCell)

  // Faces of S2 Geometry
private:
  static std::deque<std::shared_ptr<S2CellId>> FACES;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static S2PrefixTreeCell::StaticConstructor staticConstructor;

  /*Special character to define a cell leaf*/
  static constexpr char LEAF = L'+';
  /*Tokens are used to serialize cells*/
  static std::deque<char> const TOKENS;
  /*Map containing mapping between tokens and integer values*/
  static const std::unordered_map<char, int> PIXELS;

public:
  std::shared_ptr<S2CellId> cellId;
  int level = 0; // cache level
  std::shared_ptr<S2PrefixTree> tree;

  std::shared_ptr<SpatialRelation> shapeRel = nullptr;
  // C++ NOTE: Fields cannot have the same name as methods:
  bool isLeaf_ = false;
  std::shared_ptr<Shape> shape = nullptr;

  S2PrefixTreeCell(std::shared_ptr<S2PrefixTree> tree,
                   std::shared_ptr<S2CellId> cellId);

  virtual void readCell(std::shared_ptr<S2PrefixTree> tree,
                        std::shared_ptr<BytesRef> ref);

  std::shared_ptr<SpatialRelation> getShapeRel() override;

  void setShapeRel(std::shared_ptr<SpatialRelation> rel) override;

  bool isLeaf() override;

  void setLeaf() override;

  std::shared_ptr<BytesRef>
  getTokenBytesWithLeaf(std::shared_ptr<BytesRef> result) override;

  std::shared_ptr<BytesRef>
  getTokenBytesNoLeaf(std::shared_ptr<BytesRef> result) override;

  int getLevel() override;

  /**
   * Cache level of cell.
   */
private:
  void setLevel();

public:
  std::shared_ptr<CellIterator>
  getNextLevelCells(std::shared_ptr<Shape> shapeFilter) override;

  std::shared_ptr<Shape> getShape() override;

  bool isPrefixOf(std::shared_ptr<Cell> c) override;

  int compareToNoLeaf(std::shared_ptr<Cell> fromCell) override;

  /**
   * Check if a cell is a leaf.
   *
   * @param ref The Byteref of the leaf
   * @return true if it is a leaf, e.g last byte is the special Character.
   */
private:
  bool isLeaf(std::shared_ptr<BytesRef> ref);

  /**
   * Get the {@link S2CellId} from the {@link BytesRef} representation.
   *
   * @param ref The bytes.
   * @return the corresponding S2 cell.
   */
  std::shared_ptr<S2CellId>
  getS2CellIdFromBytesRef(std::shared_ptr<BytesRef> ref);

  /**
   * Codify a {@link S2CellId} into its {@link BytesRef} representation.
   *
   * @param cellId The S2 Cell id to codify.
   * @param bref   The byteref representation.
   */
  void getBytesRefFromS2CellId(std::shared_ptr<S2CellId> cellId,
                               std::shared_ptr<BytesRef> bref);

public:
  int getSubCellsSize() override;

  virtual int hashCode();

  virtual bool equals(std::any o);

  virtual std::wstring toString();
};
} // #include  "core/src/java/org/apache/lucene/spatial/prefix/tree/
