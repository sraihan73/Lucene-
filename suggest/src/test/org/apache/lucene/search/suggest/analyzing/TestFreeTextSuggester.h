#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <algorithm>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class LineFileDocs;
}

namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::analysis
{
class Analyzer;
}
namespace org::apache::lucene::analysis
{
class TokenStreamComponents;
}
namespace org::apache::lucene::search::suggest
{
class Lookup;
}
namespace org::apache::lucene::search::suggest
{
class LookupResult;
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

using LookupResult = org::apache::lucene::search::suggest::Lookup::LookupResult;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestFreeTextSuggester : public LuceneTestCase
{
  GET_CLASS_NAME(TestFreeTextSuggester)

public:
  virtual void testBasic() ;

  virtual void testIllegalByteDuringBuild() ;

  virtual void testIllegalByteDuringQuery() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Ignore public void testWiki() throws Exception
  virtual void testWiki() ;

private:
  class InputIteratorAnonymousInnerClass
      : public std::enable_shared_from_this<InputIteratorAnonymousInnerClass>,
        public InputIterator
  {
    GET_CLASS_NAME(InputIteratorAnonymousInnerClass)
  private:
    std::shared_ptr<TestFreeTextSuggester> outerInstance;

    std::shared_ptr<LineFileDocs> lfd;

  public:
    InputIteratorAnonymousInnerClass(
        std::shared_ptr<TestFreeTextSuggester> outerInstance,
        std::shared_ptr<LineFileDocs> lfd);

  private:
    int count = 0;

  public:
    int64_t weight() override;

    std::shared_ptr<BytesRef> next();

    std::shared_ptr<BytesRef> payload() override;

    bool hasPayloads() override;

    std::shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts() override;

    bool hasContexts() override;
  };

  // Make sure you can suggest based only on unigram model:
public:
  virtual void testUnigrams() ;

  // Make sure the last token is not duplicated
  virtual void testNoDupsAcrossGrams() ;

  // Lookup of just empty string produces unicode only matches:
  virtual void testEmptyString() ;

  // With one ending hole, ShingleFilter produces "of _" and
  // we should properly predict from that:
  virtual void testEndingHole() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestFreeTextSuggester> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestFreeTextSuggester> outerInstance);

    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &field) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

  // If the number of ending holes exceeds the ngrams window
  // then there are no predictions, because ShingleFilter
  // does not produce e.g. a hole only "_ _" token:
public:
  virtual void testTwoEndingHoles() ;

private:
  class AnalyzerAnonymousInnerClass2 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass2)
  private:
    std::shared_ptr<TestFreeTextSuggester> outerInstance;

  public:
    AnalyzerAnonymousInnerClass2(
        std::shared_ptr<TestFreeTextSuggester> outerInstance);

    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &field) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass2>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

private:
  static std::shared_ptr<Comparator<std::shared_ptr<LookupResult>>>
      byScoreThenKey;

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<std::shared_ptr<LookupResult>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  public:
    ComparatorAnonymousInnerClass();

    int compare(std::shared_ptr<LookupResult> a,
                std::shared_ptr<LookupResult> b) override;
  };

public:
  virtual void testRandom() ;

private:
  class InputIteratorAnonymousInnerClass2
      : public std::enable_shared_from_this<InputIteratorAnonymousInnerClass2>,
        public InputIterator
  {
    GET_CLASS_NAME(InputIteratorAnonymousInnerClass2)
  private:
    std::shared_ptr<TestFreeTextSuggester> outerInstance;

    std::deque<std::deque<std::wstring>> docs;

  public:
    InputIteratorAnonymousInnerClass2(
        std::shared_ptr<TestFreeTextSuggester> outerInstance,
        std::deque<std::deque<std::wstring>> &docs);

    int upto = 0;

    std::shared_ptr<BytesRef> next();

    int64_t weight() override;

    std::shared_ptr<BytesRef> payload() override;

    bool hasPayloads() override;

    std::shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts() override;

    bool hasContexts() override;
  };

private:
  static std::wstring getZipfToken(std::deque<std::wstring> &tokens);

  static std::wstring
  toString(std::deque<std::shared_ptr<LookupResult>> &results);

protected:
  std::shared_ptr<TestFreeTextSuggester> shared_from_this()
  {
    return std::static_pointer_cast<TestFreeTextSuggester>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::suggest::analyzing
