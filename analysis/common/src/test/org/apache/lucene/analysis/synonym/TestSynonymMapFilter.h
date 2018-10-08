#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::synonym
{
class SynonymMap;
}

namespace org::apache::lucene::analysis::synonym
{
class Builder;
}
namespace org::apache::lucene::analysis
{
class Tokenizer;
}
namespace org::apache::lucene::analysis::synonym
{
class SynonymFilter;
}
namespace org::apache::lucene::analysis
{
class Analyzer;
}
namespace org::apache::lucene::analysis
{
class TokenStreamComponents;
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
namespace org::apache::lucene::analysis::synonym
{

using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using namespace org::apache::lucene::analysis::tokenattributes;

class TestSynonymMapFilter : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestSynonymMapFilter)

private:
  std::shared_ptr<SynonymMap::Builder> b;
  std::shared_ptr<Tokenizer> tokensIn;
  std::shared_ptr<SynonymFilter> tokensOut;
  std::shared_ptr<CharTermAttribute> termAtt;
  std::shared_ptr<PositionIncrementAttribute> posIncrAtt;
  std::shared_ptr<PositionLengthAttribute> posLenAtt;
  std::shared_ptr<OffsetAttribute> offsetAtt;

  void add(const std::wstring &input, const std::wstring &output,
           bool keepOrig);

  void assertEquals(std::shared_ptr<CharTermAttribute> term,
                    const std::wstring &expected);

  // For the output string: separate positions with a space,
  // and separate multiple tokens at each position with a
  // /.  If a token should have end offset != the input
  // token's end offset then add :X to it:

  // TODO: we should probably refactor this guy to use/take analyzer,
  // the tests are a little messy
  void verify(const std::wstring &input,
              const std::wstring &output) ;

public:
  virtual void testDontKeepOrig() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestSynonymMapFilter> outerInstance;

    std::shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestSynonymMapFilter> outerInstance,
        std::shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap>
            map_obj);

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
  virtual void testDoKeepOrig() ;

private:
  class AnalyzerAnonymousInnerClass2 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass2)
  private:
    std::shared_ptr<TestSynonymMapFilter> outerInstance;

    std::shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj;

  public:
    AnalyzerAnonymousInnerClass2(
        std::shared_ptr<TestSynonymMapFilter> outerInstance,
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

public:
  virtual void testBasic() ;

private:
  std::wstring getRandomString(wchar_t start, int alphabetSize, int length);

private:
  class OneSyn : public std::enable_shared_from_this<OneSyn>
  {
    GET_CLASS_NAME(OneSyn)
  public:
    std::wstring in_;
    std::deque<std::wstring> out;
    bool keepOrig = false;
  };

public:
  virtual std::wstring
  slowSynMatcher(const std::wstring &doc,
                 std::deque<std::shared_ptr<OneSyn>> &syns,
                 int maxOutputLength);

  virtual void testRandom() ;

private:
  void pruneDups(std::deque<std::shared_ptr<OneSyn>> &syns);

  std::wstring randomNonEmptyString();

  /** simple random test, doesn't verify correctness.
   *  does verify it doesnt throw exceptions, or that the stream doesn't
   * misbehave
   */
public:
  virtual void testRandom2() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestSynonymMapFilter> outerInstance;

    std::shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj;
    bool ignoreCase = false;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestSynonymMapFilter> outerInstance,
        std::shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj,
        bool ignoreCase);

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

  // NOTE: this is an invalid test... SynFilter today can't
  // properly consume a graph... we can re-enable this once
  // we fix that...
  /*
  // Adds MockGraphTokenFilter before SynFilter:
  public void testRandom2GraphBefore() throws Exception {
    final int numIters = atLeast(10);
    Random random = random();
    for (int i = 0; i < numIters; i++) {
      b = new SynonymMap.Builder(random.nextBoolean());
      final int numEntries = atLeast(10);
      for (int j = 0; j < numEntries; j++) {
        add(randomNonEmptyString(), randomNonEmptyString(),
  random.nextBoolean());
      }
      final SynonymMap map_obj = b.build();
      final bool ignoreCase = random.nextBoolean();

      final Analyzer analyzer = new Analyzer() {
        @Override
        protected TokenStreamComponents createComponents(std::wstring fieldName) {
          Tokenizer tokenizer = new MockTokenizer(reader, MockTokenizer.SIMPLE,
  true); TokenStream graph = new MockGraphTokenFilter(random(), tokenizer);
          return new TokenStreamComponents(tokenizer, new SynonymFilter(graph,
  map_obj, ignoreCase));
        }
      };

      checkRandomData(random, analyzer, 1000*RANDOM_MULTIPLIER);
    }
  }
  */

  // Adds MockGraphTokenFilter after SynFilter:
public:
  virtual void testRandom2GraphAfter() ;

private:
  class AnalyzerAnonymousInnerClass2 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass2)
  private:
    std::shared_ptr<TestSynonymMapFilter> outerInstance;

    std::shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj;
    bool ignoreCase = false;

  public:
    AnalyzerAnonymousInnerClass2(
        std::shared_ptr<TestSynonymMapFilter> outerInstance,
        std::shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj,
        bool ignoreCase);

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

public:
  virtual void testEmptyTerm() ;

