#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/search/Query.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/PriorityQueue.h"
#include "stringhelper.h"
#include <algorithm>
#include <any>
#include <cmath>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <queue>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::classification::utils
{
class FieldVals;
}

namespace org::apache::lucene::analysis
{
class Analyzer;
}
namespace org::apache::lucene::classification::utils
{
class ScoreTermQueue;
}
namespace org::apache::lucene::index
{
class IndexReader;
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
namespace org::apache::lucene::classification::utils
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using IndexReader = org::apache::lucene::index::IndexReader;
using Term = org::apache::lucene::index::Term;
using Query = org::apache::lucene::search::Query;
using PriorityQueue = org::apache::lucene::util::PriorityQueue;

/**
 * Simplification of FuzzyLikeThisQuery, to be used in the context of KNN
 * classification.
 */
class NearestFuzzyQuery : public Query
{
  GET_CLASS_NAME(NearestFuzzyQuery)

private:
  const std::deque<std::shared_ptr<FieldVals>> fieldVals =
      std::deque<std::shared_ptr<FieldVals>>();
  const std::shared_ptr<Analyzer> analyzer;

  // fixed parameters
  static constexpr int MAX_VARIANTS_PER_TERM = 50;
  static constexpr float MIN_SIMILARITY = 1.0f;
  static constexpr int PREFIX_LENGTH = 2;
  static constexpr int MAX_NUM_TERMS = 300;

  /**
   * Default constructor
   *
   * @param analyzer the analyzer used to proecss the query text
   */
public:
  NearestFuzzyQuery(std::shared_ptr<Analyzer> analyzer);

public:
  class FieldVals : public std::enable_shared_from_this<FieldVals>
  {
    GET_CLASS_NAME(FieldVals)
  public:
    const std::wstring queryString;
    const std::wstring fieldName;
    const int maxEdits;
    const int prefixLength;

    FieldVals(const std::wstring &name, int maxEdits,
              const std::wstring &queryString);

    virtual int hashCode();

    bool equals(std::any obj) override;
  };

  /**
   * Adds user input for "fuzzification"
   *
   * @param queryString The string which will be parsed by the analyzer and for
   * which fuzzy variants will be parsed
   */
public:
  virtual void addTerms(const std::wstring &queryString,
                        const std::wstring &fieldName);

private:
  void addTerms(std::shared_ptr<IndexReader> reader,
                std::shared_ptr<FieldVals> f,
                std::shared_ptr<ScoreTermQueue> q) ;

  float idf(int docFreq, int docCount);

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
    const std::shared_ptr<Term> term;
    float score = 0;
    const std::shared_ptr<Term> fuzziedSourceTerm;

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

public:
  std::wstring toString(const std::wstring &field) override;

  virtual int hashCode();

  bool equals(std::any other) override;

private:
  bool equalsTo(std::shared_ptr<NearestFuzzyQuery> other);

protected:
  std::shared_ptr<NearestFuzzyQuery> shared_from_this()
  {
    return std::static_pointer_cast<NearestFuzzyQuery>(
        org.apache.lucene.search.Query::shared_from_this());
  }
};

} // namespace org::apache::lucene::classification::utils
