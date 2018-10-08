#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <algorithm>
#include <any>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class Analyzer;
}

namespace org::apache::lucene::search::suggest::analyzing
{
class AnalyzingInfixSuggester;
}
namespace org::apache::lucene::search::suggest
{
class Input;
}
namespace org::apache::lucene::store
{
class BaseDirectoryWrapper;
}
namespace org::apache::lucene::analysis
{
class CharArraySet;
}
namespace org::apache::lucene::analysis
{
class TokenStreamComponents;
}
namespace org::apache::lucene::util
{
class BytesRef;
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
namespace org::apache::lucene::search::suggest::analyzing
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using Input = org::apache::lucene::search::suggest::Input;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class AnalyzingInfixSuggesterTest : public LuceneTestCase
{
  GET_CLASS_NAME(AnalyzingInfixSuggesterTest)

public:
  virtual void testBasic() ;

private:
  void
  testConstructorDefaults(std::shared_ptr<AnalyzingInfixSuggester> suggester,
                          std::deque<std::shared_ptr<Input>> &keys,
                          std::shared_ptr<Analyzer> a, bool allTermsRequired,
                          bool highlight) ;

public:
  virtual void testAfterLoad() ;

  /** Used to return highlighted result; see {@link
   *  LookupResult#highlightKey} */
private:
  class LookupHighlightFragment final
      : public std::enable_shared_from_this<LookupHighlightFragment>
  {
    GET_CLASS_NAME(LookupHighlightFragment)
    /** Portion of text for this fragment. */
  public:
    const std::wstring text;

    /** True if this text matched a part of the user's
     *  query. */
    const bool isHit;

    /** Sole constructor. */
    LookupHighlightFragment(const std::wstring &text, bool isHit);

    virtual std::wstring toString();
  };

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("unchecked") public void
  // testHighlightAsObject() throws Exception
  virtual void testHighlightAsObject() ;

private:
  class AnalyzingInfixSuggesterAnonymousInnerClass
      : public AnalyzingInfixSuggester
  {
    GET_CLASS_NAME(AnalyzingInfixSuggesterAnonymousInnerClass)
  private:
    std::shared_ptr<AnalyzingInfixSuggesterTest> outerInstance;

  public:
    AnalyzingInfixSuggesterAnonymousInnerClass(
        std::shared_ptr<AnalyzingInfixSuggesterTest> outerInstance,
        std::shared_ptr<org::apache::lucene::store::BaseDirectoryWrapper>
            newDirectory,
        std::shared_ptr<Analyzer> a, std::shared_ptr<Analyzer> a);

  protected:
    std::any
    highlight(const std::wstring &text,
              std::shared_ptr<Set<std::wstring>> matchedTokens,
              const std::wstring &prefixToken)  override;

  protected:
    std::shared_ptr<AnalyzingInfixSuggesterAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          AnalyzingInfixSuggesterAnonymousInnerClass>(
          AnalyzingInfixSuggester::shared_from_this());
    }
  };

public:
  virtual std::wstring
  toString(std::deque<std::shared_ptr<LookupHighlightFragment>> &fragments);

  virtual void testRandomMinPrefixLength() ;

  virtual void testHighlight() ;

  virtual void testHighlightCaseChange() ;

private:
  class AnalyzingInfixSuggesterAnonymousInnerClass2
      : public AnalyzingInfixSuggester
  {
    GET_CLASS_NAME(AnalyzingInfixSuggesterAnonymousInnerClass2)
  private:
    std::shared_ptr<AnalyzingInfixSuggesterTest> outerInstance;

  public:
    AnalyzingInfixSuggesterAnonymousInnerClass2(
        std::shared_ptr<AnalyzingInfixSuggesterTest> outerInstance,
        std::shared_ptr<org::apache::lucene::store::BaseDirectoryWrapper>
            newDirectory,
        std::shared_ptr<Analyzer> a, std::shared_ptr<Analyzer> a);

  protected:
    void addPrefixMatch(std::shared_ptr<StringBuilder> sb,
                        const std::wstring &surface,
                        const std::wstring &analyzed,
                        const std::wstring &prefixToken) override;

  protected:
    std::shared_ptr<AnalyzingInfixSuggesterAnonymousInnerClass2>
    shared_from_this()
    {
      return std::static_pointer_cast<
          AnalyzingInfixSuggesterAnonymousInnerClass2>(
          AnalyzingInfixSuggester::shared_from_this());
    }
  };

public:
  virtual void testDoubleClose() ;

  virtual void testSuggestStopFilter() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<AnalyzingInfixSuggesterTest> outerInstance;

