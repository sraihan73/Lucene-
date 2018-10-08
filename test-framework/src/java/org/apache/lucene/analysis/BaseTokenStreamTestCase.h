#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <algorithm>
#include <any>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/AttributeImpl.h"

#include  "core/src/java/org/apache/lucene/util/AttributeReflector.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include  "core/src/java/org/apache/lucene/index/RandomIndexWriter.h"
#include  "core/src/java/org/apache/lucene/document/Field.h"
#include  "core/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include  "core/src/java/org/apache/lucene/util/AttributeFactory.h"

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
namespace org::apache::lucene::analysis
{

using namespace org::apache::lucene::analysis::tokenattributes;
using Field = org::apache::lucene::document::Field;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Attribute = org::apache::lucene::util::Attribute;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using AttributeReflector = org::apache::lucene::util::AttributeReflector;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Base class for all Lucene unit tests that use TokenStreams.
 * <p>
 * When writing unit tests for analysis components, it's highly recommended
 * to use the helper methods here (especially in conjunction with {@link
 * MockAnalyzer} or
 * {@link MockTokenizer}), as they contain many assertions and checks to
 * catch bugs.
 *
 * @see MockAnalyzer
 * @see MockTokenizer
 */
class BaseTokenStreamTestCase : public LuceneTestCase
{
  GET_CLASS_NAME(BaseTokenStreamTestCase)
  // some helpers to test Analyzers and TokenStreams:

  /**
   * Attribute that records if it was cleared or not.  This is used
   * for testing that clearAttributes() was called correctly.
   */
public:
  class CheckClearAttributesAttribute : public Attribute
  {
    GET_CLASS_NAME(CheckClearAttributesAttribute)
  public:
    virtual bool getAndResetClearCalled() = 0;
  };

  /**
   * Attribute that records if it was cleared or not.  This is used
   * for testing that clearAttributes() was called correctly.
   */
public:
  class CheckClearAttributesAttributeImpl final
      : public AttributeImpl,
        public CheckClearAttributesAttribute
  {
    GET_CLASS_NAME(CheckClearAttributesAttributeImpl)
  private:
    bool clearCalled = false;

  public:
    bool getAndResetClearCalled() override;

    void clear() override;

    bool equals(std::any other) override;

    virtual int hashCode();

    void copyTo(std::shared_ptr<AttributeImpl> target) override;

    void reflectWith(AttributeReflector reflector) override;

  protected:
    std::shared_ptr<CheckClearAttributesAttributeImpl> shared_from_this()
    {
      return std::static_pointer_cast<CheckClearAttributesAttributeImpl>(
          org.apache.lucene.util.AttributeImpl::shared_from_this());
    }
  };

  // graphOffsetsAreCorrect validates:
  //   - graph offsets are correct (all tokens leaving from
  //     pos X have the same startOffset; all tokens
  //     arriving to pos Y have the same endOffset)
public:
  static void assertTokenStreamContents(
      std::shared_ptr<TokenStream> ts, std::deque<std::wstring> &output,
      std::deque<int> &startOffsets, std::deque<int> &endOffsets,
      std::deque<std::wstring> &types, std::deque<int> &posIncrements,
      std::deque<int> &posLengths, std::optional<int> &finalOffset,
      std::optional<int> &finalPosInc, std::deque<bool> &keywordAtts,
      bool graphOffsetsAreCorrect,
      std::deque<std::deque<char>> &payloads) ;

  static void assertTokenStreamContents(
      std::shared_ptr<TokenStream> ts, std::deque<std::wstring> &output,
      std::deque<int> &startOffsets, std::deque<int> &endOffsets,
      std::deque<std::wstring> &types, std::deque<int> &posIncrements,
      std::deque<int> &posLengths, std::optional<int> &finalOffset,
      std::deque<bool> &keywordAtts,
      bool graphOffsetsAreCorrect) ;

  static void assertTokenStreamContents(
      std::shared_ptr<TokenStream> ts, std::deque<std::wstring> &output,
      std::deque<int> &startOffsets, std::deque<int> &endOffsets,
      std::deque<std::wstring> &types, std::deque<int> &posIncrements,
      std::deque<int> &posLengths, std::optional<int> &finalOffset,
      bool graphOffsetsAreCorrect) ;

