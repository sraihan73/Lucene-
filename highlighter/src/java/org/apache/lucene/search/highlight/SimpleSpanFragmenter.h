#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/highlight/QueryScorer.h"

#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
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
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;

/**
 * {@link Fragmenter} implementation which breaks text up into same-size
 * fragments but does not split up {@link Spans}. This is a simple sample class.
 */
class SimpleSpanFragmenter
    : public std::enable_shared_from_this<SimpleSpanFragmenter>,
      public Fragmenter
{
  GET_CLASS_NAME(SimpleSpanFragmenter)
private:
  static constexpr int DEFAULT_FRAGMENT_SIZE = 100;
  int fragmentSize = 0;
  int currentNumFrags = 0;
  int position = -1;
  std::shared_ptr<QueryScorer> queryScorer;
  int waitForPos = -1;
  int textSize = 0;
  std::shared_ptr<CharTermAttribute> termAtt;
  std::shared_ptr<PositionIncrementAttribute> posIncAtt;
  std::shared_ptr<OffsetAttribute> offsetAtt;

  /**
   * @param queryScorer QueryScorer that was used to score hits
   */
public:
  SimpleSpanFragmenter(std::shared_ptr<QueryScorer> queryScorer);

  /**
   * @param queryScorer QueryScorer that was used to score hits
   * @param fragmentSize size in chars of each fragment
   */
  SimpleSpanFragmenter(std::shared_ptr<QueryScorer> queryScorer,
                       int fragmentSize);

  /* (non-Javadoc)
   * @see org.apache.lucene.search.highlight.Fragmenter#isNewFragment()
   */
  bool isNewFragment() override;

  /* (non-Javadoc)
   * @see org.apache.lucene.search.highlight.Fragmenter#start(java.lang.std::wstring,
   * org.apache.lucene.analysis.TokenStream)
   */
  void start(const std::wstring &originalText,
             std::shared_ptr<TokenStream> tokenStream) override;
};

} // #include  "core/src/java/org/apache/lucene/search/highlight/
