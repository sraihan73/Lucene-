#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/DocIdSetIterator.h"

#include  "core/src/java/org/apache/lucene/search/FakeScorer.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"

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

class TestBoolean2ScorerSupplier : public LuceneTestCase
{
  GET_CLASS_NAME(TestBoolean2ScorerSupplier)

private:
  class FakeScorer : public Scorer
  {
    GET_CLASS_NAME(FakeScorer)

  private:
    const std::shared_ptr<DocIdSetIterator> it;

  public:
    FakeScorer(int64_t cost);

    int docID() override;

    float score()  override;

    std::shared_ptr<DocIdSetIterator> iterator() override;

    virtual std::wstring toString();

  protected:
    std::shared_ptr<FakeScorer> shared_from_this()
    {
      return std::static_pointer_cast<FakeScorer>(Scorer::shared_from_this());
    }
  };

private:
  class FakeScorerSupplier : public ScorerSupplier
  {
    GET_CLASS_NAME(FakeScorerSupplier)

  private:
    // C++ NOTE: Fields cannot have the same name as methods:
    const int64_t cost_;
    const std::optional<int64_t> leadCost;

  public:
    FakeScorerSupplier(int64_t cost);

    FakeScorerSupplier(int64_t cost, int64_t leadCost);

    std::shared_ptr<Scorer> get(int64_t leadCost)  override;

    int64_t cost() override;

    virtual std::wstring toString();

  protected:
    std::shared_ptr<FakeScorerSupplier> shared_from_this()
    {
      return std::static_pointer_cast<FakeScorerSupplier>(
          ScorerSupplier::shared_from_this());
    }
  };

public:
  virtual void testConjunctionCost();

  virtual void testDisjunctionCost() ;

  virtual void testDisjunctionWithMinShouldMatchCost() ;

  virtual void testDuelCost() ;

  // test the tester...
  virtual void testFakeScorerSupplier();

  virtual void testConjunctionLeadCost() ;

  virtual void testDisjunctionLeadCost() ;

  virtual void testDisjunctionWithMinShouldMatchLeadCost() ;

  virtual void testProhibitedLeadCost() ;

  virtual void testMixedLeadCost() ;

protected:
  std::shared_ptr<TestBoolean2ScorerSupplier> shared_from_this()
  {
    return std::static_pointer_cast<TestBoolean2ScorerSupplier>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