  static void assertTokenStreamContents(
      std::shared_ptr<TokenStream> ts, std::deque<std::wstring> &output,
      std::deque<int> &startOffsets, std::deque<int> &endOffsets,
      std::deque<std::wstring> &types, std::deque<int> &posIncrements,
      std::deque<int> &posLengths,
      std::optional<int> &finalOffset) ;

  static void assertTokenStreamContents(
      std::shared_ptr<TokenStream> ts, std::deque<std::wstring> &output,
      std::deque<int> &startOffsets, std::deque<int> &endOffsets,
      std::deque<std::wstring> &types, std::deque<int> &posIncrements,
      std::optional<int> &finalOffset) ;

  static void assertTokenStreamContents(
      std::shared_ptr<TokenStream> ts, std::deque<std::wstring> &output,
      std::deque<int> &startOffsets, std::deque<int> &endOffsets,
      std::deque<std::wstring> &types,
      std::deque<int> &posIncrements) ;

  static void assertTokenStreamContents(
      std::shared_ptr<TokenStream> ts, std::deque<std::wstring> &output,
      std::deque<int> &startOffsets, std::deque<int> &endOffsets,
      std::deque<std::wstring> &types, std::deque<int> &posIncrements,
      std::deque<int> &posLengths) ;

  static void assertTokenStreamContents(
      std::shared_ptr<TokenStream> ts,
      std::deque<std::wstring> &output) ;

  static void assertTokenStreamContents(
      std::shared_ptr<TokenStream> ts, std::deque<std::wstring> &output,
      std::deque<std::wstring> &types) ;

  static void
  assertTokenStreamContents(std::shared_ptr<TokenStream> ts,
                            std::deque<std::wstring> &output,
                            std::deque<int> &posIncrements) ;

  static void
  assertTokenStreamContents(std::shared_ptr<TokenStream> ts,
                            std::deque<std::wstring> &output,
                            std::deque<int> &startOffsets,
                            std::deque<int> &endOffsets) ;

  static void assertTokenStreamContents(
      std::shared_ptr<TokenStream> ts, std::deque<std::wstring> &output,
      std::deque<int> &startOffsets, std::deque<int> &endOffsets,
      std::optional<int> &finalOffset) ;

  static void assertTokenStreamContents(
      std::shared_ptr<TokenStream> ts, std::deque<std::wstring> &output,
      std::deque<int> &startOffsets, std::deque<int> &endOffsets,
      std::deque<int> &posIncrements) ;

  static void assertTokenStreamContents(
      std::shared_ptr<TokenStream> ts, std::deque<std::wstring> &output,
      std::deque<int> &startOffsets, std::deque<int> &endOffsets,
      std::deque<int> &posIncrements,
      std::optional<int> &finalOffset) ;

  static void assertTokenStreamContents(
      std::shared_ptr<TokenStream> ts, std::deque<std::wstring> &output,
      std::deque<int> &startOffsets, std::deque<int> &endOffsets,
      std::deque<int> &posIncrements, std::deque<int> &posLengths,
      std::optional<int> &finalOffset) ;

  static void
  assertAnalyzesTo(std::shared_ptr<Analyzer> a, const std::wstring &input,
                   std::deque<std::wstring> &output,
                   std::deque<int> &startOffsets, std::deque<int> &endOffsets,
                   std::deque<std::wstring> &types,
                   std::deque<int> &posIncrements) ;

  static void assertAnalyzesTo(std::shared_ptr<Analyzer> a,
                               const std::wstring &input,
                               std::deque<std::wstring> &output,
                               std::deque<int> &startOffsets,
                               std::deque<int> &endOffsets,
                               std::deque<std::wstring> &types,
                               std::deque<int> &posIncrements,
                               std::deque<int> &posLengths) ;

  static void assertAnalyzesTo(
      std::shared_ptr<Analyzer> a, const std::wstring &input,
      std::deque<std::wstring> &output, std::deque<int> &startOffsets,
      std::deque<int> &endOffsets, std::deque<std::wstring> &types,
      std::deque<int> &posIncrements, std::deque<int> &posLengths,
      bool graphOffsetsAreCorrect) ;

  static void
  assertAnalyzesTo(std::shared_ptr<Analyzer> a, const std::wstring &input,
                   std::deque<std::wstring> &output,
                   std::deque<int> &startOffsets, std::deque<int> &endOffsets,
                   std::deque<std::wstring> &types,
                   std::deque<int> &posIncrements,
                   std::deque<int> &posLengths, bool graphOffsetsAreCorrect,
                   std::deque<std::deque<char>> &payloads) ;

