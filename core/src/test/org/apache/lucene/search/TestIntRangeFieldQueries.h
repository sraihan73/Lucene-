#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/document/IntRange.h"

#include  "core/src/java/org/apache/lucene/search/Query.h"

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
namespace org::apache::lucene::search
{

using IntRange = org::apache::lucene::document::IntRange;

/**
 * Random testing for IntRange Queries.
 */
class TestIntRangeFieldQueries : public BaseRangeFieldQueryTestCase
{
  GET_CLASS_NAME(TestIntRangeFieldQueries)
private:
  static const std::wstring FIELD_NAME;

  int nextIntInternal();

protected:
  std::shared_ptr<Range>
  nextRange(int dimensions)  override;

  std::shared_ptr<IntRange> newRangeField(std::shared_ptr<Range> r) override;

  std::shared_ptr<Query> newIntersectsQuery(std::shared_ptr<Range> r) override;

  std::shared_ptr<Query> newContainsQuery(std::shared_ptr<Range> r) override;

  std::shared_ptr<Query> newWithinQuery(std::shared_ptr<Range> r) override;

  std::shared_ptr<Query> newCrossesQuery(std::shared_ptr<Range> r) override;

  /** Basic test */
public:
  virtual void testBasics() ;

  /** IntRange test class implementation - use to validate IntRange */
protected:
  class IntTestRange : public Range
  {
    GET_CLASS_NAME(IntTestRange)
  private:
    std::shared_ptr<TestIntRangeFieldQueries> outerInstance;

  public:
    std::deque<int> min;
    std::deque<int> max;

    IntTestRange(std::shared_ptr<TestIntRangeFieldQueries> outerInstance,
                 std::deque<int> &min, std::deque<int> &max);

  protected:
    int numDimensions() override;

    std::optional<int> getMin(int dim) override;

    void setMin(int dim, std::any val) override;

    std::optional<int> getMax(int dim) override;

    void setMax(int dim, std::any val) override;

    bool isEqual(std::shared_ptr<Range> other) override;

    bool isDisjoint(std::shared_ptr<Range> o) override;

    bool isWithin(std::shared_ptr<Range> o) override;

    bool contains(std::shared_ptr<Range> o) override;

  public:
    virtual std::wstring toString();

  protected:
    std::shared_ptr<IntTestRange> shared_from_this()
    {
      return std::static_pointer_cast<IntTestRange>(Range::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestIntRangeFieldQueries> shared_from_this()
  {
    return std::static_pointer_cast<TestIntRangeFieldQueries>(
        BaseRangeFieldQueryTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
