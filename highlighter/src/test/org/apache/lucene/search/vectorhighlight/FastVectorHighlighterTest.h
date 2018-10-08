#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <map_obj>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/Query.h"

#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include  "core/src/java/org/apache/lucene/analysis/Token.h"

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
namespace org::apache::lucene::search::vectorhighlight
{

using Token = org::apache::lucene::analysis::Token;
using Query = org::apache::lucene::search::Query;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class FastVectorHighlighterTest : public LuceneTestCase
{
  GET_CLASS_NAME(FastVectorHighlighterTest)

public:
  virtual void testSimpleHighlightTest() ;

  virtual void testCustomScoreQueryHighlight() ;

  virtual void testFunctionScoreQueryHighlight() ;

  virtual void testPhraseHighlightLongTextTest() ;

  // see LUCENE-4899
  virtual void testPhraseHighlightTest() ;

  virtual void testBoostedPhraseHighlightTest() ;

  virtual void testCommonTermsQueryHighlight() ;

  virtual void testMatchedFields() ;

  virtual void testMultiValuedSortByScore() ;

  virtual void testWithSynonym() ;

  virtual void testBooleanPhraseWithSynonym() ;

  virtual void testPhrasesSpanningFieldValues() ;

private:
  void
  matchedFieldsTestCase(const std::wstring &fieldValue,
                        const std::wstring &expected,
                        std::deque<Query> &queryClauses) ;

  void
  matchedFieldsTestCase(bool useMatchedFields, bool fieldMatch,
                        const std::wstring &fieldValue,
                        const std::wstring &expected,
                        std::deque<Query> &queryClauses) ;

private:
  class DelegatingAnalyzerWrapperAnonymousInnerClass
      : public DelegatingAnalyzerWrapper
  {
    GET_CLASS_NAME(DelegatingAnalyzerWrapperAnonymousInnerClass)
  private:
    std::shared_ptr<FastVectorHighlighterTest> outerInstance;

    std::unordered_map<std::wstring, std::shared_ptr<Analyzer>> fieldAnalyzers;

  public:
    DelegatingAnalyzerWrapperAnonymousInnerClass(
        std::shared_ptr<FastVectorHighlighterTest> outerInstance,
        std::shared_ptr<UnknownType> PER_FIELD_REUSE_STRATEGY,
        std::unordered_map<std::wstring, std::shared_ptr<Analyzer>>
            &fieldAnalyzers);

    std::shared_ptr<Analyzer>
    getWrappedAnalyzer(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<DelegatingAnalyzerWrapperAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          DelegatingAnalyzerWrapperAnonymousInnerClass>(
          org.apache.lucene.analysis
              .DelegatingAnalyzerWrapper::shared_from_this());
    }
  };

private:
  std::shared_ptr<Query> clause(const std::wstring &field,
                                std::deque<std::wstring> &terms);

  std::shared_ptr<Query> clause(const std::wstring &field, float boost,
                                std::deque<std::wstring> &terms);

  static std::shared_ptr<Token> token(const std::wstring &term, int posInc,
                                      int startOffset, int endOffset);

protected:
  std::shared_ptr<FastVectorHighlighterTest> shared_from_this()
  {
    return std::static_pointer_cast<FastVectorHighlighterTest>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/vectorhighlight/
