#pragma once
#include "stringhelper.h"
#include <memory>
#include <optional>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class Query;
}

namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::search
{
class TopDocs;
}
namespace org::apache::lucene::search
{
class ScoreDoc;
}
namespace org::apache::lucene::search
{
class Explanation;
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

/** A {@link Rescorer} that uses a provided Query to assign
 *  scores to the first-pass hits.
 *
 * @lucene.experimental */
class QueryRescorer : public Rescorer
{
  GET_CLASS_NAME(QueryRescorer)

private:
  const std::shared_ptr<Query> query;

  /** Sole constructor, passing the 2nd pass query to
   *  assign scores to the 1st pass hits.  */
public:
  QueryRescorer(std::shared_ptr<Query> query);

  /**
   * Implement this in a subclass to combine the first pass and
   * second pass scores.  If secondPassMatches is false then
   * the second pass query failed to match a hit from the
   * first pass query, and you should ignore the
   * secondPassScore.
   */
protected:
  virtual float combine(float firstPassScore, bool secondPassMatches,
                        float secondPassScore) = 0;

public:
  std::shared_ptr<TopDocs> rescore(std::shared_ptr<IndexSearcher> searcher,
                                   std::shared_ptr<TopDocs> firstPassTopDocs,
                                   int topN)  override;

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<std::shared_ptr<ScoreDoc>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  private:
    std::shared_ptr<QueryRescorer> outerInstance;

  public:
    ComparatorAnonymousInnerClass(std::shared_ptr<QueryRescorer> outerInstance);

    int compare(std::shared_ptr<ScoreDoc> a, std::shared_ptr<ScoreDoc> b);
  };

private:
  class ComparatorAnonymousInnerClass2
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass2>,
        public Comparator<std::shared_ptr<ScoreDoc>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass2)
  private:
    std::shared_ptr<QueryRescorer> outerInstance;

  public:
    ComparatorAnonymousInnerClass2(
        std::shared_ptr<QueryRescorer> outerInstance);

    int compare(std::shared_ptr<ScoreDoc> a, std::shared_ptr<ScoreDoc> b);
  };

public:
  std::shared_ptr<Explanation>
  explain(std::shared_ptr<IndexSearcher> searcher,
          std::shared_ptr<Explanation> firstPassExplanation,
          int docID)  override;

  /** Sugar API, calling {#rescore} using a simple linear
   *  combination of firstPassScore + weight * secondPassScore */
  static std::shared_ptr<TopDocs>
  rescore(std::shared_ptr<IndexSearcher> searcher,
          std::shared_ptr<TopDocs> topDocs, std::shared_ptr<Query> query,
          double const weight, int topN) ;

private:
  class QueryRescorerAnonymousInnerClass;

protected:
  std::shared_ptr<QueryRescorer> shared_from_this()
  {
    return std::static_pointer_cast<QueryRescorer>(
        Rescorer::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
class QueryRescorer::QueryRescorerAnonymousInnerClass : public QueryRescorer
{
  GET_CLASS_NAME(QueryRescorer::QueryRescorerAnonymousInnerClass)
private:
  double weight = 0;

public:
  QueryRescorerAnonymousInnerClass(
      std::shared_ptr<org::apache::lucene::search::Query> query, double weight);

protected:
  float combine(float firstPassScore, bool secondPassMatches,
                float secondPassScore) override;

protected:
  std::shared_ptr<QueryRescorerAnonymousInnerClass> shared_from_this()
  {
    return std::static_pointer_cast<QueryRescorerAnonymousInnerClass>(
        QueryRescorer::shared_from_this());
  }
};
