#pragma once
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class Weight;
}

namespace org::apache::lucene::search
{
class DocIdSetIterator;
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

class TestPositiveScoresOnlyCollector : public LuceneTestCase
{
  GET_CLASS_NAME(TestPositiveScoresOnlyCollector)

private:
  class SimpleScorer final : public Scorer
  {
    GET_CLASS_NAME(SimpleScorer)
  private:
    int idx = -1;

  public:
    SimpleScorer(std::shared_ptr<Weight> weight);

    float score() override;

    int docID() override;
    std::shared_ptr<DocIdSetIterator> iterator() override;

  private:
    class DocIdSetIteratorAnonymousInnerClass : public DocIdSetIterator
    {
      GET_CLASS_NAME(DocIdSetIteratorAnonymousInnerClass)
    private:
      std::shared_ptr<SimpleScorer> outerInstance;

    public:
      DocIdSetIteratorAnonymousInnerClass(
          std::shared_ptr<SimpleScorer> outerInstance);

      int docID() override;

      int nextDoc() override;

      int advance(int target) override;

      int64_t cost() override;

    protected:
      std::shared_ptr<DocIdSetIteratorAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<DocIdSetIteratorAnonymousInnerClass>(
            DocIdSetIterator::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<SimpleScorer> shared_from_this()
    {
      return std::static_pointer_cast<SimpleScorer>(Scorer::shared_from_this());
    }
  };

  // The scores must have positive as well as negative values
private:
  static std::deque<float> const scores;

public:
  virtual void testNegativeScores() ;

protected:
  std::shared_ptr<TestPositiveScoresOnlyCollector> shared_from_this()
  {
    return std::static_pointer_cast<TestPositiveScoresOnlyCollector>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
