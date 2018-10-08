#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"

#include  "core/src/java/org/apache/lucene/search/Weight.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/Explanation.h"
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

/**
 * A query that matches no documents.
 */

class MatchNoDocsQuery : public Query
{
  GET_CLASS_NAME(MatchNoDocsQuery)

private:
  const std::wstring reason;

  /** Default constructor */
public:
  MatchNoDocsQuery();

  /** Provides a reason explaining why this query was used */
  MatchNoDocsQuery(const std::wstring &reason);

  std::shared_ptr<Weight> createWeight(std::shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost)  override;

private:
  class WeightAnonymousInnerClass : public Weight
  {
    GET_CLASS_NAME(WeightAnonymousInnerClass)
  private:
    std::shared_ptr<MatchNoDocsQuery> outerInstance;

  public:
    WeightAnonymousInnerClass(std::shared_ptr<MatchNoDocsQuery> outerInstance);

    void
    extractTerms(std::shared_ptr<Set<std::shared_ptr<Term>>> terms) override;

    std::shared_ptr<Explanation>
    explain(std::shared_ptr<LeafReaderContext> context,
            int doc)  override;

    std::shared_ptr<Scorer> scorer(
        std::shared_ptr<LeafReaderContext> context)  override;

    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  protected:
    std::shared_ptr<WeightAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<WeightAnonymousInnerClass>(
          Weight::shared_from_this());
    }
  };

public:
  std::wstring toString(const std::wstring &field) override;

  virtual bool equals(std::any o);

  virtual int hashCode();

protected:
  std::shared_ptr<MatchNoDocsQuery> shared_from_this()
  {
    return std::static_pointer_cast<MatchNoDocsQuery>(
        Query::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
