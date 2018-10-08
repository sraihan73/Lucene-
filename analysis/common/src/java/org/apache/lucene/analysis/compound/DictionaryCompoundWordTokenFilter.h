#pragma once
#include "CompoundWordTokenFilterBase.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/CharArraySet.h"

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
namespace org::apache::lucene::analysis::compound
{

using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;

/**
 * A {@link org.apache.lucene.analysis.TokenFilter} that decomposes compound
 * words found in many Germanic languages. <p> "Donaudampfschiff" becomes Donau,
 * dampf, schiff so that you can find "Donaudampfschiff" even when you only
 * enter "schiff". It uses a brute-force algorithm to achieve this.
 */
class DictionaryCompoundWordTokenFilter : public CompoundWordTokenFilterBase
{
  GET_CLASS_NAME(DictionaryCompoundWordTokenFilter)

  /**
   * Creates a new {@link DictionaryCompoundWordTokenFilter}
   *
   * @param input
   *          the {@link org.apache.lucene.analysis.TokenStream} to process
   * @param dictionary
   *          the word dictionary to match against.
   */
public:
  DictionaryCompoundWordTokenFilter(std::shared_ptr<TokenStream> input,
                                    std::shared_ptr<CharArraySet> dictionary);

  /**
   * Creates a new {@link DictionaryCompoundWordTokenFilter}
   *
   * @param input
   *          the {@link org.apache.lucene.analysis.TokenStream} to process
   * @param dictionary
   *          the word dictionary to match against.
   * @param minWordSize
   *          only words longer than this get processed
   * @param minSubwordSize
   *          only subwords longer than this get to the output stream
   * @param maxSubwordSize
   *          only subwords shorter than this get to the output stream
   * @param onlyLongestMatch
   *          Add only the longest matching subword to the stream
   */
  DictionaryCompoundWordTokenFilter(std::shared_ptr<TokenStream> input,
                                    std::shared_ptr<CharArraySet> dictionary,
                                    int minWordSize, int minSubwordSize,
                                    int maxSubwordSize, bool onlyLongestMatch);

protected:
  void decompose() override;

protected:
  std::shared_ptr<DictionaryCompoundWordTokenFilter> shared_from_this()
  {
    return std::static_pointer_cast<DictionaryCompoundWordTokenFilter>(
        CompoundWordTokenFilterBase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/compound/
