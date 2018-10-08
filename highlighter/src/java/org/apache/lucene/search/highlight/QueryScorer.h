#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/highlight/WeightedSpanTerm.h"

#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include  "core/src/java/org/apache/lucene/search/highlight/WeightedSpanTermExtractor.h"
#include  "core/src/java/org/apache/lucene/search/highlight/TextFragment.h"

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
namespace org::apache::lucene::search::highlight
{

using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using IndexReader = org::apache::lucene::index::IndexReader;
using Query = org::apache::lucene::search::Query;

/**
 * {@link Scorer} implementation which scores text fragments by the number of
 * unique query terms found. This class converts appropriate {@link Query}s to
 * {@link SpanQuery}s and attempts to score only those terms that participated
in GET_CLASS_NAME(converts)
 * generating the 'hit' on the document.
 */
class QueryScorer : public std::enable_shared_from_this<QueryScorer>,
                    public Scorer
{
  GET_CLASS_NAME(QueryScorer)
private:
  float totalScore = 0;
  std::shared_ptr<Set<std::wstring>> foundTerms;
  std::unordered_map<std::wstring, std::shared_ptr<WeightedSpanTerm>>
      fieldWeightedSpanTerms;
  float maxTermWeight = 0;
  int position = -1;
  std::wstring defaultField;
  std::shared_ptr<CharTermAttribute> termAtt;
  std::shared_ptr<PositionIncrementAttribute> posIncAtt;
  bool expandMultiTermQuery = true;
  std::shared_ptr<Query> query;
  std::wstring field;
  std::shared_ptr<IndexReader> reader;
  bool skipInitExtractor = false;
  bool wrapToCaching = true;
  int maxCharsToAnalyze = 0;
  bool usePayloads = false;

  /**
   * @param query Query to use for highlighting
   */
public:
  QueryScorer(std::shared_ptr<Query> query);

  /**
   * @param query Query to use for highlighting
   * @param field Field to highlight - pass null to ignore fields
   */
  QueryScorer(std::shared_ptr<Query> query, const std::wstring &field);

  /**
   * @param query Query to use for highlighting
   * @param field Field to highlight - pass null to ignore fields
   * @param reader {@link IndexReader} to use for quasi tf/idf scoring
   */
  QueryScorer(std::shared_ptr<Query> query, std::shared_ptr<IndexReader> reader,
              const std::wstring &field);

  /**
   * @param query to use for highlighting
   * @param reader {@link IndexReader} to use for quasi tf/idf scoring
   * @param field to highlight - pass null to ignore fields
   */
  QueryScorer(std::shared_ptr<Query> query, std::shared_ptr<IndexReader> reader,
              const std::wstring &field, const std::wstring &defaultField);

  /**
   * @param defaultField - The default field for queries with the field name
   * unspecified
   */
  QueryScorer(std::shared_ptr<Query> query, const std::wstring &field,
              const std::wstring &defaultField);

  /**
   * @param weightedTerms an array of pre-created {@link WeightedSpanTerm}s
   */
  QueryScorer(std::deque<std::shared_ptr<WeightedSpanTerm>> &weightedTerms);

  /*
   * (non-Javadoc)
   *
   * @see org.apache.lucene.search.highlight.Scorer#getFragmentScore()
   */
  float getFragmentScore() override;

  /**
   *
   * @return The highest weighted term (useful for passing to
   *         GradientFormatter to set top end of coloring scale).
   */
  virtual float getMaxTermWeight();

  /*
   * (non-Javadoc)
   *
   * @see
   * org.apache.lucene.search.highlight.Scorer#getTokenScore(org.apache.lucene.analysis.Token,
   *      int)
   */
  float getTokenScore() override;

  /* (non-Javadoc)
   * @see
   * org.apache.lucene.search.highlight.Scorer#init(org.apache.lucene.analysis.TokenStream)
   */
  std::shared_ptr<TokenStream>
  init(std::shared_ptr<TokenStream> tokenStream)  override;

  /**
   * Retrieve the {@link WeightedSpanTerm} for the specified token. Useful for
   * passing Span information to a {@link Fragmenter}.
   *
   * @param token to get {@link WeightedSpanTerm} for
   * @return WeightedSpanTerm for token
   */
  virtual std::shared_ptr<WeightedSpanTerm>
  getWeightedSpanTerm(const std::wstring &token);

  /**
   */
private:
  void init(std::shared_ptr<Query> query, const std::wstring &field,
            std::shared_ptr<IndexReader> reader, bool expandMultiTermQuery);

  std::shared_ptr<TokenStream>
  initExtractor(std::shared_ptr<TokenStream> tokenStream) ;

protected:
  virtual std::shared_ptr<WeightedSpanTermExtractor>
  newTermExtractor(const std::wstring &defaultField);

  /*
   * (non-Javadoc)
   *
   * @see
   * org.apache.lucene.search.highlight.Scorer#startFragment(org.apache.lucene.search.highlight.TextFragment)
   */
public:
  void startFragment(std::shared_ptr<TextFragment> newFragment) override;

  /**
   * @return true if multi-term queries should be expanded
   */
  virtual bool isExpandMultiTermQuery();

  /**
   * Controls whether or not multi-term queries are expanded
   * against a {@link MemoryIndex} {@link IndexReader}.
   *
   * @param expandMultiTermQuery true if multi-term queries should be expanded
   */
  virtual void setExpandMultiTermQuery(bool expandMultiTermQuery);

  /**
   * Whether or not we should capture payloads in {@link MemoryIndex} at each
   * position so that queries can access them. This does not apply to term
   * deque based TokenStreams, which support payloads only when the term deque
   * has them.
   */
  virtual bool isUsePayloads();

  virtual void setUsePayloads(bool usePayloads);

  /**
   * By default, {@link TokenStream}s that are not of the type
   * {@link CachingTokenFilter} are wrapped in a {@link CachingTokenFilter} to
   * ensure an efficient reset - if you are already using a different caching
   * {@link TokenStream} impl and you don't want it to be wrapped, set this to
   * false. Note that term-deque based tokenstreams are detected and won't be
   * wrapped either.
   */
  virtual void setWrapIfNotCachingTokenFilter(bool wrap);

  virtual void setMaxDocCharsToAnalyze(int maxDocCharsToAnalyze);
};

} // #include  "core/src/java/org/apache/lucene/search/highlight/
