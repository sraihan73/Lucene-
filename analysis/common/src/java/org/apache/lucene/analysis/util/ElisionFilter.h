#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/CharArraySet.h"

#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
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
namespace org::apache::lucene::analysis::util
{

using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

/**
 * Removes elisions from a {@link TokenStream}. For example, "l'avion" (the
 * plane) will be tokenized as "avion" (plane).
 *
 * @see <a href="http://fr.wikipedia.org/wiki/%C3%89lision">Elision in
 * Wikipedia</a>
 */
class ElisionFilter final : public TokenFilter
{
  GET_CLASS_NAME(ElisionFilter)
private:
  const std::shared_ptr<CharArraySet> articles;
  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);

  /**
   * Constructs an elision filter with a Set of stop words
   * @param input the source {@link TokenStream}
   * @param articles a set of stopword articles
   */
public:
  ElisionFilter(std::shared_ptr<TokenStream> input,
                std::shared_ptr<CharArraySet> articles);

  /**
   * Increments the {@link TokenStream} with a {@link CharTermAttribute} without
   * elisioned start
   */
  bool incrementToken()  override final;

protected:
  std::shared_ptr<ElisionFilter> shared_from_this()
  {
    return std::static_pointer_cast<ElisionFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/util/
