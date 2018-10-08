#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/synonym/SynonymGraphFilter.h"

#include  "core/src/java/org/apache/lucene/analysis/core/FlattenGraphFilter.h"
#include  "core/src/java/org/apache/lucene/analysis/synonym/SynonymMap.h"
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStreamComponents.h"
#include  "core/src/java/org/apache/lucene/analysis/synonym/Builder.h"
#include  "core/src/java/org/apache/lucene/util/automaton/Automaton.h"
#include  "core/src/java/org/apache/lucene/util/IntsRef.h"
#include  "core/src/java/org/apache/lucene/util/automaton/Builder.h"
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

namespace org::apache::lucene::analysis::synonym
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using FlattenGraphFilter =
    org::apache::lucene::analysis::core::FlattenGraphFilter;
using namespace org::apache::lucene::analysis::tokenattributes;
using IntsRef = org::apache::lucene::util::IntsRef;
using Automaton = org::apache::lucene::util::automaton::Automaton;

class TestSynonymGraphFilter : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestSynonymGraphFilter)

  /** Set as a side effect by {@link #getAnalyzer} and {@link
   * #getFlattenAnalyzer}. */
private:
  std::shared_ptr<SynonymGraphFilter> synFilter;
  std::shared_ptr<FlattenGraphFilter> flattenFilter;

public:
  virtual void testBasicKeepOrigOneOutput() ;

  virtual void testMixedKeepOrig() ;

  virtual void testNoParseAfterBuffer() ;

  virtual void testOneInputMultipleOutputKeepOrig() ;

  /**
   * Verify type of token and positionLength after analyzer.
   */
  virtual void testPositionLengthAndTypeSimple() ;

  /**
   * parse a syn file with some escaped syntax chars
   */
  virtual void testEscapedStuff() ;

  /**
   * parse a syn file with bad syntax
   */
  virtual void testInvalidAnalyzesToNothingOutput() ;

  /**
   * parse a syn file with bad syntax
   */
  virtual void testInvalidDoubleMap() ;

  /**
   * Tests some simple examples from the solr wiki
   */
  virtual void testSimple() ;

  virtual void testBufferLength() ;

private:
  std::shared_ptr<Analyzer>
  solrSynsToAnalyzer(const std::wstring &syns) throw(IOException,
                                                     ParseException);

