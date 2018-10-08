#pragma once
#include "stringhelper.h"
#include <algorithm>
#include <any>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <queue>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::similarities
{
class TFIDFSimilarity;
}

namespace org::apache::lucene::sandbox::queries
{
class FieldVals;
}
namespace org::apache::lucene::analysis
{
class Analyzer;
}
namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::sandbox::queries
{
class ScoreTermQueue;
}
namespace org::apache::lucene::index
{
class Term;
}
namespace org::apache::lucene::search
{
class Query;
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
namespace org::apache::lucene::sandbox::queries
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using IndexReader = org::apache::lucene::index::IndexReader;
using Term = org::apache::lucene::index::Term;
using Query = org::apache::lucene::search::Query;
using ClassicSimilarity =
    org::apache::lucene::search::similarities::ClassicSimilarity;
using TFIDFSimilarity =
    org::apache::lucene::search::similarities::TFIDFSimilarity;
using PriorityQueue = org::apache::lucene::util::PriorityQueue;

/**
 * Fuzzifies ALL terms provided as strings and then picks the best n
 * differentiating terms. In effect this mixes the behaviour of FuzzyQuery and
 * MoreLikeThis but with special consideration of fuzzy scoring factors. This
 * generally produces good results for queries where users may provide details
 * in a number of fields and have no knowledge of bool query syntax and also
 * want a degree of fuzzy matching and a fast query.
 *
 * For each source term the fuzzy variants are held in a BooleanQuery with no
 * coord factor (because we are not looking for matches on multiple variants in
 * any one doc). Additionally, a specialized TermQuery is used for variants and
 * does not use that variant term's IDF because this would favour rarer terms eg
 * misspellings. Instead, all variants use the same IDF ranking (the one for the
 * source query term) and this is factored into the variant's boost. If the
 * source query term does not exist in the index the average IDF of the variants
 * is used.
 */
class FuzzyLikeThisQuery : public Query
{
  GET_CLASS_NAME(FuzzyLikeThisQuery)
  // TODO: generalize this query (at least it should not reuse this static sim!
  // a better way might be to convert this into multitermquery rewrite methods.
  // the rewrite method can 'average' the TermContext's term statistics
  // (docfreq,totalTermFreq) provided to TermQuery, so that the general idea is
  // agnostic to any scoring system...
public:
  static std::shared_ptr<TFIDFSimilarity> sim;
  std::deque<std::shared_ptr<FieldVals>> fieldVals =
      std::deque<std::shared_ptr<FieldVals>>();
  std::shared_ptr<Analyzer> analyzer;

  int MAX_VARIANTS_PER_TERM = 50;
  bool ignoreTF = false;

private:
  int maxNumTerms = 0;

public:
  virtual int hashCode();

  bool equals(std::any other) override;

private:
  bool equalsTo(std::shared_ptr<FuzzyLikeThisQuery> other);

  /**
   *
   * @param maxNumTerms The total number of terms clauses that will appear once
   * rewritten as a BooleanQuery
   */
public:
  FuzzyLikeThisQuery(int maxNumTerms, std::shared_ptr<Analyzer> analyzer);

public:
  class FieldVals : public std::enable_shared_from_this<FieldVals>
  {
    GET_CLASS_NAME(FieldVals)
  public:
    std::wstring queryString;
    std::wstring fieldName;
    int maxEdits = 0;
    int prefixLength = 0;
    FieldVals(const std::wstring &name, int maxEdits, int length,
              const std::wstring &queryString);

    virtual int hashCode();

    bool equals(std::any obj) override;
  };

  /**
   * Adds user input for "fuzzification"
   * @param queryString The string which will be parsed by the analyzer and for
   * which fuzzy variants will be parsed
   * @param minSimilarity The minimum similarity of the term variants; must be
   * 0, 1 or 2 (see FuzzyTermsEnum)
   * @param prefixLength Length of required common prefix on variant terms (see
   * FuzzyTermsEnum)
   */
public:
  virtual void addTerms(const std::wstring &queryString,
                        const std::wstring &fieldName, float minSimilarity,
                        int prefixLength);

private:
  void addTerms(std::shared_ptr<IndexReader> reader,
                std::shared_ptr<FieldVals> f,
                std::shared_ptr<ScoreTermQueue> q) ;

  std::shared_ptr<Query>
  newTermQuery(std::shared_ptr<IndexReader> reader,
               std::shared_ptr<Term> term) ;

public:
  std::shared_ptr<Query>
  rewrite(std::shared_ptr<IndexReader> reader)  override;

  // Holds info for a fuzzy term variant - initially score is set to edit
  // distance (for ranking best
  // term variants) then is reset with IDF for use in ranking against all other
  // terms/fields
private:
  class ScoreTerm : public std::enable_shared_from_this<ScoreTerm>
  {
    GET_CLASS_NAME(ScoreTerm)
  public:
    std::shared_ptr<Term> term;
    float score = 0;
    std::shared_ptr<Term> fuzziedSourceTerm;

    ScoreTerm(std::shared_ptr<Term> term, float score,
              std::shared_ptr<Term> fuzziedSourceTerm);
  };

private:
  class ScoreTermQueue : public PriorityQueue<std::shared_ptr<ScoreTerm>>
  {
    GET_CLASS_NAME(ScoreTermQueue)
  public:
    ScoreTermQueue(int size);

    /* (non-Javadoc)
     * @see org.apache.lucene.util.PriorityQueue#lessThan(java.lang.Object,
     * java.lang.Object)
     */
  protected:
    bool lessThan(std::shared_ptr<ScoreTerm> termA,
                  std::shared_ptr<ScoreTerm> termB) override;

  protected:
    std::shared_ptr<ScoreTermQueue> shared_from_this()
    {
      return std::static_pointer_cast<ScoreTermQueue>(
          org.apache.lucene.util.PriorityQueue<ScoreTerm>::shared_from_this());
    }
  };

  /* (non-Javadoc)
   * @see org.apache.lucene.search.Query#toString(java.lang.std::wstring)
   */
public:
  std::wstring toString(const std::wstring &field) override;

  virtual bool isIgnoreTF();

  virtual void setIgnoreTF(bool ignoreTF);

protected:
  std::shared_ptr<FuzzyLikeThisQuery> shared_from_this()
  {
    return std::static_pointer_cast<FuzzyLikeThisQuery>(
        org.apache.lucene.search.Query::shared_from_this());
  }
};

} // namespace org::apache::lucene::sandbox::queries
