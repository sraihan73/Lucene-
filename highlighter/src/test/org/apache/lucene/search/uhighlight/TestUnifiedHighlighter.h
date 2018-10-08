#pragma once
#include "stringhelper.h"
#include <any>
#include <functional>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::document
{
class FieldType;
}

namespace org::apache::lucene::analysis
{
class MockAnalyzer;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::analysis
{
class Analyzer;
}
namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::search::uhighlight
{
class UnifiedHighlighter;
}
namespace org::apache::lucene::search
{
class DocIdSetIterator;
}
namespace org::apache::lucene::search::uhighlight
{
class PassageFormatter;
}
namespace org::apache::lucene::search::uhighlight
{
class Passage;
}
namespace org::apache::lucene::index
{
class IndexReader;
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
namespace org::apache::lucene::search::uhighlight
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using FieldType = org::apache::lucene::document::FieldType;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using HighlightFlag =
    org::apache::lucene::search::uhighlight::UnifiedHighlighter::HighlightFlag;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestUnifiedHighlighter : public LuceneTestCase
{
  GET_CLASS_NAME(TestUnifiedHighlighter)

private:
  const std::shared_ptr<FieldType>
      fieldType; // for "body" generally, but not necessarily others. See
                 // constructor

  std::shared_ptr<MockAnalyzer> indexAnalyzer;
  std::shared_ptr<Directory> dir;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @ParametersFactory public static Iterable<Object[]>
  // parameters()
  static std::deque<std::deque<std::any>> parameters();

  TestUnifiedHighlighter(std::shared_ptr<FieldType> fieldType);

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Before public void doBefore() throws java.io.IOException
  virtual void doBefore() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @After public void doAfter() throws java.io.IOException
  virtual void doAfter() ;

  static std::shared_ptr<UnifiedHighlighter>
  randomUnifiedHighlighter(std::shared_ptr<IndexSearcher> searcher,
                           std::shared_ptr<Analyzer> indexAnalyzer);

  static std::shared_ptr<UnifiedHighlighter> randomUnifiedHighlighter(
      std::shared_ptr<IndexSearcher> searcher,
      std::shared_ptr<Analyzer> indexAnalyzer,
      std::shared_ptr<EnumSet<HighlightFlag>> mandatoryFlags);

private:
  class UnifiedHighlighterAnonymousInnerClass : public UnifiedHighlighter
  {
    GET_CLASS_NAME(UnifiedHighlighterAnonymousInnerClass)
  private:
    std::shared_ptr<EnumSet<HighlightFlag>> mandatoryFlags;

  public:
    UnifiedHighlighterAnonymousInnerClass(
        std::shared_ptr<IndexSearcher> searcher,
        std::shared_ptr<Analyzer> indexAnalyzer,
        std::shared_ptr<EnumSet<HighlightFlag>> mandatoryFlags);

  protected:
    std::shared_ptr<Set<HighlightFlag>>
    getFlags(const std::wstring &field) override;

  protected:
    std::shared_ptr<UnifiedHighlighterAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<UnifiedHighlighterAnonymousInnerClass>(
          UnifiedHighlighter::shared_from_this());
    }
  };

  //
  //  Tests below were ported from the PostingsHighlighter. Possibly augmented.
  //  Far below are newer tests.
  //

public:
  virtual void testBasics() ;

  virtual void
  testFormatWithMatchExceedingContentLength2() ;

  virtual void
  testFormatWithMatchExceedingContentLength3() ;

  virtual void
  testFormatWithMatchExceedingContentLength() ;

private:
  std::deque<std::wstring> formatWithMatchExceedingContentLength(
      const std::wstring &bodyText) ;

  // simple test highlighting last word.
public:
  virtual void testHighlightLastWord() ;

  // simple test with one sentence documents.
  virtual void testOneSentence() ;

  // simple test with multiple values that make a result longer than maxLength.
  virtual void testMaxLengthWithMultivalue() ;

  virtual void testMultipleFields() ;

  virtual void testMultipleTerms() ;

  virtual void testMultiplePassages() ;

  virtual void testBuddhism() ;

  virtual void testCuriousGeorge() ;

  virtual void testCambridgeMA() ;

  virtual void testPassageRanking() ;

  virtual void testBooleanMustNot() ;

  virtual void testHighlightAllText() ;

private:
  class UnifiedHighlighterAnonymousInnerClass2 : public UnifiedHighlighter
  {
    GET_CLASS_NAME(UnifiedHighlighterAnonymousInnerClass2)
  private:
    std::shared_ptr<TestUnifiedHighlighter> outerInstance;

  public:
    UnifiedHighlighterAnonymousInnerClass2(
        std::shared_ptr<TestUnifiedHighlighter> outerInstance,
        std::shared_ptr<IndexSearcher> searcher,
        std::shared_ptr<MockAnalyzer> indexAnalyzer);

