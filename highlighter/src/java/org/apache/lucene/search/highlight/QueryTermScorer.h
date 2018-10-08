#pragma once
#include "stringhelper.h"
#include <algorithm>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/highlight/TextFragment.h"

#include  "core/src/java/org/apache/lucene/search/highlight/WeightedTerm.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"

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
using IndexReader = org::apache::lucene::index::IndexReader;
using Query = org::apache::lucene::search::Query;

/**
 * {@link Scorer} implementation which scores text fragments by the number of
 * unique query terms found. This class uses the {@link QueryTermExtractor}
 * class to process determine the query terms and their boosts to be used.
 */
// TODO: provide option to boost score of fragments near beginning of document
// based on fragment.getFragNum()
class QueryTermScorer : public std::enable_shared_from_this<QueryTermScorer>,
                        public Scorer
{
  GET_CLASS_NAME(QueryTermScorer)

public:
  std::shared_ptr<TextFragment> currentTextFragment = nullptr;
  std::unordered_set<std::wstring> uniqueTermsInFragment;

  float totalScore = 0;
  float maxTermWeight = 0;

private:
  std::unordered_map<std::wstring, std::shared_ptr<WeightedTerm>> termsToFind;

  std::shared_ptr<CharTermAttribute> termAtt;

  /**
   *
   * @param query a Lucene query (ideally rewritten using query.rewrite before
   *        being passed to this class and the searcher)
   */
public:
  QueryTermScorer(std::shared_ptr<Query> query);

  /**
   *
   * @param query a Lucene query (ideally rewritten using query.rewrite before
   *        being passed to this class and the searcher)
   * @param fieldName the Field name which is used to match Query terms
   */
  QueryTermScorer(std::shared_ptr<Query> query, const std::wstring &fieldName);

  /**
   *
   * @param query a Lucene query (ideally rewritten using query.rewrite before
   *        being passed to this class and the searcher)
   * @param reader used to compute IDF which can be used to a) score selected
   *        fragments better b) use graded highlights eg set font color
   *        intensity
   * @param fieldName the field on which Inverse Document Frequency (IDF)
   *        calculations are based
   */
  QueryTermScorer(std::shared_ptr<Query> query,
                  std::shared_ptr<IndexReader> reader,
                  const std::wstring &fieldName);

  QueryTermScorer(std::deque<std::shared_ptr<WeightedTerm>> &weightedTerms);

  /* (non-Javadoc)
   * @see
   * org.apache.lucene.search.highlight.Scorer#init(org.apache.lucene.analysis.TokenStream)
   */
  std::shared_ptr<TokenStream>
  init(std::shared_ptr<TokenStream> tokenStream) override;

  /*
   * (non-Javadoc)
   *
   * @see
   * org.apache.lucene.search.highlight.FragmentScorer#startFragment(org.apache
   * .lucene.search.highlight.TextFragment)
   */
  void startFragment(std::shared_ptr<TextFragment> newFragment) override;

  /* (non-Javadoc)
   * @see org.apache.lucene.search.highlight.Scorer#getTokenScore()
   */
  float getTokenScore() override;

  /* (non-Javadoc)
   * @see org.apache.lucene.search.highlight.Scorer#getFragmentScore()
   */
  float getFragmentScore() override;

  /*
   * (non-Javadoc)
   *
   * @see
   * org.apache.lucene.search.highlight.FragmentScorer#allFragmentsProcessed()
   */
  virtual void allFragmentsProcessed();

  /**
   *
   * @return The highest weighted term (useful for passing to GradientFormatter
   *         to set top end of coloring scale.
   */
  virtual float getMaxTermWeight();
};

} // #include  "core/src/java/org/apache/lucene/search/highlight/
