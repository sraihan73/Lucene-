#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/fst/FST.h"
#include "stringhelper.h"
#include <cctype>
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/miscellaneous/StemmerOverrideMap.h"

#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/KeywordAttribute.h"
namespace org::apache::lucene::util::fst
{
template <typename T>
class FST;
}
#include  "core/src/java/org/apache/lucene/util/fst/BytesReader.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"
namespace org::apache::lucene::util::fst
{
template <typename T>
class Arc;
}
#include  "core/src/java/org/apache/lucene/util/BytesRefHash.h"
#include  "core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include  "core/src/java/org/apache/lucene/util/CharsRefBuilder.h"

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
namespace org::apache::lucene::analysis::miscellaneous
{

using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using KeywordAttribute =
    org::apache::lucene::analysis::tokenattributes::KeywordAttribute;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using BytesRefHash = org::apache::lucene::util::BytesRefHash;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;
using FST = org::apache::lucene::util::fst::FST;
using org::apache::lucene::util::fst::FST::Arc;
using BytesReader = org::apache::lucene::util::fst::FST::BytesReader;

/**
 * Provides the ability to override any {@link KeywordAttribute} aware stemmer
 * with custom dictionary-based stemming.
 */
class StemmerOverrideFilter final : public TokenFilter
{
  GET_CLASS_NAME(StemmerOverrideFilter)
private:
  const std::shared_ptr<StemmerOverrideMap> stemmerOverrideMap;

  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<KeywordAttribute> keywordAtt =
      addAttribute(KeywordAttribute::typeid);
  const std::shared_ptr<FST::BytesReader> fstReader;
  const std::shared_ptr<FST::Arc<std::shared_ptr<BytesRef>>> scratchArc =
      std::make_shared<FST::Arc<std::shared_ptr<BytesRef>>>();
  std::deque<wchar_t> spare = std::deque<wchar_t>(0);

  /**
   * Create a new StemmerOverrideFilter, performing dictionary-based stemming
   * with the provided <code>dictionary</code>.
   * <p>
   * Any dictionary-stemmed terms will be marked with {@link KeywordAttribute}
   * so that they will not be stemmed with stemmers down the chain.
   * </p>
   */
public:
  StemmerOverrideFilter(std::shared_ptr<TokenStream> input,
                        std::shared_ptr<StemmerOverrideMap> stemmerOverrideMap);

  bool incrementToken()  override;

  /**
   * A read-only 4-byte FST backed map_obj that allows fast case-insensitive key
   * value lookups for {@link StemmerOverrideFilter}
   */
  // TODO maybe we can generalize this and reuse this map_obj somehow?
public:
  class StemmerOverrideMap final
      : public std::enable_shared_from_this<StemmerOverrideMap>
  {
    GET_CLASS_NAME(StemmerOverrideMap)
  private:
    const std::shared_ptr<FST<std::shared_ptr<BytesRef>>> fst;
    const bool ignoreCase;

    /**
     * Creates a new {@link StemmerOverrideMap}
     * @param fst the fst to lookup the overrides
     * @param ignoreCase if the keys case should be ingored
     */
  public:
    StemmerOverrideMap(std::shared_ptr<FST<std::shared_ptr<BytesRef>>> fst,
                       bool ignoreCase);

    /**
     * Returns a {@link BytesReader} to pass to the {@link #get(char[], int,
     * FST.Arc, FST.BytesReader)} method.
     */
    std::shared_ptr<FST::BytesReader> getBytesReader();

    /**
     * Returns the value mapped to the given key or <code>null</code> if the key
     * is not in the FST dictionary.
     */
    std::shared_ptr<BytesRef>
    get(std::deque<wchar_t> &buffer, int bufferLen,
        std::shared_ptr<FST::Arc<std::shared_ptr<BytesRef>>> scratchArc,
        std::shared_ptr<FST::BytesReader> fstReader) ;
  };
  /**
   * This builder builds an {@link FST} for the {@link StemmerOverrideFilter}
   */
public:
  class Builder : public std::enable_shared_from_this<Builder>
  {
    GET_CLASS_NAME(Builder)
  private:
    const std::shared_ptr<BytesRefHash> hash = std::make_shared<BytesRefHash>();
    const std::shared_ptr<BytesRefBuilder> spare =
        std::make_shared<BytesRefBuilder>();
    const std::deque<std::shared_ptr<std::wstring>> outputValues =
        std::deque<std::shared_ptr<std::wstring>>();
    const bool ignoreCase;
    const std::shared_ptr<CharsRefBuilder> charsSpare =
        std::make_shared<CharsRefBuilder>();

    /**
     * Creates a new {@link Builder} with ignoreCase set to <code>false</code>
     */
  public:
    Builder();

    /**
     * Creates a new {@link Builder}
     * @param ignoreCase if the input case should be ignored.
     */
    Builder(bool ignoreCase);

    /**
     * Adds an input string and its stemmer override output to this builder.
     *
     * @param input the input char sequence
     * @param output the stemmer override output char sequence
     * @return <code>false</code> iff the input has already been added to this
     * builder otherwise <code>true</code>.
     */
    virtual bool add(std::shared_ptr<std::wstring> input,
                     std::shared_ptr<std::wstring> output);

    /**
     * Returns an {@link StemmerOverrideMap} to be used with the {@link
     * StemmerOverrideFilter}
     * @return an {@link StemmerOverrideMap} to be used with the {@link
     * StemmerOverrideFilter}
     * @throws IOException if an {@link IOException} occurs;
     */
    virtual std::shared_ptr<StemmerOverrideMap> build() ;
  };

protected:
  std::shared_ptr<StemmerOverrideFilter> shared_from_this()
  {
    return std::static_pointer_cast<StemmerOverrideFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/miscellaneous/
