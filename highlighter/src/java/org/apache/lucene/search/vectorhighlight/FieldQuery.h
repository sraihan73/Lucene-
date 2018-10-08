#pragma once
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/vectorhighlight/QueryPhraseMap.h"

#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/search/PhraseQuery.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/search/vectorhighlight/FieldTermStack.h"
#include  "core/src/java/org/apache/lucene/search/vectorhighlight/TermInfo.h"

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
namespace org::apache::lucene::search::vectorhighlight
{

using IndexReader = org::apache::lucene::index::IndexReader;
using Term = org::apache::lucene::index::Term;
using PhraseQuery = org::apache::lucene::search::PhraseQuery;
using Query = org::apache::lucene::search::Query;
using TermInfo =
    org::apache::lucene::search::vectorhighlight::FieldTermStack::TermInfo;

/**
 * FieldQuery breaks down query object into terms/phrases and keeps
 * them in a QueryPhraseMap structure.
 */
class FieldQuery : public std::enable_shared_from_this<FieldQuery>
{
  GET_CLASS_NAME(FieldQuery)

public:
  const bool fieldMatch;

  // fieldMatch==true,  Map<fieldName,QueryPhraseMap>
  // fieldMatch==false, Map<null,QueryPhraseMap>
  std::unordered_map<std::wstring, std::shared_ptr<QueryPhraseMap>> rootMaps =
      std::unordered_map<std::wstring, std::shared_ptr<QueryPhraseMap>>();

  // fieldMatch==true,  Map<fieldName,setOfTermsInQueries>
  // fieldMatch==false, Map<null,setOfTermsInQueries>
  std::unordered_map<std::wstring, Set<std::wstring>> termSetMap =
      std::unordered_map<std::wstring, Set<std::wstring>>();

  int termOrPhraseNumber = 0; // used for colored tag support

  // The maximum number of different matching terms accumulated from any one
  // MultiTermQuery
private:
  static constexpr int MAX_MTQ_TERMS = 1024;

public:
  FieldQuery(std::shared_ptr<Query> query, std::shared_ptr<IndexReader> reader,
             bool phraseHighlight, bool fieldMatch) ;

  /** For backwards compatibility you can initialize FieldQuery without
   * an IndexReader, which is only required to support MultiTermQuery
   */
  FieldQuery(std::shared_ptr<Query> query, bool phraseHighlight,
             bool fieldMatch) ;

  virtual void
  flatten(std::shared_ptr<Query> sourceQuery,
          std::shared_ptr<IndexReader> reader,
          std::shared_ptr<std::deque<std::shared_ptr<Query>>> flatQueries,
          float boost) ;

  /*
   * Create expandQueries from flatQueries.
   *
   * expandQueries := flatQueries + overlapped phrase queries
   *
   * ex1) flatQueries={a,b,c}
   *      => expandQueries={a,b,c}
   * ex2) flatQueries={a,"b c","c d"}
   *      => expandQueries={a,"b c","c d","b c d"}
   */
  virtual std::shared_ptr<std::deque<std::shared_ptr<Query>>>
  expand(std::shared_ptr<std::deque<std::shared_ptr<Query>>> flatQueries);

  /*
   * Check if PhraseQuery A and B have overlapped part.
   *
   * ex1) A="a b", B="b c" => overlap; expandQueries={"a b c"}
   * ex2) A="b c", B="a b" => overlap; expandQueries={"a b c"}
   * ex3) A="a b", B="c d" => no overlap; expandQueries={}
   */
private:
  void checkOverlap(
      std::shared_ptr<std::deque<std::shared_ptr<Query>>> expandQueries,
      std::shared_ptr<PhraseQuery> a, float aBoost,
      std::shared_ptr<PhraseQuery> b, float bBoost);

  /*
   * Check if src and dest have overlapped part and if it is, create
   * PhraseQueries and add expandQueries.
   *
   * ex1) src="a b", dest="c d"       => no overlap
   * ex2) src="a b", dest="a b c"     => no overlap
   * ex3) src="a b", dest="b c"       => overlap; expandQueries={"a b c"}
   * ex4) src="a b c", dest="b c d"   => overlap; expandQueries={"a b c d"}
   * ex5) src="a b c", dest="b c"     => no overlap
   * ex6) src="a b c", dest="b"       => no overlap
   * ex7) src="a a a a", dest="a a a" => overlap;
   *                                     expandQueries={"a a a a a","a a a a a
   * a"} ex8) src="a b c d", dest="b c"   => no overlap
   */
  void checkOverlap(
      std::shared_ptr<std::deque<std::shared_ptr<Query>>> expandQueries,
      std::deque<std::shared_ptr<Term>> &src,
      std::deque<std::shared_ptr<Term>> &dest, int slop, float boost);

public:
  virtual std::shared_ptr<QueryPhraseMap>
  getRootMap(std::shared_ptr<Query> query);