  static void
  assertAnalyzesTo(std::shared_ptr<Analyzer> a, const std::wstring &input,
                   std::deque<std::wstring> &output) ;

  static void
  assertAnalyzesTo(std::shared_ptr<Analyzer> a, const std::wstring &input,
                   std::deque<std::wstring> &output,
                   std::deque<std::wstring> &types) ;

  static void
  assertAnalyzesTo(std::shared_ptr<Analyzer> a, const std::wstring &input,
                   std::deque<std::wstring> &output,
                   std::deque<int> &posIncrements) ;

  static void assertAnalyzesToPositions(
      std::shared_ptr<Analyzer> a, const std::wstring &input,
      std::deque<std::wstring> &output, std::deque<int> &posIncrements,
      std::deque<int> &posLengths) ;

  static void assertAnalyzesToPositions(
      std::shared_ptr<Analyzer> a, const std::wstring &input,
      std::deque<std::wstring> &output, std::deque<std::wstring> &types,
      std::deque<int> &posIncrements,
      std::deque<int> &posLengths) ;

  static void assertAnalyzesTo(std::shared_ptr<Analyzer> a,
                               const std::wstring &input,
                               std::deque<std::wstring> &output,
                               std::deque<int> &startOffsets,
                               std::deque<int> &endOffsets) ;

  static void
  assertAnalyzesTo(std::shared_ptr<Analyzer> a, const std::wstring &input,
                   std::deque<std::wstring> &output,
                   std::deque<int> &startOffsets, std::deque<int> &endOffsets,
                   std::deque<int> &posIncrements) ;

  static void checkResetException(std::shared_ptr<Analyzer> a,
                                  const std::wstring &input) ;

  // simple utility method for testing stemmers

  static void checkOneTerm(std::shared_ptr<Analyzer> a,
                           const std::wstring &input,
                           const std::wstring &expected) ;

  /** utility method for blasting tokenstreams with data to make sure they don't
   * do anything crazy */
  static void checkRandomData(std::shared_ptr<Random> random,
                              std::shared_ptr<Analyzer> a,
                              int iterations) ;

  /** utility method for blasting tokenstreams with data to make sure they don't
   * do anything crazy */
  static void checkRandomData(std::shared_ptr<Random> random,
                              std::shared_ptr<Analyzer> a, int iterations,
                              int maxWordLength) ;

  /**
   * utility method for blasting tokenstreams with data to make sure they don't
   * do anything crazy
   * @param simple true if only ascii strings will be used (try to avoid)
   */
  static void checkRandomData(std::shared_ptr<Random> random,
                              std::shared_ptr<Analyzer> a, int iterations,
                              bool simple) ;

  /** Asserts that the given stream has expected number of tokens. */
  static void
  assertStreamHasNumberOfTokens(std::shared_ptr<TokenStream> ts,
                                int expectedCount) ;

public:
  class AnalysisThread : public Thread
  {
    GET_CLASS_NAME(AnalysisThread)
  public:
    const int iterations;
    const int maxWordLength;
    const int64_t seed;
    const std::shared_ptr<Analyzer> a;
    const bool useCharFilter;
    const bool simple;
    const bool graphOffsetsAreCorrect;
    const std::shared_ptr<RandomIndexWriter> iw;
    const std::shared_ptr<CountDownLatch> latch;

    // NOTE: not volatile because we don't want the tests to
    // add memory barriers (ie alter how threads
    // interact)... so this is just "best effort":
    bool failed = false;

    AnalysisThread(int64_t seed, std::shared_ptr<CountDownLatch> latch,
                   std::shared_ptr<Analyzer> a, int iterations,
                   int maxWordLength, bool useCharFilter, bool simple,
                   bool graphOffsetsAreCorrect,
                   std::shared_ptr<RandomIndexWriter> iw);

    void run() override;

  protected:
    std::shared_ptr<AnalysisThread> shared_from_this()
    {
      return std::static_pointer_cast<AnalysisThread>(
          Thread::shared_from_this());
    }
  };

public:
  static void checkRandomData(std::shared_ptr<Random> random,
                              std::shared_ptr<Analyzer> a, int iterations,
                              int maxWordLength,
                              bool simple) ;