  protected:
    std::shared_ptr<BreakIterator>
    getBreakIterator(const std::wstring &field) override;

  protected:
    std::shared_ptr<UnifiedHighlighterAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<UnifiedHighlighterAnonymousInnerClass2>(
          UnifiedHighlighter::shared_from_this());
    }
  };

public:
  virtual void testSpecificDocIDs() ;

  virtual void testCustomFieldValueSource() ;

private:
  class UnifiedHighlighterAnonymousInnerClass3 : public UnifiedHighlighter
  {
    GET_CLASS_NAME(UnifiedHighlighterAnonymousInnerClass3)
  private:
    std::shared_ptr<TestUnifiedHighlighter> outerInstance;

    std::wstring text;

  public:
    UnifiedHighlighterAnonymousInnerClass3(
        std::shared_ptr<TestUnifiedHighlighter> outerInstance,
        std::shared_ptr<IndexSearcher> searcher,
        std::shared_ptr<MockAnalyzer> indexAnalyzer, const std::wstring &text);

  protected:
    std::deque<std::deque<std::shared_ptr<std::wstring>>>
    loadFieldValues(std::deque<std::wstring> &fields,
                    std::shared_ptr<DocIdSetIterator> docIter,
                    int cacheCharsThreshold)  override;

    std::shared_ptr<BreakIterator>
    getBreakIterator(const std::wstring &field) override;

  protected:
    std::shared_ptr<UnifiedHighlighterAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<UnifiedHighlighterAnonymousInnerClass3>(
          UnifiedHighlighter::shared_from_this());
    }
  };

  /**
   * Make sure highlighter returns first N sentences if
   * there were no hits.
   */
public:
  virtual void testEmptyHighlights() ;

  /**
   * Not empty but nothing analyzes. Ensures we address null term-vectors.
   */
  virtual void testNothingAnalyzes() ;

  /**
   * Make sure highlighter we can customize how emtpy
   * highlight is returned.
   */
  virtual void testCustomEmptyHighlights() ;

  /**
   * Make sure highlighter returns whole text when there
   * are no hits and BreakIterator is null.
   */
  virtual void testEmptyHighlightsWhole() ;

private:
  class UnifiedHighlighterAnonymousInnerClass4 : public UnifiedHighlighter
  {
    GET_CLASS_NAME(UnifiedHighlighterAnonymousInnerClass4)
  private:
    std::shared_ptr<TestUnifiedHighlighter> outerInstance;

  public:
    UnifiedHighlighterAnonymousInnerClass4(
        std::shared_ptr<TestUnifiedHighlighter> outerInstance,
        std::shared_ptr<IndexSearcher> searcher,
        std::shared_ptr<MockAnalyzer> indexAnalyzer);

  protected:
    std::shared_ptr<BreakIterator>
    getBreakIterator(const std::wstring &field) override;

  protected:
    std::shared_ptr<UnifiedHighlighterAnonymousInnerClass4> shared_from_this()
    {
      return std::static_pointer_cast<UnifiedHighlighterAnonymousInnerClass4>(
          UnifiedHighlighter::shared_from_this());
    }
  };

  /**
   * Make sure highlighter is OK with entirely missing
   * field.
   */
public:
  virtual void testFieldIsMissing() ;

  virtual void testFieldIsJustSpace() ;

  virtual void testFieldIsEmptyString() ;

  virtual void testMultipleDocs() ;

  virtual void testMultipleSnippetSizes() ;

  virtual void testEncode() ;

private:
  class UnifiedHighlighterAnonymousInnerClass5 : public UnifiedHighlighter
  {
    GET_CLASS_NAME(UnifiedHighlighterAnonymousInnerClass5)
  private:
    std::shared_ptr<TestUnifiedHighlighter> outerInstance;

  public:
    UnifiedHighlighterAnonymousInnerClass5(
        std::shared_ptr<TestUnifiedHighlighter> outerInstance,
        std::shared_ptr<IndexSearcher> searcher,
        std::shared_ptr<MockAnalyzer> indexAnalyzer);

  protected:
    std::shared_ptr<PassageFormatter>
    getFormatter(const std::wstring &field) override;

  protected:
    std::shared_ptr<UnifiedHighlighterAnonymousInnerClass5> shared_from_this()
    {
      return std::static_pointer_cast<UnifiedHighlighterAnonymousInnerClass5>(
          UnifiedHighlighter::shared_from_this());
    }
  };

  // LUCENE-4906
public:
  virtual void testObjectFormatter() ;

