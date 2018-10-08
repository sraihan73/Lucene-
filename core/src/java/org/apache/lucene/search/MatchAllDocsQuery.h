#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"

#include  "core/src/java/org/apache/lucene/search/Weight.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"
#include  "core/src/java/org/apache/lucene/search/BulkScorer.h"
#include  "core/src/java/org/apache/lucene/search/LeafCollector.h"
#include  "core/src/java/org/apache/lucene/util/Bits.h"

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

/**
 * A query that matches all documents.
 *
 */
class MatchAllDocsQuery final : public Query
{
  GET_CLASS_NAME(MatchAllDocsQuery)

public:
  std::shared_ptr<Weight> createWeight(std::shared_ptr<IndexSearcher> searcher,
                                       bool needsScores, float boost) override;

private:
  class ConstantScoreWeightAnonymousInnerClass : public ConstantScoreWeight
  {
    GET_CLASS_NAME(ConstantScoreWeightAnonymousInnerClass)
  private:
    std::shared_ptr<MatchAllDocsQuery> outerInstance;

  public:
    ConstantScoreWeightAnonymousInnerClass(
        std::shared_ptr<MatchAllDocsQuery> outerInstance, float boost);

    virtual std::wstring toString();
    std::shared_ptr<Scorer> scorer(
        std::shared_ptr<LeafReaderContext> context)  override;

    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

    std::shared_ptr<BulkScorer> bulkScorer(
        std::shared_ptr<LeafReaderContext> context)  override;

  private:
    class BulkScorerAnonymousInnerClass : public BulkScorer
    {
      GET_CLASS_NAME(BulkScorerAnonymousInnerClass)
    private:
      std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance;

      float score = 0;
      int maxDoc = 0;

    public:
      BulkScorerAnonymousInnerClass(
          std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance,
          float score, int maxDoc);

      int score(std::shared_ptr<LeafCollector> collector,
                std::shared_ptr<Bits> acceptDocs, int min,
                int max)  override;
      int64_t cost() override;

    protected:
      std::shared_ptr<BulkScorerAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<BulkScorerAnonymousInnerClass>(
            BulkScorer::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ConstantScoreWeightAnonymousInnerClass>(
          ConstantScoreWeight::shared_from_this());
    }
  };

public:
  std::wstring toString(const std::wstring &field) override;

  virtual bool equals(std::any o);

  virtual int hashCode();

protected:
  std::shared_ptr<MatchAllDocsQuery> shared_from_this()
  {
    return std::static_pointer_cast<MatchAllDocsQuery>(
        Query::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
