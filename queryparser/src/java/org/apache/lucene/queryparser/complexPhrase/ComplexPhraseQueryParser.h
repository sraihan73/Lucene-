#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/queryparser/complexPhrase/ComplexPhraseQuery.h"

#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/queryparser/classic/ParseException.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/BooleanQuery.h"
#include  "core/src/java/org/apache/lucene/search/SynonymQuery.h"
#include  "core/src/java/org/apache/lucene/search/spans/SpanQuery.h"

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
namespace org::apache::lucene::queryparser::complexPhrase
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using IndexReader = org::apache::lucene::index::IndexReader;
using Term = org::apache::lucene::index::Term;
using ParseException =
    org::apache::lucene::queryparser::classic::ParseException;
using QueryParser = org::apache::lucene::queryparser::classic::QueryParser;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using Query = org::apache::lucene::search::Query;
using SynonymQuery = org::apache::lucene::search::SynonymQuery;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;

/**
 * QueryParser which permits complex phrase query syntax eg "(john jon
 * jonathan~) peters*".
 * <p>
 * Performs potentially multiple passes over Query text to parse any nested
 * logic in PhraseQueries. - First pass takes any PhraseQuery content between
 * quotes and stores for subsequent pass. All other query content is parsed as
 * normal - Second pass parses any stored PhraseQuery content, checking all
 * embedded clauses are referring to the same field and therefore can be
 * rewritten as Span queries. All PhraseQuery clauses are expressed as
 * ComplexPhraseQuery objects
 * </p>
 * <p>
 * This could arguably be done in one pass using a new QueryParser but here I am
 * working within the constraints of the existing parser as a base class. This
 * currently simply feeds all phrase content through an analyzer to select
 * phrase terms - any "special" syntax such as * ~ * etc are not given special
 * status
 * </p>
 *
 */
class ComplexPhraseQueryParser : public QueryParser
{
  GET_CLASS_NAME(ComplexPhraseQueryParser)
private:
  std::deque<std::shared_ptr<ComplexPhraseQuery>> complexPhrases;

  bool isPass2ResolvingPhrases = false;

  bool inOrder = true;

  /**
   * When <code>inOrder</code> is true, the search terms must
   * exists in the documents as the same order as in query.
   *
   * @param inOrder parameter to choose between ordered or un-ordered proximity
   * search
   */
public:
  virtual void setInOrder(bool const inOrder);

private:
  std::shared_ptr<ComplexPhraseQuery> currentPhraseQuery = nullptr;

public:
  ComplexPhraseQueryParser(const std::wstring &f, std::shared_ptr<Analyzer> a);

protected:
  std::shared_ptr<Query> getFieldQuery(const std::wstring &field,
                                       const std::wstring &queryText,
                                       int slop) override;

public:
  std::shared_ptr<Query>
  parse(const std::wstring &query)  override;

  // There is No "getTermQuery throws ParseException" method to override so
  // unfortunately need
  // to throw a runtime exception here if a term for another field is embedded
  // in phrase query
protected:
  std::shared_ptr<Query> newTermQuery(std::shared_ptr<Term> term) override;

  // Helper method used to report on any clauses that appear in query syntax
private:
  void checkPhraseClauseIsForSameField(const std::wstring &field) throw(
      ParseException);

protected:
  std::shared_ptr<Query>
  getWildcardQuery(const std::wstring &field,
                   const std::wstring &termStr)  override;

  std::shared_ptr<Query>
  getRangeQuery(const std::wstring &field, const std::wstring &part1,
                const std::wstring &part2, bool startInclusive,
                bool endInclusive)  override;

  std::shared_ptr<Query> newRangeQuery(const std::wstring &field,
                                       const std::wstring &part1,
                                       const std::wstring &part2,
                                       bool startInclusive,
                                       bool endInclusive) override;

  std::shared_ptr<Query>
  getFuzzyQuery(const std::wstring &field, const std::wstring &termStr,
                float minSimilarity)  override;

  /*
   * Used to handle the query content in between quotes and produced Span-based
   * interpretations of the clauses.
   */
public:
  class ComplexPhraseQuery : public Query
  {
    GET_CLASS_NAME(ComplexPhraseQuery)

  public:
    const std::wstring field;

    const std::wstring phrasedQueryStringContents;

    const int slopFactor;

  private:
    const bool inOrder;

    std::deque<std::shared_ptr<Query>> const contents =
        std::deque<std::shared_ptr<Query>>(1);

  public:
    ComplexPhraseQuery(const std::wstring &field,
                       const std::wstring &phrasedQueryStringContents,
                       int slopFactor, bool inOrder);

    // Called by ComplexPhraseQueryParser for each phrase after the main
    // parse
    // thread is through
  protected:
    virtual void parsePhraseElements(
        std::shared_ptr<ComplexPhraseQueryParser> qp) ;

  public:
    std::shared_ptr<Query>
    rewrite(std::shared_ptr<IndexReader> reader)  override;

  private:
    std::shared_ptr<BooleanQuery> convert(std::shared_ptr<SynonymQuery> qc);

    void
    addComplexPhraseClause(std::deque<std::shared_ptr<SpanQuery>> &spanClauses,
                           std::shared_ptr<BooleanQuery> qc);

  public:
    std::wstring toString(const std::wstring &field) override;

    virtual int hashCode();

    bool equals(std::any other) override;

  private:
    bool equalsTo(std::shared_ptr<ComplexPhraseQuery> other);

  protected:
    std::shared_ptr<ComplexPhraseQuery> shared_from_this()
    {
      return std::static_pointer_cast<ComplexPhraseQuery>(
          org.apache.lucene.search.Query::shared_from_this());
    }
  };

protected:
  std::shared_ptr<ComplexPhraseQueryParser> shared_from_this()
  {
    return std::static_pointer_cast<ComplexPhraseQueryParser>(
        org.apache.lucene.queryparser.classic.QueryParser::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queryparser/complexPhrase/
