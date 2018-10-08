#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::spatial::prefix
{
class RecursivePrefixTreeStrategy;
}

namespace org::apache::lucene::spatial::prefix::tree
{
class SpatialPrefixTree;
}
namespace org::apache::lucene::spatial::prefix
{
class CellToBytesRefIterator;
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
namespace org::apache::lucene::spatial::prefix
{

/** Test RandomSpatialOpFuzzyPrefixTreeTest using the PrefixTree index format
 * found in 5.0 and prior. */
class RandomSpatialOpFuzzyPrefixTree50Test
    : public RandomSpatialOpFuzzyPrefixTreeTest
{
  GET_CLASS_NAME(RandomSpatialOpFuzzyPrefixTree50Test)

protected:
  std::shared_ptr<RecursivePrefixTreeStrategy> newRPT() override;

private:
  class RecursivePrefixTreeStrategyAnonymousInnerClass
      : public RecursivePrefixTreeStrategy
  {
    GET_CLASS_NAME(RecursivePrefixTreeStrategyAnonymousInnerClass)
  private:
    std::shared_ptr<RandomSpatialOpFuzzyPrefixTree50Test> outerInstance;

  public:
    RecursivePrefixTreeStrategyAnonymousInnerClass(
        std::shared_ptr<RandomSpatialOpFuzzyPrefixTree50Test> outerInstance,
        std::shared_ptr<
            org::apache::lucene::spatial::prefix::tree::SpatialPrefixTree>
            grid,
        std::shared_ptr<UnknownType> getSimpleName);

  protected:
    std::shared_ptr<CellToBytesRefIterator>
    newCellToBytesRefIterator() override;

  protected:
    std::shared_ptr<RecursivePrefixTreeStrategyAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          RecursivePrefixTreeStrategyAnonymousInnerClass>(
          RecursivePrefixTreeStrategy::shared_from_this());
    }
  };

protected:
  std::shared_ptr<RandomSpatialOpFuzzyPrefixTree50Test> shared_from_this()
  {
    return std::static_pointer_cast<RandomSpatialOpFuzzyPrefixTree50Test>(
        RandomSpatialOpFuzzyPrefixTreeTest::shared_from_this());
  }
};

} // namespace org::apache::lucene::spatial::prefix
