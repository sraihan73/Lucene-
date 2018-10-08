#pragma once
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class IndexSearcher;
}

namespace org::apache::lucene::search
{
class Weight;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::search
{
class Scorer;
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
namespace org::apache::lucene::search
{

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestUsageTrackingFilterCachingPolicy : public LuceneTestCase
{
  GET_CLASS_NAME(TestUsageTrackingFilterCachingPolicy)

public:
  virtual void testCostlyFilter();

  virtual void testNeverCacheMatchAll() ;

  virtual void testNeverCacheTermFilter() ;

  virtual void testBooleanQueries() ;

private:
  class DummyQuery : public Query
  {
    GET_CLASS_NAME(DummyQuery)

  private:
    const int id;

  public:
    DummyQuery(int id);

    std::wstring toString(const std::wstring &field) override;

    bool equals(std::any obj) override;

    virtual int hashCode();

    std::shared_ptr<Weight>
    createWeight(std::shared_ptr<IndexSearcher> searcher, bool needsScores,
                 float boost)  override;

  private:
    class ConstantScoreWeightAnonymousInnerClass : public ConstantScoreWeight
    {
      GET_CLASS_NAME(ConstantScoreWeightAnonymousInnerClass)
    private:
      std::shared_ptr<DummyQuery> outerInstance;

    public:
      // C++ TODO: You cannot use 'shared_from_this' in a constructor:
      ConstantScoreWeightAnonymousInnerClass(
          std::shared_ptr<DummyQuery> outerInstance,
          std::shared_ptr<org::apache::lucene::search::
                              TestUsageTrackingFilterCachingPolicy::DummyQuery>
              shared_from_this(),
          float boost);

      std::shared_ptr<Scorer>
      scorer(std::shared_ptr<LeafReaderContext> context) throw(
          IOException) override;

      bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

    protected:
      std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<ConstantScoreWeightAnonymousInnerClass>(
            ConstantScoreWeight::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<DummyQuery> shared_from_this()
    {
      return std::static_pointer_cast<DummyQuery>(Query::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestUsageTrackingFilterCachingPolicy> shared_from_this()
  {
    return std::static_pointer_cast<TestUsageTrackingFilterCachingPolicy>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