private:
  class AnalyzerAnonymousInnerClass3 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass3)
  private:
    std::shared_ptr<TestSynonymMapFilter> outerInstance;

    std::shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj;
    bool ignoreCase = false;

  public:
    AnalyzerAnonymousInnerClass3(
        std::shared_ptr<TestSynonymMapFilter> outerInstance,
        std::shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj,
        bool ignoreCase);

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

  /** simple random test like testRandom2, but for larger docs
   */
public:
  virtual void testRandomHuge() ;

private:
  class AnalyzerAnonymousInnerClass4 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass4)
  private:
    std::shared_ptr<TestSynonymMapFilter> outerInstance;

    std::shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj;
    bool ignoreCase = false;

  public:
    AnalyzerAnonymousInnerClass4(
        std::shared_ptr<TestSynonymMapFilter> outerInstance,
        std::shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj,
        bool ignoreCase);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass4> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass4>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

  // LUCENE-3375
public:
  virtual void testVanishingTerms() ;

private:
  class AnalyzerAnonymousInnerClass5 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass5)
  private:
    std::shared_ptr<TestSynonymMapFilter> outerInstance;

    std::shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj;

  public:
    AnalyzerAnonymousInnerClass5(
        std::shared_ptr<TestSynonymMapFilter> outerInstance,
        std::shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap>
            map_obj);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass5> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass5>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  virtual void testBasic2() ;

  virtual void testMatching() ;

private:
  class AnalyzerAnonymousInnerClass6 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass6)
  private:
    std::shared_ptr<TestSynonymMapFilter> outerInstance;

    std::shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj;

  public:
    AnalyzerAnonymousInnerClass6(
        std::shared_ptr<TestSynonymMapFilter> outerInstance,
        std::shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap>
            map_obj);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass6> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass6>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  virtual void testRepeatsOff() ;

private:
  class AnalyzerAnonymousInnerClass7 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass7)
  private:
    std::shared_ptr<TestSynonymMapFilter> outerInstance;

    std::shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj;

  public:
    AnalyzerAnonymousInnerClass7(
        std::shared_ptr<TestSynonymMapFilter> outerInstance,
        std::shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap>
            map_obj);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass7> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass7>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  virtual void testRepeatsOn() ;

private:
  class AnalyzerAnonymousInnerClass8 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass8)
  private:
    std::shared_ptr<TestSynonymMapFilter> outerInstance;

    std::shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj;

  public:
    AnalyzerAnonymousInnerClass8(
        std::shared_ptr<TestSynonymMapFilter> outerInstance,
        std::shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap>
            map_obj);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass8> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass8>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  virtual void testRecursion() ;

private:
  class AnalyzerAnonymousInnerClass9 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass9)
  private:
    std::shared_ptr<TestSynonymMapFilter> outerInstance;

    std::shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj;

  public:
    AnalyzerAnonymousInnerClass9(
        std::shared_ptr<TestSynonymMapFilter> outerInstance,
        std::shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap>
            map_obj);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass9> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass9>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  virtual void testRecursion2() ;

private:
  class AnalyzerAnonymousInnerClass10 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass10)
  private:
    std::shared_ptr<TestSynonymMapFilter> outerInstance;

    std::shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj;

  public:
    AnalyzerAnonymousInnerClass10(
        std::shared_ptr<TestSynonymMapFilter> outerInstance,
        std::shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap>
            map_obj);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass10> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass10>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  virtual void testOutputHangsOffEnd() ;

  virtual void testIncludeOrig() ;

private:
  class AnalyzerAnonymousInnerClass11 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass11)
  private:
    std::shared_ptr<TestSynonymMapFilter> outerInstance;

    std::shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj;

  public:
    AnalyzerAnonymousInnerClass11(
        std::shared_ptr<TestSynonymMapFilter> outerInstance,
        std::shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap>
            map_obj);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass11> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass11>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  virtual void testRecursion3() ;

private:
  class AnalyzerAnonymousInnerClass12 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass12)
  private:
    std::shared_ptr<TestSynonymMapFilter> outerInstance;

    std::shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj;

  public:
    AnalyzerAnonymousInnerClass12(
        std::shared_ptr<TestSynonymMapFilter> outerInstance,
        std::shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap>
            map_obj);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass12> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass12>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  virtual void testRecursion4() ;

private:
  class AnalyzerAnonymousInnerClass13 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass13)
  private:
    std::shared_ptr<TestSynonymMapFilter> outerInstance;

    std::shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj;

  public:
    AnalyzerAnonymousInnerClass13(
        std::shared_ptr<TestSynonymMapFilter> outerInstance,
        std::shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap>
            map_obj);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass13> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass13>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  virtual void testMultiwordOffsets() ;

private:
  class AnalyzerAnonymousInnerClass14 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass14)
  private:
    std::shared_ptr<TestSynonymMapFilter> outerInstance;

    std::shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj;

  public:
    AnalyzerAnonymousInnerClass14(
        std::shared_ptr<TestSynonymMapFilter> outerInstance,
        std::shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap>
            map_obj);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass14> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass14>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  virtual void testEmpty() ;

protected:
  std::shared_ptr<TestSynonymMapFilter> shared_from_this()
  {
    return std::static_pointer_cast<TestSynonymMapFilter>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::synonym
