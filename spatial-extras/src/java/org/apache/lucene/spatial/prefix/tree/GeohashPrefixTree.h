#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

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
namespace org::apache::lucene::spatial::prefix::tree
{

using org::locationtech::spatial4j::context::SpatialContext;
using org::locationtech::spatial4j::shape::Point;
using org::locationtech::spatial4j::shape::Shape;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * A {@link SpatialPrefixTree} based on
 * <a href="http://en.wikipedia.org/wiki/Geohash">Geohashes</a>.
 * Uses {@link GeohashUtils} to do all the geohash work.
 *
 * @lucene.experimental
 */
class GeohashPrefixTree : public LegacyPrefixTree
{
  GET_CLASS_NAME(GeohashPrefixTree)

  /**
   * Factory for creating {@link GeohashPrefixTree} instances with useful
   * defaults
   */
public:
  class Factory : public SpatialPrefixTreeFactory
  {
    GET_CLASS_NAME(Factory)

  protected:
    int getLevelForDistance(double degrees) override;

    std::shared_ptr<SpatialPrefixTree> newSPT() override;

  protected:
    std::shared_ptr<Factory> shared_from_this()
    {
      return std::static_pointer_cast<Factory>(
          SpatialPrefixTreeFactory::shared_from_this());
    }
  };

public:
  GeohashPrefixTree(std::shared_ptr<SpatialContext> ctx, int maxLevels);

  /** Any more than this and there's no point (double lat and lon are the same).
   */
  static int getMaxLevelsPossible();

  std::shared_ptr<Cell> getWorldCell() override;

  int getLevelForDistance(double dist) override;

protected:
  std::shared_ptr<Cell> getCell(std::shared_ptr<Point> p, int level) override;

private:
  static std::deque<char> stringToBytesPlus1(const std::wstring &token);

private:
  class GhCell : public LegacyCell
  {
    GET_CLASS_NAME(GhCell)
  private:
    std::shared_ptr<GeohashPrefixTree> outerInstance;

    std::wstring geohash; // cache; never has leaf byte, simply a geohash

  public:
    GhCell(std::shared_ptr<GeohashPrefixTree> outerInstance,
           const std::wstring &geohash);

    GhCell(std::shared_ptr<GeohashPrefixTree> outerInstance,
           std::deque<char> &bytes, int off, int len);

  protected:
    std::shared_ptr<GeohashPrefixTree> getGrid() override;
    int getMaxLevels() override;
    void readCell(std::shared_ptr<BytesRef> bytesRef) override;

  public:
    std::shared_ptr<std::deque<std::shared_ptr<Cell>>> getSubCells() override;

    int getSubCellsSize() override;

  protected:
    std::shared_ptr<GhCell> getSubCell(std::shared_ptr<Point> p) override;

  public:
    std::shared_ptr<Shape> getShape() override;

  private:
    std::wstring getGeohash();

  protected:
    std::shared_ptr<GhCell> shared_from_this()
    {
      return std::static_pointer_cast<GhCell>(LegacyCell::shared_from_this());
    }
  }; // class GhCell

protected:
  std::shared_ptr<GeohashPrefixTree> shared_from_this()
  {
    return std::static_pointer_cast<GeohashPrefixTree>(
        LegacyPrefixTree::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial::prefix::tree