  /*
   * Return 'key' string. 'key' is the field name of the Query.
   * If not fieldMatch, 'key' will be null.
   */
private:
  std::wstring getKey(std::shared_ptr<Query> query);

  /*
   * Save the set of terms in the queries to termSetMap.
   *
   * ex1) q=name:john
   *      - fieldMatch==true
   *          termSetMap=Map<"name",std::unordered_set<"john">>
   *      - fieldMatch==false
   *          termSetMap=Map<null,std::unordered_set<"john">>
   *
   * ex2) q=name:john title:manager
   *      - fieldMatch==true
   *          termSetMap=Map<"name",std::unordered_set<"john">,
   *                         "title",std::unordered_set<"manager">>
   *      - fieldMatch==false
   *          termSetMap=Map<null,std::unordered_set<"john","manager">>
   *
   * ex3) q=name:"john lennon"
   *      - fieldMatch==true
   *          termSetMap=Map<"name",std::unordered_set<"john","lennon">>
   *      - fieldMatch==false
   *          termSetMap=Map<null,std::unordered_set<"john","lennon">>
   */
public:
  virtual void
  saveTerms(std::shared_ptr<std::deque<std::shared_ptr<Query>>> flatQueries,
            std::shared_ptr<IndexReader> reader) ;

private:
  std::shared_ptr<Set<std::wstring>> getTermSet(std::shared_ptr<Query> query);

public:
  virtual std::shared_ptr<Set<std::wstring>>
  getTermSet(const std::wstring &field);

  /**
   *
   * @return QueryPhraseMap
   */
  virtual std::shared_ptr<QueryPhraseMap>
  getFieldTermMap(const std::wstring &fieldName, const std::wstring &term);

  /**
   *
   * @return QueryPhraseMap
   */
  virtual std::shared_ptr<QueryPhraseMap>
  searchPhrase(const std::wstring &fieldName,
               std::deque<std::shared_ptr<TermInfo>> &phraseCandidate);

private:
  std::shared_ptr<QueryPhraseMap> getRootMap(const std::wstring &fieldName);

public:
  virtual int nextTermOrPhraseNumber();

  /**
   * Internal structure of a query for highlighting: represents
   * a nested query structure
   */
public:
  class QueryPhraseMap : public std::enable_shared_from_this<QueryPhraseMap>
  {
    GET_CLASS_NAME(QueryPhraseMap)

  public:
    bool terminal = false;
    int slop = 0;    // valid if terminal == true and phraseHighlight == true
    float boost = 0; // valid if terminal == true
    int termOrPhraseNumber = 0; // valid if terminal == true
    std::shared_ptr<FieldQuery> fieldQuery;
    std::unordered_map<std::wstring, std::shared_ptr<QueryPhraseMap>> subMap =
        std::unordered_map<std::wstring, std::shared_ptr<QueryPhraseMap>>();

    QueryPhraseMap(std::shared_ptr<FieldQuery> fieldQuery);

    virtual void addTerm(std::shared_ptr<Term> term, float boost);

  private:
    std::shared_ptr<QueryPhraseMap> getOrNewMap(
        std::unordered_map<std::wstring, std::shared_ptr<QueryPhraseMap>>
            &subMap,
        const std::wstring &term);

  public:
    virtual void add(std::shared_ptr<Query> query,
                     std::shared_ptr<IndexReader> reader);

    virtual std::shared_ptr<QueryPhraseMap>
    getTermMap(const std::wstring &term);

  private:
    void markTerminal(float boost);

    void markTerminal(int slop, float boost);

  public:
    virtual bool isTerminal();

    virtual int getSlop();

    virtual float getBoost();

    virtual int getTermOrPhraseNumber();

    virtual std::shared_ptr<QueryPhraseMap>
    searchPhrase(std::deque<std::shared_ptr<TermInfo>> &phraseCandidate);

    virtual bool isValidTermOrPhrase(
        std::deque<std::shared_ptr<TermInfo>> &phraseCandidate);
  };
};

} // #include  "core/src/java/org/apache/lucene/search/vectorhighlight/