  static void checkRandomData(std::shared_ptr<Random> random,
                              std::shared_ptr<Analyzer> a, int iterations,
                              int maxWordLength, bool simple,
                              bool graphOffsetsAreCorrect) ;

private:
  static void
  checkRandomData(std::shared_ptr<Random> random, std::shared_ptr<Analyzer> a,
                  int iterations, int maxWordLength, bool useCharFilter,
                  bool simple, bool graphOffsetsAreCorrect,
                  std::shared_ptr<RandomIndexWriter> iw) ;

public:
  static std::wstring escape(const std::wstring &s);

  static void
  checkAnalysisConsistency(std::shared_ptr<Random> random,
                           std::shared_ptr<Analyzer> a, bool useCharFilter,
                           const std::wstring &text) ;

  static void
  checkAnalysisConsistency(std::shared_ptr<Random> random,
                           std::shared_ptr<Analyzer> a, bool useCharFilter,
                           const std::wstring &text,
                           bool graphOffsetsAreCorrect) ;

private:
  static void checkAnalysisConsistency(
      std::shared_ptr<Random> random, std::shared_ptr<Analyzer> a,
      bool useCharFilter, const std::wstring &text, bool graphOffsetsAreCorrect,
      std::shared_ptr<Field> field) ;

protected:
  virtual std::wstring toDot(std::shared_ptr<Analyzer> a,
                             const std::wstring &inputText) ;

  virtual void toDotFile(std::shared_ptr<Analyzer> a,
                         const std::wstring &inputText,
                         const std::wstring &localFileName) ;

public:
  static std::deque<int> toIntArray(std::deque<int> &deque);

protected:
  static std::shared_ptr<MockTokenizer>
  whitespaceMockTokenizer(std::shared_ptr<Reader> input) ;

  static std::shared_ptr<MockTokenizer>
  whitespaceMockTokenizer(const std::wstring &input) ;

  static std::shared_ptr<MockTokenizer>
  keywordMockTokenizer(std::shared_ptr<Reader> input) ;

  static std::shared_ptr<MockTokenizer>
  keywordMockTokenizer(const std::wstring &input) ;

  /** Returns a random AttributeFactory impl */
public:
  static std::shared_ptr<AttributeFactory>
  newAttributeFactory(std::shared_ptr<Random> random);

  /** Returns a random AttributeFactory impl */
  static std::shared_ptr<AttributeFactory> newAttributeFactory();

private:
  static std::wstring toString(std::shared_ptr<Set<std::wstring>> strings);

  /**
   * Enumerates all accepted strings in the token graph created by the analyzer
   * on the provided text, and then asserts that it's equal to the expected
   * strings. Uses {@link TokenStreamToAutomaton} to create an automaton.
   * Asserts the finite strings of the automaton are all and only the given
   * valid strings.
   * @param analyzer analyzer containing the SynonymFilter under test.
   * @param text text to be analyzed.
   * @param expectedStrings all expected finite strings.
   */
public:
  static void assertGraphStrings(
      std::shared_ptr<Analyzer> analyzer, const std::wstring &text,
      std::deque<std::wstring> &expectedStrings) ;

  /**
   * Enumerates all accepted strings in the token graph created by the already
   * initialized {@link TokenStream}.
   */
  static void assertGraphStrings(
      std::shared_ptr<TokenStream> tokenStream,
      std::deque<std::wstring> &expectedStrings) ;

  /** Returns all paths accepted by the token stream graph produced by analyzing
   * text with the provided analyzer.  The tokens {@link CharTermAttribute}
   * values are concatenated, and separated with space. */
  static std::shared_ptr<Set<std::wstring>>
  getGraphStrings(std::shared_ptr<Analyzer> analyzer,
                  const std::wstring &text) ;

  /** Returns all paths accepted by the token stream graph produced by the
   * already initialized {@link TokenStream}. */
  static std::shared_ptr<Set<std::wstring>>
  getGraphStrings(std::shared_ptr<TokenStream> tokenStream) ;

  /** Returns a {@code std::wstring} summary of the tokens this analyzer produces on
   * this text */
  static std::wstring toString(std::shared_ptr<Analyzer> analyzer,
                               const std::wstring &text) ;

protected:
  std::shared_ptr<BaseTokenStreamTestCase> shared_from_this()
  {
    return std::static_pointer_cast<BaseTokenStreamTestCase>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/
