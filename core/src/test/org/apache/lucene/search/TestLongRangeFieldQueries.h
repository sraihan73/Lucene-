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
#include  "core/src/java/org/apache/lucene/document/LongRange.h"

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

using LongRange = org::apache::lucene::document::LongRange;

/**
 * Random testing for LongRange Queries.
 */
// See: https://issues.apache.org/jira/browse/SOLR-12028 Tests cannot remove
// files on Windows machines occasionally
class TestLongRangeFieldQueries : public BaseRangeFieldQueryTestCase
{
  GET_CLASS_NAME(TestLongRangeFieldQueries)
private:
  static const std::wstring FIELD_NAME;

  int64_t nextLongInternal();

protected:
  std::shared_ptr<Range>
  nextRange(int dimensions)  override;

  std::shared_ptr<LongRange> newRangeField(std::shared_ptr<Range> r) override;

  std::shared_ptr<Query> newIntersectsQuery(std::shared_ptr<Range> r) override;

  std::shared_ptr<Query> newContainsQuery(std::shared_ptr<Range> r) override;

  std::shared_ptr<Query> newWithinQuery(std::shared_ptr<Range> r) override;

  std::shared_ptr<Query> newCrossesQuery(std::shared_ptr<Range> r) override;

  /** Basic test */
public:
  virtual void testBasics() ;

  /** LongRange test class implementation - use to validate LongRange */
private:
  class LongTestRange : public Range
  {
    GET_CLASS_NAME(LongTestRange)
  private:
    std::shared_ptr<TestLongRangeFieldQueries> outerInstance;

  public:
    std::deque<int64_t> min;
    std::deque<int64_t> max;

    LongTestRange(std::shared_ptr<TestLongRangeFieldQueries> outerInstance,
                  std::deque<int64_t> &min, std::deque<int64_t> &max);

  protected:
    int numDimensions() override;

    std::optional<int64_t> getMin(int dim) override;

    void setMin(int dim, std::any val) override;

    std::optional<int64_t> getMax(int dim) override;

    void setMax(int dim, std::any val) override;

    bool isEqual(std::shared_ptr<Range> other) override;

    bool isDisjoint(std::shared_ptr<Range> o) override;

    bool isWithin(std::shared_ptr<Range> o) override;

    bool contains(std::shared_ptr<Range> o) override;

  public:
    virtual std::wstring toString();

  protected:
    std::shared_ptr<LongTestRange> shared_from_this()
    {
      return std::static_pointer_cast<LongTestRange>(Range::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestLongRangeFieldQueries> shared_from_this()
  {
    return std::static_pointer_cast<TestLongRangeFieldQueries>(
        BaseRangeFieldQueryTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
