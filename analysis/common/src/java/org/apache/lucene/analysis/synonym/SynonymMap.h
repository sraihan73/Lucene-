#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/fst/FST.h"
#include "stringhelper.h"
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

#include  "core/src/java/org/apache/lucene/util/BytesRefHash.h"
namespace org::apache::lucene::util::fst
{
template <typename T>
class FST;
}
#include  "core/src/java/org/apache/lucene/analysis/synonym/MapEntry.h"
#include  "core/src/java/org/apache/lucene/util/CharsRef.h"
#include  "core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include  "core/src/java/org/apache/lucene/util/CharsRefBuilder.h"
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"

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
namespace org::apache::lucene::analysis::synonym
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using BytesRefHash = org::apache::lucene::util::BytesRefHash;
using CharsRef = org::apache::lucene::util::CharsRef;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;
using FST = org::apache::lucene::util::fst::FST;

/**
 * A map_obj of synonyms, keys and values are phrases.
 * @lucene.experimental
 */
class SynonymMap : public std::enable_shared_from_this<SynonymMap>
{
  GET_CLASS_NAME(SynonymMap)
  /** for multiword support, you must separate words with this separator */
public:
  static constexpr wchar_t WORD_SEPARATOR = 0;
  /** map_obj&lt;input word, deque&lt;ord&gt;&gt; */
  const std::shared_ptr<FST<std::shared_ptr<BytesRef>>> fst;
  /** map_obj&lt;ord, outputword&gt; */
  const std::shared_ptr<BytesRefHash> words;
  /** maxHorizontalContext: maximum context we need on the tokenstream */
  const int maxHorizontalContext;

  SynonymMap(std::shared_ptr<FST<std::shared_ptr<BytesRef>>> fst,
             std::shared_ptr<BytesRefHash> words, int maxHorizontalContext);

  /**
   * Builds an FSTSynonymMap.
   * <p>
   * Call add() until you have added all the mappings, then call build() to get
   * an FSTSynonymMap
   * @lucene.experimental
   */
public:
  class Builder : public std::enable_shared_from_this<Builder>
  {
    GET_CLASS_NAME(Builder)
  private:
    const std::unordered_map<std::shared_ptr<CharsRef>,
                             std::shared_ptr<MapEntry>>
        workingSet = std::unordered_map<std::shared_ptr<CharsRef>,
                                        std::shared_ptr<MapEntry>>();
    const std::shared_ptr<BytesRefHash> words =
        std::make_shared<BytesRefHash>();
    const std::shared_ptr<BytesRefBuilder> utf8Scratch =
        std::make_shared<BytesRefBuilder>();
    int maxHorizontalContext = 0;
    const bool dedup;

    /** Default constructor, passes {@code dedup=true}. */
  public:
    Builder();

    /** If dedup is true then identical rules (same input,
     *  same output) will be added only once. */
    Builder(bool dedup);

  private:
    class MapEntry : public std::enable_shared_from_this<MapEntry>
    {
      GET_CLASS_NAME(MapEntry)
    public:
      bool includeOrig = false;
      // we could sort for better sharing ultimately, but it could confuse
      // people
      std::deque<int> ords = std::deque<int>();
    };

    /** Sugar: just joins the provided terms with {@link
     *  SynonymMap#WORD_SEPARATOR}.  reuse and its chars
     *  must not be null. */
  public:
    static std::shared_ptr<CharsRef>
    join(std::deque<std::wstring> &words,
         std::shared_ptr<CharsRefBuilder> reuse);

    /** only used for asserting! */
  private:
    bool hasHoles(std::shared_ptr<CharsRef> chars);

    // NOTE: while it's tempting to make this public, since
    // caller's parser likely knows the
    // numInput/numOutputWords, sneaky exceptions, much later
    // on, will result if these values are wrong; so we always
    // recompute ourselves to be safe:
    void add(std::shared_ptr<CharsRef> input, int numInputWords,
             std::shared_ptr<CharsRef> output, int numOutputWords,
             bool includeOrig);

    int countWords(std::shared_ptr<CharsRef> chars);

    /**
     * Add a phrase-&gt;phrase synonym mapping.
     * Phrases are character sequences where words are
     * separated with character zero (U+0000).  Empty words
     * (two U+0000s in a row) are not allowed in the input nor
     * the output!
     *
     * @param input input phrase
     * @param output output phrase
     * @param includeOrig true if the original should be included
     */
  public:
    virtual void add(std::shared_ptr<CharsRef> input,
                     std::shared_ptr<CharsRef> output, bool includeOrig);

    /**
     * Builds an {@link SynonymMap} and returns it.
     */
    virtual std::shared_ptr<SynonymMap> build() ;
  };

  /**
   * Abstraction for parsing synonym files.
   *
   * @lucene.experimental
   */
public:
  class Parser : public Builder
  {
    GET_CLASS_NAME(Parser)

  private:
    const std::shared_ptr<Analyzer> analyzer;

  public:
    Parser(bool dedup, std::shared_ptr<Analyzer> analyzer);

    /**
     * Parse the given input, adding synonyms to the inherited {@link Builder}.
     * @param in The input to parse
     */
    virtual void parse(std::shared_ptr<Reader> in_) = 0;

    /** Sugar: analyzes the text with the analyzer and
     *  separates by {@link SynonymMap#WORD_SEPARATOR}.
     *  reuse and its chars must not be null. */
    virtual std::shared_ptr<CharsRef>
    analyze(const std::wstring &text,
            std::shared_ptr<CharsRefBuilder> reuse) ;

  protected:
    std::shared_ptr<Parser> shared_from_this()
    {
      return std::static_pointer_cast<Parser>(Builder::shared_from_this());
    }
  };
};

} // #include  "core/src/java/org/apache/lucene/analysis/synonym/