    std::shared_ptr<CharArraySet> stopWords;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<AnalyzingInfixSuggesterTest> outerInstance,
        std::shared_ptr<CharArraySet> stopWords);

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

private:
  class AnalyzerAnonymousInnerClass2 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass2)
  private:
    std::shared_ptr<AnalyzingInfixSuggesterTest> outerInstance;

    std::shared_ptr<CharArraySet> stopWords;

  public:
    AnalyzerAnonymousInnerClass2(
        std::shared_ptr<AnalyzingInfixSuggesterTest> outerInstance,
        std::shared_ptr<CharArraySet> stopWords);

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
  virtual void testEmptyAtStart() ;

  virtual void testBothExactAndPrefix() ;

private:
  static std::wstring randomText();

private:
  class Update : public std::enable_shared_from_this<Update>
  {
    GET_CLASS_NAME(Update)
  public:
    int64_t weight = 0;
    int index = 0;
  };

private:
  class LookupThread : public Thread
  {
    GET_CLASS_NAME(LookupThread)
  private:
    const std::shared_ptr<AnalyzingInfixSuggester> suggester;
    // C++ TODO: 'volatile' has a different meaning in C++:
    // ORIGINAL LINE: private volatile bool stop;
    bool stop = false;

  public:
    LookupThread(std::shared_ptr<AnalyzingInfixSuggester> suggester);

    virtual void finish() ;

    void run() override;

  protected:
    std::shared_ptr<LookupThread> shared_from_this()
    {
      return std::static_pointer_cast<LookupThread>(Thread::shared_from_this());
    }
  };

public:
  virtual void testRandomNRT() ;

private:
  static std::wstring hilite(bool lastPrefix,
                             std::deque<std::wstring> &inputTerms,
                             std::deque<std::wstring> &queryTerms);

public:
  virtual void testBasicNRT() ;

  virtual void testNRTWithParallelAdds() throw(IOException,
                                               InterruptedException);

private:
  class IndexDocument : public std::enable_shared_from_this<IndexDocument>,
                        public Runnable
  {
    GET_CLASS_NAME(IndexDocument)
  public:
    std::shared_ptr<AnalyzingInfixSuggester> suggester;
    std::wstring key;

  private:
    IndexDocument(std::shared_ptr<AnalyzingInfixSuggester> suggester,
                  const std::wstring &key);

  public:
    void run() override;
  };

private:
  std::shared_ptr<Set<std::shared_ptr<BytesRef>>>
  asSet(std::deque<std::wstring> &values);

  std::shared_ptr<Set<std::shared_ptr<BytesRef>>>
  asSet(std::deque<char> &values);

  // LUCENE-5528 and LUCENE-6464
public:
  virtual void testBasicContext() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testAddPrefixMatch() throws
  // java.io.IOException
  virtual void testAddPrefixMatch() ;

private:
  std::wstring pfmToString(std::shared_ptr<AnalyzingInfixSuggester> suggester,
                           const std::wstring &surface,
                           const std::wstring &prefix) ;

public:
  virtual void testBinaryContext() ;

  virtual void testContextNotAllTermsRequired() ;

  virtual void testCloseIndexWriterOnBuild() ;

  virtual void testCommitAfterBuild() ;

  virtual void testRefreshAfterBuild() ;

  virtual void testDisallowCommitBeforeBuild() ;

  virtual void testDisallowRefreshBeforeBuild() ;

private:
  std::deque<std::shared_ptr<Input>> sharedInputs =
      std::deque<std::shared_ptr<Input>>{
          std::make_shared<Input>(L"lend me your ear", 8,
                                  std::make_shared<BytesRef>(L"foobar")),
          std::make_shared<Input>(L"a penny saved is a penny earned", 10,
                                  std::make_shared<BytesRef>(L"foobaz"))};

private:
  class SuggesterOperation
  {
    GET_CLASS_NAME(SuggesterOperation)
  public:
    virtual void
    operate(std::shared_ptr<AnalyzingInfixSuggester> suggester) = 0;
  };

  /**
   * Perform the given operation on suggesters constructed with all combinations
   * of options commitOnBuild and closeIndexWriterOnBuild, including defaults.
   */
private:
  void performOperationWithAllOptionCombinations(
      std::shared_ptr<SuggesterOperation> operation) ;

protected:
  std::shared_ptr<AnalyzingInfixSuggesterTest> shared_from_this()
  {
    return std::static_pointer_cast<AnalyzingInfixSuggesterTest>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::suggest::analyzing
