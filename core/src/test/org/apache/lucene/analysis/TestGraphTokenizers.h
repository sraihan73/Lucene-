#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class Token;
}

namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class OffsetAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class PositionIncrementAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class PositionLengthAttribute;
}
namespace org::apache::lucene::analysis
{
class TokenStream;
}
namespace org::apache::lucene::util::automaton
{
class Automaton;
}

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

using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using PositionLengthAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionLengthAttribute;
using Automata = org::apache::lucene::util::automaton::Automata;
using Automaton = org::apache::lucene::util::automaton::Automaton;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.util.automaton.Operations.DEFAULT_MAX_DETERMINIZED_STATES;

class TestGraphTokenizers : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestGraphTokenizers)

  // Makes a graph TokenStream from the string; separate
  // positions with single space, multiple tokens at the same
  // position with /, and add optional position length with
  // :.  EG "a b c" is a simple chain, "a/x b c" adds 'x'
  // over 'a' at position 0 with posLen=1, "a/x:3 b c" adds
  // 'x' over a with posLen=3.  Tokens are in normal-form!
  // So, offsets are computed based on the first token at a
  // given position.  NOTE: each token must be a single
  // character!  We assume this when computing offsets...

  // NOTE: all input tokens must be length 1!!!  This means
  // you cannot turn on MockCharFilter when random
  // testing...

private:
  class GraphTokenizer : public Tokenizer
  {
    GET_CLASS_NAME(GraphTokenizer)
  private:
    std::deque<std::shared_ptr<Token>> tokens;
    int upto = 0;
    int inputLength = 0;

    const std::shared_ptr<CharTermAttribute> termAtt =
        addAttribute(CharTermAttribute::typeid);
    const std::shared_ptr<OffsetAttribute> offsetAtt =
        addAttribute(OffsetAttribute::typeid);
    const std::shared_ptr<PositionIncrementAttribute> posIncrAtt =
        addAttribute(PositionIncrementAttribute::typeid);
    const std::shared_ptr<PositionLengthAttribute> posLengthAtt =
        addAttribute(PositionLengthAttribute::typeid);

  public:
    void reset()  override;

    bool incrementToken()  override;

    void end()  override;

  private:
    void fillTokens() ;

  protected:
    std::shared_ptr<GraphTokenizer> shared_from_this()
    {
      return std::static_pointer_cast<GraphTokenizer>(
          Tokenizer::shared_from_this());
    }
  };

public:
  virtual void testMockGraphTokenFilterBasic() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestGraphTokenizers> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestGraphTokenizers> outerInstance);

  protected:
    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          Analyzer::shared_from_this());
    }
  };

public:
  virtual void testMockGraphTokenFilterOnGraphInput() ;

private:
  class AnalyzerAnonymousInnerClass2 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass2)
  private:
    std::shared_ptr<TestGraphTokenizers> outerInstance;

  public:
    AnalyzerAnonymousInnerClass2(
        std::shared_ptr<TestGraphTokenizers> outerInstance);

  protected:
    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass2>(
          Analyzer::shared_from_this());
    }
  };

  // Just deletes (leaving hole) token 'a':
private:
  class RemoveATokens final : public TokenFilter
  {
    GET_CLASS_NAME(RemoveATokens)
  private:
    int pendingPosInc = 0;

    const std::shared_ptr<CharTermAttribute> termAtt =
        addAttribute(CharTermAttribute::typeid);
    const std::shared_ptr<PositionIncrementAttribute> posIncAtt =
        addAttribute(PositionIncrementAttribute::typeid);

  public:
    RemoveATokens(std::shared_ptr<TokenStream> in_);

    void reset()  override;

    void end()  override;

    bool incrementToken()  override;

  protected:
    std::shared_ptr<RemoveATokens> shared_from_this()
    {
      return std::static_pointer_cast<RemoveATokens>(
          TokenFilter::shared_from_this());
    }
  };

public:
  virtual void testMockGraphTokenFilterBeforeHoles() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestGraphTokenizers> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestGraphTokenizers> outerInstance);

  protected:
    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          Analyzer::shared_from_this());
    }
  };

public:
  virtual void testMockGraphTokenFilterAfterHoles() ;

