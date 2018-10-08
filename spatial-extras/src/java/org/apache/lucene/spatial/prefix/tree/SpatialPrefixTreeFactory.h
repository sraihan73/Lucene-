#pragma once
#include "stringhelper.h"
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/spatial/prefix/tree/SpatialPrefixTree.h"

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

/**
 * Abstract Factory for creating {@link SpatialPrefixTree} instances with useful
 * defaults and passed on configurations defined in a Map.
 *
 * @lucene.experimental
 */
class SpatialPrefixTreeFactory
    : public std::enable_shared_from_this<SpatialPrefixTreeFactory>
{
  GET_CLASS_NAME(SpatialPrefixTreeFactory)

private:
  static constexpr double DEFAULT_GEO_MAX_DETAIL_KM = 0.001; // 1m
public:
  static const std::wstring PREFIX_TREE;
  static const std::wstring MAX_LEVELS;
  static const std::wstring MAX_DIST_ERR;

protected:
  std::unordered_map<std::wstring, std::wstring> args;
  std::shared_ptr<SpatialContext> ctx;
  std::optional<int> maxLevels;

  /**
   * The factory  is looked up via "prefixTree" in args, expecting "geohash" or
   * "quad". If it's neither of these, then "geohash" is chosen for a geo
   * context, otherwise "quad" is chosen.
   */
public:
  static std::shared_ptr<SpatialPrefixTree>
  makeSPT(std::unordered_map<std::wstring, std::wstring> &args,
          std::shared_ptr<ClassLoader> classLoader,
          std::shared_ptr<SpatialContext> ctx);

protected:
  virtual void init(std::unordered_map<std::wstring, std::wstring> &args,
                    std::shared_ptr<SpatialContext> ctx);

  virtual void initMaxLevels();

  /** Calls {@link SpatialPrefixTree#getLevelForDistance(double)}. */
  virtual int getLevelForDistance(double degrees) = 0;

  virtual std::shared_ptr<SpatialPrefixTree> newSPT() = 0;
};

} // #include  "core/src/java/org/apache/lucene/spatial/prefix/tree/
