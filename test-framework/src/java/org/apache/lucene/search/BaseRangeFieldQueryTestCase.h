#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <cmath>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/document/Field.h"

#include  "core/src/java/org/apache/lucene/search/Range.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/util/FixedBitSet.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/document/Document.h"

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

using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Abstract class to do basic tests for a RangeField query. Testing rigor
 * inspired by {@code BaseGeoPointTestCase}
 */
class BaseRangeFieldQueryTestCase : public LuceneTestCase
{
  GET_CLASS_NAME(BaseRangeFieldQueryTestCase)
protected:
  virtual std::shared_ptr<Field> newRangeField(std::shared_ptr<Range> box) = 0;

  virtual std::shared_ptr<Query>
  newIntersectsQuery(std::shared_ptr<Range> box) = 0;

  virtual std::shared_ptr<Query>
  newContainsQuery(std::shared_ptr<Range> box) = 0;

  virtual std::shared_ptr<Query> newWithinQuery(std::shared_ptr<Range> box) = 0;

  virtual std::shared_ptr<Query>
  newCrossesQuery(std::shared_ptr<Range> box) = 0;

  virtual std::shared_ptr<Range> nextRange(int dimensions) = 0;

  virtual int dimension();

public:
  virtual void testRandomTiny() ;

  virtual void testRandomMedium() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Nightly public void testRandomBig() throws Exception
  virtual void testRandomBig() ;

  virtual void testMultiValued() ;

private:
  void doTestRandom(int count, bool multiValued) ;

  void verify(std::deque<std::deque<std::shared_ptr<Range>>> &ranges) throw(
      std::runtime_error);

private:
  class SimpleCollectorAnonymousInnerClass : public SimpleCollector
  {
    GET_CLASS_NAME(SimpleCollectorAnonymousInnerClass)
  private:
    std::shared_ptr<BaseRangeFieldQueryTestCase> outerInstance;

    std::shared_ptr<FixedBitSet> hits;

  public:
    SimpleCollectorAnonymousInnerClass(
        std::shared_ptr<BaseRangeFieldQueryTestCase> outerInstance,
        std::shared_ptr<FixedBitSet> hits);

  private:
    int docBase = 0;

  public:
    void collect(int doc) override;

  protected:
    void doSetNextReader(std::shared_ptr<LeafReaderContext> context) throw(
        IOException) override;

  public:
    bool needsScores() override;

  protected:
    std::shared_ptr<SimpleCollectorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SimpleCollectorAnonymousInnerClass>(
          SimpleCollector::shared_from_this());
    }
  };

protected:
  virtual void addRange(std::shared_ptr<Document> doc,
                        std::shared_ptr<Range> box);

  virtual bool expectedResult(std::shared_ptr<Range> queryRange,
                              std::deque<std::shared_ptr<Range>> &range,
                              Range::QueryType queryType);

  virtual bool expectedBBoxQueryResult(std::shared_ptr<Range> queryRange,
                                       std::shared_ptr<Range> range,
                                       Range::QueryType queryType);

  /** base class for range verification */
protected:
  class Range : public std::enable_shared_from_this<Range>
  {
    GET_CLASS_NAME(Range)
  protected:
    bool isMissing = false;

    /** supported query relations */
  protected:
    enum class QueryType {
      GET_CLASS_NAME(QueryType) INTERSECTS,
      WITHIN,
      CONTAINS,
      CROSSES
    };

  protected:
    virtual int numDimensions() = 0;
    virtual std::any getMin(int dim) = 0;
    virtual void setMin(int dim, std::any val) = 0;
    virtual std::any getMax(int dim) = 0;
    virtual void setMax(int dim, std::any val) = 0;
    virtual bool isEqual(std::shared_ptr<Range> other) = 0;
    virtual bool isDisjoint(std::shared_ptr<Range> other) = 0;
    virtual bool isWithin(std::shared_ptr<Range> other) = 0;
    virtual bool contains(std::shared_ptr<Range> other) = 0;

    virtual QueryType relate(std::shared_ptr<Range> other);
  };

protected:
  std::shared_ptr<BaseRangeFieldQueryTestCase> shared_from_this()
  {
    return std::static_pointer_cast<BaseRangeFieldQueryTestCase>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