private:
  class AnalyzerAnonymousInnerClass2 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass2)
  private:
    std::shared_ptr<TestGraphTokenizers> outerInstance;

  public:
    AnalyzerAnonymousInnerClass2(
        std::shared_ptr<TestGraphTokenizers> outerInstance);

  protected:
    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass2>(
          Analyzer::shared_from_this());
    }
  };

public:
  virtual void testMockGraphTokenFilterRandom() ;

private:
  class AnalyzerAnonymousInnerClass3 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass3)
  private:
    std::shared_ptr<TestGraphTokenizers> outerInstance;

  public:
    AnalyzerAnonymousInnerClass3(
        std::shared_ptr<TestGraphTokenizers> outerInstance);

  protected:
    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass3>(
          Analyzer::shared_from_this());
    }
  };

  // Two MockGraphTokenFilters
public:
  virtual void testDoubleMockGraphTokenFilterRandom() ;

private:
  class AnalyzerAnonymousInnerClass4 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass4)
  private:
    std::shared_ptr<TestGraphTokenizers> outerInstance;

  public:
    AnalyzerAnonymousInnerClass4(
        std::shared_ptr<TestGraphTokenizers> outerInstance);

  protected:
    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass4> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass4>(
          Analyzer::shared_from_this());
    }
  };

public:
  virtual void
  testMockGraphTokenFilterBeforeHolesRandom() ;

private:
  class AnalyzerAnonymousInnerClass5 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass5)
  private:
    std::shared_ptr<TestGraphTokenizers> outerInstance;

  public:
    AnalyzerAnonymousInnerClass5(
        std::shared_ptr<TestGraphTokenizers> outerInstance);

  protected:
    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass5> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass5>(
          Analyzer::shared_from_this());
    }
  };

public:
  virtual void
  testMockGraphTokenFilterAfterHolesRandom() ;

private:
  class AnalyzerAnonymousInnerClass6 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass6)
  private:
    std::shared_ptr<TestGraphTokenizers> outerInstance;

  public:
    AnalyzerAnonymousInnerClass6(
        std::shared_ptr<TestGraphTokenizers> outerInstance);

  protected:
    std::shared_ptr<TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass6> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass6>(
          Analyzer::shared_from_this());
    }
  };

private:
  static std::shared_ptr<Token> token(const std::wstring &term, int posInc,
                                      int posLength);

  static std::shared_ptr<Token> token(const std::wstring &term, int posInc,
                                      int posLength, int startOffset,
                                      int endOffset);

public:
  virtual void testSingleToken() ;

  virtual void testMultipleHoles() ;

  virtual void testSynOverMultipleHoles() ;

  // for debugging!
  /*
  private static void toDot(Automaton a) throws IOException {
    final std::wstring s = a.toDot();
    Writer w = new OutputStreamWriter(new FileOutputStream("/x/tmp/out.dot"));
    w.write(s);
    w.close();
    System.out.println("TEST: saved to /x/tmp/out.dot");
  }
  */

private:
  static const std::shared_ptr<Automaton> SEP_A;
  static const std::shared_ptr<Automaton> HOLE_A;

  std::shared_ptr<Automaton> join(std::deque<std::wstring> &strings);

  std::shared_ptr<Automaton> join(std::deque<Automaton> &as);

  std::shared_ptr<Automaton> s2a(const std::wstring &s);

public:
  virtual void testTwoTokens() ;

  virtual void testHole() ;

  virtual void testOverlappedTokensSausage() ;

  virtual void testOverlappedTokensLattice() ;

  virtual void testSynOverHole() ;

  virtual void testSynOverHole2() ;

  virtual void testOverlappedTokensLattice2() ;

  virtual void testToDot() ;

  virtual void testStartsWithHole() ;

  virtual void testEndsWithHole() ;

  virtual void testSynHangingOverEnd() ;

  /** Returns all paths */
private:
  std::shared_ptr<Set<std::wstring>>
  toPathStrings(std::shared_ptr<Automaton> a);

  void assertSameLanguage(std::shared_ptr<Automaton> expected,
                          std::shared_ptr<TokenStream> ts) ;

  void assertSameLanguage(std::shared_ptr<Automaton> expected,
                          std::shared_ptr<Automaton> actual);

public:
  virtual void testTokenStreamGraphWithHoles() ;

protected:
  std::shared_ptr<TestGraphTokenizers> shared_from_this()
  {
    return std::static_pointer_cast<TestGraphTokenizers>(
        BaseTokenStreamTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis
