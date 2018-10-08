#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <cctype>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"

#include  "core/src/java/org/apache/lucene/document/FieldType.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/search/uhighlight/FakePassageFormatter.h"
#include  "core/src/java/org/apache/lucene/search/uhighlight/PassageFormatter.h"
#include  "core/src/java/org/apache/lucene/search/uhighlight/Pair.h"
#include  "core/src/java/org/apache/lucene/search/uhighlight/Passage.h"
#include  "core/src/java/org/apache/lucene/search/uhighlight/PassageScorer.h"

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
using FieldType = org::apache::lucene::document::FieldType;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestUnifiedHighlighterRanking : public LuceneTestCase
{
  GET_CLASS_NAME(TestUnifiedHighlighterRanking)

public:
  std::shared_ptr<Analyzer> indexAnalyzer;

  // note: all offset sources, by default, use term freq, so it shouldn't matter
  // which we choose.
  const std::shared_ptr<FieldType> fieldType =
      UHTestHelper::randomFieldType(random());

  /**
   * indexes a bunch of gibberish, and then highlights top(n).
   * asserts that top(n) highlights is a subset of top(n+1) up to some max N
   */
  // TODO: this only tests single-valued fields. we should also index multiple
  // values per field!
  virtual void testRanking() ;

private:
  void checkDocument(std::shared_ptr<IndexSearcher> is, int doc,
                     int maxTopN) ;

  void checkQuery(std::shared_ptr<IndexSearcher> is,
                  std::shared_ptr<Query> query, int doc,
                  int maxTopN) ;

private:
  class UnifiedHighlighterAnonymousInnerClass : public UnifiedHighlighter
  {
    GET_CLASS_NAME(UnifiedHighlighterAnonymousInnerClass)
  private:
    std::shared_ptr<TestUnifiedHighlighterRanking> outerInstance;

    std::shared_ptr<org::apache::lucene::search::uhighlight::
                        TestUnifiedHighlighterRanking::FakePassageFormatter>
        f1;

  public:
    UnifiedHighlighterAnonymousInnerClass(
        std::shared_ptr<TestUnifiedHighlighterRanking> outerInstance,
        std::shared_ptr<IndexSearcher> is,
        std::shared_ptr<Analyzer> indexAnalyzer,
        std::shared_ptr<org::apache::lucene::search::uhighlight::
                            TestUnifiedHighlighterRanking::FakePassageFormatter>
            f1);

  protected:
    std::shared_ptr<PassageFormatter>
    getFormatter(const std::wstring &field) override;

  protected:
    std::shared_ptr<UnifiedHighlighterAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<UnifiedHighlighterAnonymousInnerClass>(
          UnifiedHighlighter::shared_from_this());
    }
  };

private:
  class UnifiedHighlighterAnonymousInnerClass2 : public UnifiedHighlighter
  {
    GET_CLASS_NAME(UnifiedHighlighterAnonymousInnerClass2)
  private:
    std::shared_ptr<TestUnifiedHighlighterRanking> outerInstance;

    std::shared_ptr<org::apache::lucene::search::uhighlight::
                        TestUnifiedHighlighterRanking::FakePassageFormatter>
        f2;

  public:
    UnifiedHighlighterAnonymousInnerClass2(
        std::shared_ptr<TestUnifiedHighlighterRanking> outerInstance,
        std::shared_ptr<IndexSearcher> is,
        std::shared_ptr<Analyzer> indexAnalyzer,
        std::shared_ptr<org::apache::lucene::search::uhighlight::
                            TestUnifiedHighlighterRanking::FakePassageFormatter>
            f2);

  protected:
    std::shared_ptr<PassageFormatter>
    getFormatter(const std::wstring &field) override;

  protected:
    std::shared_ptr<UnifiedHighlighterAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<UnifiedHighlighterAnonymousInnerClass2>(
          UnifiedHighlighter::shared_from_this());
    }
  };

  /**
   * returns a new random sentence, up to maxSentenceLength "words" in length.
   * each word is a single character (a-z). The first one is capitalized.
   */
private:
  std::wstring newSentence(std::shared_ptr<Random> r, int maxSentenceLength);

  /**
   * a fake formatter that doesn't actually format passages.
   * instead it just collects them for asserts!
   */
public:
  class FakePassageFormatter : public PassageFormatter
  {
    GET_CLASS_NAME(FakePassageFormatter)
  public:
    std::unordered_set<std::shared_ptr<Pair>> seen =
        std::unordered_set<std::shared_ptr<Pair>>();

    std::wstring format(std::deque<std::shared_ptr<Passage>> &passages,
                        const std::wstring &content) override;

  protected:
    std::shared_ptr<FakePassageFormatter> shared_from_this()
    {
      return std::static_pointer_cast<FakePassageFormatter>(
          PassageFormatter::shared_from_this());
    }
  };

public:
  class Pair : public std::enable_shared_from_this<Pair>
  {
    GET_CLASS_NAME(Pair)
  public:
    const int start;
    const int end;

    Pair(int start, int end);

    virtual int hashCode();

    bool equals(std::any obj) override;

    virtual std::wstring toString();
  };

  /**
   * sets b=0 to disable passage length normalization
   */
public:
  virtual void testCustomB() ;

private:
  class UnifiedHighlighterAnonymousInnerClass : public UnifiedHighlighter
  {
    GET_CLASS_NAME(UnifiedHighlighterAnonymousInnerClass)
  private:
    std::shared_ptr<TestUnifiedHighlighterRanking> outerInstance;

  public:
    UnifiedHighlighterAnonymousInnerClass(
        std::shared_ptr<TestUnifiedHighlighterRanking> outerInstance,
        std::shared_ptr<IndexSearcher> searcher,
        std::shared_ptr<Analyzer> indexAnalyzer);

  protected:
    std::shared_ptr<PassageScorer>
    getScorer(const std::wstring &field) override;

  protected:
    std::shared_ptr<UnifiedHighlighterAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<UnifiedHighlighterAnonymousInnerClass>(
          UnifiedHighlighter::shared_from_this());
    }
  };

  /**
   * sets k1=0 for simple coordinate-level match (# of query terms present)
   */
public:
  virtual void testCustomK1() ;

private:
  class UnifiedHighlighterAnonymousInnerClass2 : public UnifiedHighlighter
  {
    GET_CLASS_NAME(UnifiedHighlighterAnonymousInnerClass2)
  private:
    std::shared_ptr<TestUnifiedHighlighterRanking> outerInstance;

  public:
    UnifiedHighlighterAnonymousInnerClass2(
        std::shared_ptr<TestUnifiedHighlighterRanking> outerInstance,
        std::shared_ptr<IndexSearcher> searcher,
        std::shared_ptr<Analyzer> indexAnalyzer);

  protected:
    std::shared_ptr<PassageScorer>
    getScorer(const std::wstring &field) override;

  protected:
    std::shared_ptr<UnifiedHighlighterAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<UnifiedHighlighterAnonymousInnerClass2>(
          UnifiedHighlighter::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestUnifiedHighlighterRanking> shared_from_this()
  {
    return std::static_pointer_cast<TestUnifiedHighlighterRanking>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/uhighlight/