public:
  virtual void testMoreThanOneLookAhead() ;

  virtual void testLookaheadAfterParse() ;

  virtual void testLookaheadSecondParse() ;

  virtual void testOneInputMultipleOutputNoKeepOrig() ;

  virtual void
  testOneInputMultipleOutputMixedKeepOrig() ;

  virtual void testSynAtEnd() ;

  virtual void testTwoSynsInARow() ;

  virtual void testBasicKeepOrigTwoOutputs() ;

  virtual void testNoCaptureIfNoMatch() ;

  virtual void testBasicNotKeepOrigOneOutput() ;

  virtual void testBasicNoKeepOrigTwoOutputs() ;

  virtual void testIgnoreCase() ;

  virtual void testDoNotIgnoreCase() ;

  virtual void testBufferedFinish1() ;

  virtual void testBufferedFinish2() ;

  virtual void testCanReuse() ;

  /** Multiple input tokens map_obj to a single output token */
  virtual void testManyToOne() ;

  virtual void testBufferAfterMatch() ;

  virtual void testZeroSyns() ;

  virtual void testOutputHangsOffEnd() ;

  virtual void testDedup() ;

  virtual void testNoDedup() ;

  virtual void testMatching() ;

  virtual void testBasic1() ;

  virtual void testBasic2() ;

  /** If we expand synonyms during indexing, it's a bit better than
   *  SynonymFilter is today, but still necessarily has false
   *  positive and negative PhraseQuery matches because we do not
   *  index posLength, so we lose information. */
  virtual void testFlattenedGraph() ;

  // Needs TermAutomatonQuery, which is in sandbox still:
  /*
  public void testAccurateGraphQuery1() throws Exception {
    Directory dir = newDirectory();
    RandomIndexWriter w = new RandomIndexWriter(random(), dir);
    Document doc = new Document();
    doc.add(newTextField("field", "wtf happened", Field.Store.NO));
    w.addDocument(doc);
    IndexReader r = w.getReader();
    w.close();

    IndexSearcher s = newSearcher(r);

    SynonymMap.Builder b = new SynonymMap.Builder();
    add(b, "what the fudge", "wtf", true);

    SynonymMap map_obj = b.build();

    TokenStreamToTermAutomatonQuery ts2q = new
  TokenStreamToTermAutomatonQuery();

    TokenStream in = new CannedTokenStream(0, 23, new Token[] {
        token("what", 1, 1, 0, 4),
        token("the", 1, 1, 5, 8),
        token("fudge", 1, 1, 9, 14),
        token("happened", 1, 1, 15, 23),
      });

    assertEquals(1, s.count(ts2q.toQuery("field", new SynonymGraphFilter(in,
  map_obj, true))));

    in = new CannedTokenStream(0, 12, new Token[] {
        token("wtf", 1, 1, 0, 3),
        token("happened", 1, 1, 4, 12),
      });

    assertEquals(1, s.count(ts2q.toQuery("field", new SynonymGraphFilter(in,
  map_obj, true))));

    // "what happened" should NOT match:
    in = new CannedTokenStream(0, 13, new Token[] {
        token("what", 1, 1, 0, 4),
        token("happened", 1, 1, 5, 13),
      });
    assertEquals(0, s.count(ts2q.toQuery("field", new SynonymGraphFilter(in,
  map_obj, true))));

    IOUtils.close(r, dir);
  }
  */

  /** If we expand synonyms at search time, the results are correct. */
  // Needs TermAutomatonQuery, which is in sandbox still:
  /*
  public void testAccurateGraphQuery2() throws Exception {
    Directory dir = newDirectory();
    RandomIndexWriter w = new RandomIndexWriter(random(), dir);
    Document doc = new Document();
    doc.add(newTextField("field", "say wtf happened", Field.Store.NO));
    w.addDocument(doc);
    IndexReader r = w.getReader();
    w.close();

    IndexSearcher s = newSearcher(r);

    SynonymMap.Builder b = new SynonymMap.Builder();
    add(b, "what the fudge", "wtf", true);

    SynonymMap map_obj = b.build();

    TokenStream in = new CannedTokenStream(0, 26, new Token[] {
        token("say", 1, 1, 0, 3),
        token("what", 1, 1, 3, 7),
        token("the", 1, 1, 8, 11),
        token("fudge", 1, 1, 12, 17),
        token("happened", 1, 1, 18, 26),
      });

    TokenStreamToTermAutomatonQuery ts2q = new
  TokenStreamToTermAutomatonQuery();

    assertEquals(1, s.count(ts2q.toQuery("field", new SynonymGraphFilter(in,
  map_obj, true))));

    // "what happened" should NOT match:
    in = new CannedTokenStream(0, 13, new Token[] {
        token("what", 1, 1, 0, 4),
        token("happened", 1, 1, 5, 13),
      });
    assertEquals(0, s.count(ts2q.toQuery("field", new SynonymGraphFilter(in,
  map_obj, true))));

    IOUtils.close(r, dir);
  }
  */

  // Needs TermAutomatonQuery, which is in sandbox still:
  /*
  public void testAccurateGraphQuery3() throws Exception {
    Directory dir = newDirectory();
    RandomIndexWriter w = new RandomIndexWriter(random(), dir);
    Document doc = new Document();
    doc.add(newTextField("field", "say what the fudge happened",
  Field.Store.NO)); w.addDocument(doc); IndexReader r = w.getReader();
    w.close();

    IndexSearcher s = newSearcher(r);

    SynonymMap.Builder b = new SynonymMap.Builder();
    add(b, "wtf", "what the fudge", true);

    SynonymMap map_obj = b.build();

    TokenStream in = new CannedTokenStream(0, 15, new Token[] {
        token("say", 1, 1, 0, 3),
        token("wtf", 1, 1, 3, 6),
        token("happened", 1, 1, 7, 15),
      });

    TokenStreamToTermAutomatonQuery ts2q = new
  TokenStreamToTermAutomatonQuery();

    assertEquals(1, s.count(ts2q.toQuery("field", new SynonymGraphFilter(in,
  map_obj, true))));

    // "what happened" should NOT match:
    in = new CannedTokenStream(0, 13, new Token[] {
        token("what", 1, 1, 0, 4),
        token("happened", 1, 1, 5, 13),
      });
    assertEquals(0, s.count(ts2q.toQuery("field", new SynonymGraphFilter(in,
  map_obj, true))));

    IOUtils.close(r, dir);
  }

  private static Token token(std::wstring term, int posInc, int posLength, int
  startOffset, int endOffset) { final Token t = new Token(term, startOffset,
  endOffset); t.setPositionIncrement(posInc); t.setPositionLength(posLength);
    return t;
  }
  */

