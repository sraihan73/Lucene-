#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/document/FeatureField.h"

#include  "core/src/java/org/apache/lucene/document/FeatureFunction.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/Weight.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/search/Explanation.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"
#include  "core/src/java/org/apache/lucene/search/similarities/Similarity.h"
#include  "core/src/java/org/apache/lucene/search/similarities/SimScorer.h"
#include  "core/src/java/org/apache/lucene/index/PostingsEnum.h"
#include  "core/src/java/org/apache/lucene/search/DocIdSetIterator.h"

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
namespace org::apache::lucene::document
{

using FeatureFunction =
    org::apache::lucene::document::FeatureField::FeatureFunction;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Weight = org::apache::lucene::search::Weight;

class FeatureQuery final : public Query
{
  GET_CLASS_NAME(FeatureQuery)

private:
  const std::wstring fieldName;
  const std::wstring featureName;
  const std::shared_ptr<FeatureFunction> function;

public:
  FeatureQuery(const std::wstring &fieldName, const std::wstring &featureName,
               std::shared_ptr<FeatureFunction> function);

  std::shared_ptr<Query>
  rewrite(std::shared_ptr<IndexReader> reader)  override;

  bool equals(std::any obj) override;

  virtual int hashCode();

  std::shared_ptr<Weight> createWeight(std::shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost)  override;

private:
  class WeightAnonymousInnerClass : public Weight
  {
    GET_CLASS_NAME(WeightAnonymousInnerClass)
  private:
    std::shared_ptr<FeatureQuery> outerInstance;

    bool needsScores = false;
    float boost = 0;

  public:
    WeightAnonymousInnerClass(std::shared_ptr<FeatureQuery> outerInstance,
                              bool needsScores, float boost);

    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

    void
    extractTerms(std::shared_ptr<Set<std::shared_ptr<Term>>> terms) override;

    std::shared_ptr<Explanation>
    explain(std::shared_ptr<LeafReaderContext> context,
            int doc)  override;

    std::shared_ptr<Scorer> scorer(
        std::shared_ptr<LeafReaderContext> context)  override;

  private:
    class ScorerAnonymousInnerClass : public Scorer
    {
      GET_CLASS_NAME(ScorerAnonymousInnerClass)
    private:
      std::shared_ptr<WeightAnonymousInnerClass> outerInstance;

      std::shared_ptr<SimScorer> scorer;
      std::shared_ptr<PostingsEnum> postings;

    public:
      ScorerAnonymousInnerClass(
          std::shared_ptr<WeightAnonymousInnerClass> outerInstance,
          std::shared_ptr<SimScorer> scorer,
          std::shared_ptr<PostingsEnum> postings);

      int docID() override;

      float score()  override;

      std::shared_ptr<DocIdSetIterator> iterator() override;

    protected:
      std::shared_ptr<ScorerAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<ScorerAnonymousInnerClass>(
            org.apache.lucene.search.Scorer::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<WeightAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<WeightAnonymousInnerClass>(
          org.apache.lucene.search.Weight::shared_from_this());
    }
  };

public:
  std::wstring toString(const std::wstring &field) override;

protected:
  std::shared_ptr<FeatureQuery> shared_from_this()
  {
    return std::static_pointer_cast<FeatureQuery>(
        org.apache.lucene.search.Query::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/document/