private:
  class UnifiedHighlighterAnonymousInnerClass6 : public UnifiedHighlighter
  {
    GET_CLASS_NAME(UnifiedHighlighterAnonymousInnerClass6)
  private:
    std::shared_ptr<TestUnifiedHighlighter> outerInstance;

  public:
    UnifiedHighlighterAnonymousInnerClass6(
        std::shared_ptr<TestUnifiedHighlighter> outerInstance,
        std::shared_ptr<IndexSearcher> searcher,
        std::shared_ptr<MockAnalyzer> indexAnalyzer);

  protected:
    std::shared_ptr<PassageFormatter>
    getFormatter(const std::wstring &field) override;

  private:
    class PassageFormatterAnonymousInnerClass : public PassageFormatter
    {
      GET_CLASS_NAME(PassageFormatterAnonymousInnerClass)
    private:
      std::shared_ptr<UnifiedHighlighterAnonymousInnerClass6> outerInstance;

    public:
      PassageFormatterAnonymousInnerClass(
          std::shared_ptr<UnifiedHighlighterAnonymousInnerClass6>
              outerInstance);

      std::shared_ptr<PassageFormatter> defaultFormatter;

      std::deque<std::wstring>
      format(std::deque<std::shared_ptr<Passage>> &passages,
             const std::wstring &content) override;

    protected:
      std::shared_ptr<PassageFormatterAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<PassageFormatterAnonymousInnerClass>(
            PassageFormatter::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<UnifiedHighlighterAnonymousInnerClass6> shared_from_this()
    {
      return std::static_pointer_cast<UnifiedHighlighterAnonymousInnerClass6>(
          UnifiedHighlighter::shared_from_this());
    }
  };

private:
  std::shared_ptr<IndexReader> indexSomeFields() ;

public:
  virtual void testFieldMatcherTermQuery() ;

private:
  class UnifiedHighlighterAnonymousInnerClass7 : public UnifiedHighlighter
  {
    GET_CLASS_NAME(UnifiedHighlighterAnonymousInnerClass7)
  private:
    std::shared_ptr<TestUnifiedHighlighter> outerInstance;

  public:
    UnifiedHighlighterAnonymousInnerClass7(
        std::shared_ptr<TestUnifiedHighlighter> outerInstance,
        std::shared_ptr<IndexSearcher> searcher,
        std::shared_ptr<MockAnalyzer> indexAnalyzer);

    std::shared_ptr<protected> std::function <
        bool(std::shared_ptr<const> std) override;

  protected:
    std::shared_ptr<UnifiedHighlighterAnonymousInnerClass7> shared_from_this()
    {
      return std::static_pointer_cast<UnifiedHighlighterAnonymousInnerClass7>(
          UnifiedHighlighter::shared_from_this());
    }
  };

public:
  virtual void testFieldMatcherMultiTermQuery() ;

private:
  class UnifiedHighlighterAnonymousInnerClass8 : public UnifiedHighlighter
  {
    GET_CLASS_NAME(UnifiedHighlighterAnonymousInnerClass8)
  private:
    std::shared_ptr<TestUnifiedHighlighter> outerInstance;

  public:
    UnifiedHighlighterAnonymousInnerClass8(
        std::shared_ptr<TestUnifiedHighlighter> outerInstance,
        std::shared_ptr<IndexSearcher> searcher,
        std::shared_ptr<MockAnalyzer> indexAnalyzer);

    std::shared_ptr<protected> std::function <
        bool(std::shared_ptr<const> std) override;

  protected:
    std::shared_ptr<UnifiedHighlighterAnonymousInnerClass8> shared_from_this()
    {
      return std::static_pointer_cast<UnifiedHighlighterAnonymousInnerClass8>(
          UnifiedHighlighter::shared_from_this());
    }
  };

public:
  virtual void testFieldMatcherPhraseQuery() ;

private:
  class UnifiedHighlighterAnonymousInnerClass9 : public UnifiedHighlighter
  {
    GET_CLASS_NAME(UnifiedHighlighterAnonymousInnerClass9)
  private:
    std::shared_ptr<TestUnifiedHighlighter> outerInstance;

  public:
    UnifiedHighlighterAnonymousInnerClass9(
        std::shared_ptr<TestUnifiedHighlighter> outerInstance,
        std::shared_ptr<IndexSearcher> searcher,
        std::shared_ptr<MockAnalyzer> indexAnalyzer);

    std::shared_ptr<protected> std::function <
        bool(std::shared_ptr<const> std) override;

  protected:
    std::shared_ptr<UnifiedHighlighterAnonymousInnerClass9> shared_from_this()
    {
      return std::static_pointer_cast<UnifiedHighlighterAnonymousInnerClass9>(
          UnifiedHighlighter::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestUnifiedHighlighter> shared_from_this()
  {
    return std::static_pointer_cast<TestUnifiedHighlighter>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::uhighlight