private:
  std::wstring randomNonEmptyString();

  // Adds MockGraphTokenFilter after SynFilter:
public:
  virtual void testRandomGraphAfter() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestSynonymGraphFilter> outerInstance;

    std::shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj;
    bool ignoreCase = false;
    bool doFlatten = false;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestSynonymGraphFilter> outerInstance,
        std::shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj,
        bool ignoreCase, bool doFlatten);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  virtual void testEmptyStringInput() ;

  /** simple random test, doesn't verify correctness.
   *  does verify it doesnt throw exceptions, or that the stream doesn't
   * misbehave
   */
  virtual void testRandom2() ;

  /** simple random test like testRandom2, but for larger docs
   */
  virtual void testRandomHuge() ;

  virtual void testEmptyTerm() ;

  // LUCENE-3375
  virtual void testVanishingTermsNoFlatten() ;

  // LUCENE-3375
  virtual void testVanishingTermsWithFlatten() ;

  virtual void testBuilderDedup() ;

  virtual void testBuilderNoDedup() ;

  virtual void testRecursion1() ;

  virtual void testRecursion2() ;

  virtual void testRecursion3() ;

  virtual void testRecursion4() ;

  virtual void testKeepOrig() ;

  /**
   * verify type of token and positionLengths on synonyms of different word
   * counts, with non preserving, explicit rules.
   */
  virtual void testNonPreservingMultiwordSynonyms() ;

private:
  std::shared_ptr<Analyzer>
  getAnalyzer(std::shared_ptr<SynonymMap::Builder> b,
              bool const ignoreCase) ;

private:
  class AnalyzerAnonymousInnerClass2 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass2)
  private:
    std::shared_ptr<TestSynonymGraphFilter> outerInstance;

    bool ignoreCase = false;
    std::shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj;

  public:
    AnalyzerAnonymousInnerClass2(
        std::shared_ptr<TestSynonymGraphFilter> outerInstance, bool ignoreCase,
        std::shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap>
            map_obj);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass2>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

  /** Appends FlattenGraphFilter too */
private:
  std::shared_ptr<Analyzer>
  getFlattenAnalyzer(std::shared_ptr<SynonymMap::Builder> b,
                     bool ignoreCase) ;

private:
  class AnalyzerAnonymousInnerClass3 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass3)
  private:
    std::shared_ptr<TestSynonymGraphFilter> outerInstance;

    bool ignoreCase = false;
    std::shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj;

  public:
    AnalyzerAnonymousInnerClass3(
        std::shared_ptr<TestSynonymGraphFilter> outerInstance, bool ignoreCase,
        std::shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap>
            map_obj);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass3>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

private:
  void add(std::shared_ptr<SynonymMap::Builder> b, const std::wstring &input,
           const std::wstring &output, bool keepOrig);

  std::deque<wchar_t> randomBinaryChars(int minLen, int maxLen, double bias,
                                         wchar_t base);

  static std::wstring toTokenString(std::deque<wchar_t> &chars);

