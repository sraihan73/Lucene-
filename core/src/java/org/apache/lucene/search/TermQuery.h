#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/Term.h"

#include  "core/src/java/org/apache/lucene/index/TermContext.h"
#include  "core/src/java/org/apache/lucene/search/similarities/Similarity.h"
#include  "core/src/java/org/apache/lucene/search/similarities/SimWeight.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/Matches.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"
#include  "core/src/java/org/apache/lucene/index/TermsEnum.h"
#include  "core/src/java/org/apache/lucene/index/LeafReader.h"
#include  "core/src/java/org/apache/lucene/search/Explanation.h"
#include  "core/src/java/org/apache/lucene/search/Weight.h"

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

using LeafReader = org::apache::lucene::index::LeafReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using Similarity = org::apache::lucene::search::similarities::Similarity;

/**
 * A Query that matches documents containing a term. This may be combined with
 * other terms with a {@link BooleanQuery}.
 */
class TermQuery : public Query
{
  GET_CLASS_NAME(TermQuery)

private:
  const std::shared_ptr<Term> term;
  const std::shared_ptr<TermContext> perReaderTermState;

public:
  class TermWeight final : public Weight
  {
    GET_CLASS_NAME(TermWeight)
  private:
    std::shared_ptr<TermQuery> outerInstance;

    const std::shared_ptr<Similarity> similarity;
    const std::shared_ptr<Similarity::SimWeight> stats;
    const std::shared_ptr<TermContext> termStates;
    const bool needsScores;

  public:
    TermWeight(std::shared_ptr<TermQuery> outerInstance,
               std::shared_ptr<IndexSearcher> searcher, bool needsScores,
               float boost,
               std::shared_ptr<TermContext> termStates) ;

    void
    extractTerms(std::shared_ptr<Set<std::shared_ptr<Term>>> terms) override;

    std::shared_ptr<Matches> matches(std::shared_ptr<LeafReaderContext> context,
                                     int doc)  override;

    virtual std::wstring toString();

    std::shared_ptr<Scorer> scorer(
        std::shared_ptr<LeafReaderContext> context)  override;

    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

    /**
     * Returns a {@link TermsEnum} positioned at this weights Term or null if
     * the term does not exist in the given context
     */
  private:
    std::shared_ptr<TermsEnum>
    getTermsEnum(std::shared_ptr<LeafReaderContext> context) ;

    bool termNotInReader(std::shared_ptr<LeafReader> reader,
                         std::shared_ptr<Term> term) ;

  public:
    std::shared_ptr<Explanation>
    explain(std::shared_ptr<LeafReaderContext> context,
            int doc)  override;

  protected:
    std::shared_ptr<TermWeight> shared_from_this()
    {
      return std::static_pointer_cast<TermWeight>(Weight::shared_from_this());
    }
  };

  /** Constructs a query for the term <code>t</code>. */
public:
  TermQuery(std::shared_ptr<Term> t);

  /**
   * Expert: constructs a TermQuery that will use the provided docFreq instead
   * of looking up the docFreq against the searcher.
   */
  TermQuery(std::shared_ptr<Term> t, std::shared_ptr<TermContext> states);

  /** Returns the term of this query. */
  virtual std::shared_ptr<Term> getTerm();

  std::shared_ptr<Weight> createWeight(std::shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost)  override;

  /** Prints a user-readable version of this query. */
  std::wstring toString(const std::wstring &field) override;

  /** Returns true iff <code>o</code> is equal to this. */
  bool equals(std::any other) override;

  virtual int hashCode();

protected:
  std::shared_ptr<TermQuery> shared_from_this()
  {
    return std::static_pointer_cast<TermQuery>(Query::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