private:
  class OneSyn : public std::enable_shared_from_this<OneSyn>
  {
    GET_CLASS_NAME(OneSyn)
  public:
    std::deque<wchar_t> in_;
    std::deque<wchar_t> out;
    bool keepOrig = false;

    virtual std::wstring toString();
  };

public:
  virtual void testRandomSyns() ;

  /** Only used when true equality is too costly to check! */
private:
  bool approxEquals(std::shared_ptr<Automaton> actual,
                    std::shared_ptr<Automaton> expected);

  bool approxSubsetOf(std::shared_ptr<Automaton> a1,
                      std::shared_ptr<Automaton> a2);

  /** Like {@link Operations#run} except the incoming automaton is allowed to be
   * non-deterministic. */
  static bool accepts(std::shared_ptr<Automaton> a,
                      std::shared_ptr<IntsRef> path);

  /** Stupid, slow brute-force, yet hopefully bug-free, synonym filter. */
  std::shared_ptr<Automaton>
  slowSynFilter(const std::wstring &doc,
                std::deque<std::shared_ptr<OneSyn>> &syns, bool flatten);

  /** Just creates a side path from startState to endState with the provided
   * tokens. */
  static void addSidePath(std::shared_ptr<Automaton::Builder> a, int startState,
                          int endState, std::deque<wchar_t> &tokens,
                          std::deque<int> &flatStates);

  std::shared_ptr<Automaton>
  toAutomaton(std::shared_ptr<TokenStream> ts) ;

  /*
  private std::wstring toDot(TokenStream ts) throws IOException {
    PositionIncrementAttribute posIncAtt =
  ts.addAttribute(PositionIncrementAttribute.class); PositionLengthAttribute
  posLenAtt = ts.addAttribute(PositionLengthAttribute.class); CharTermAttribute
  termAtt = ts.addAttribute(CharTermAttribute.class); TypeAttribute typeAtt =
  ts.addAttribute(TypeAttribute.class); ts.reset(); int srcNode = -1; int
  destNode = -1;

    StringBuilder b = new StringBuilder();
    b.append("digraph Automaton {\n");
    b.append("  rankdir = LR\n");
    b.append("  node [width=0.2, height=0.2, fontsize=8]\n");
    b.append("  initial [shape=plaintext,label=\"\"]\n");
    b.append("  initial -> 0\n");

    while (ts.incrementToken()) {
      int posInc = posIncAtt.getPositionIncrement();
      if (posInc != 0) {
        srcNode += posInc;
        b.append("  ");
        b.append(srcNode);
        b.append(" [shape=circle,label=\"" + srcNode + "\"]\n");
      }
      destNode = srcNode + posLenAtt.getPositionLength();
      b.append("  ");
      b.append(srcNode);
      b.append(" -> ");
      b.append(destNode);
      b.append(" [label=\"");
      b.append(termAtt);
      b.append("\"");
      if (typeAtt.type().equals("word") == false) {
        b.append(" color=red");
      }
      b.append("]\n");
    }
    ts.end();
    ts.close();

    b.append('}');
    return b.toString();
  }
  */

  /** Renumbers nodes according to their topo sort */
  std::shared_ptr<Automaton> topoSort(std::shared_ptr<Automaton> in_);

  /**
   * verify type of token and positionLengths on synonyms of different word
   * counts.
   */
public:
  virtual void testPositionLengthAndType() ;

  virtual void testMultiwordOffsets() ;

  virtual void testIncludeOrig() ;

  virtual void testUpperCase() ;

private:
  void assertMapping(const std::wstring &inputString,
                     const std::wstring &outputString) ;

public:
  class CustomAnalyzer : public Analyzer
  {
    GET_CLASS_NAME(CustomAnalyzer)
  private:
    std::shared_ptr<SynonymMap> synonymMap;

  public:
    CustomAnalyzer(std::shared_ptr<SynonymMap> synonymMap);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &s) override;

  protected:
    std::shared_ptr<CustomAnalyzer> shared_from_this()
    {
      return std::static_pointer_cast<CustomAnalyzer>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestSynonymGraphFilter> shared_from_this()
  {
    return std::static_pointer_cast<TestSynonymGraphFilter>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/synonym/
