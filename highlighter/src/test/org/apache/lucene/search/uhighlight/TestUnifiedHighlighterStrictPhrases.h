#pragma once
#include "stringhelper.h"
#include <any>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/document/FieldType.h"

#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/analysis/MockAnalyzer.h"
#include  "core/src/java/org/apache/lucene/index/RandomIndexWriter.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/uhighlight/UnifiedHighlighter.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/document/Document.h"
#include  "core/src/java/org/apache/lucene/search/PhraseQuery.h"
#include  "core/src/java/org/apache/lucene/search/uhighlight/Passage.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/search/Weight.h"

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

using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using FieldType = org::apache::lucene::document::FieldType;
using IndexReader = org::apache::lucene::index::IndexReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using PhraseQuery = org::apache::lucene::search::PhraseQuery;
using Query = org::apache::lucene::search::Query;
using Weight = org::apache::lucene::search::Weight;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

// TODO rename to reflect position sensitivity
class TestUnifiedHighlighterStrictPhrases : public LuceneTestCase
{
  GET_CLASS_NAME(TestUnifiedHighlighterStrictPhrases)

public:
  const std::shared_ptr<FieldType> fieldType;

  std::shared_ptr<Directory> dir;
  std::shared_ptr<MockAnalyzer> indexAnalyzer;
  std::shared_ptr<RandomIndexWriter> indexWriter;
  std::shared_ptr<IndexSearcher> searcher;
  std::shared_ptr<UnifiedHighlighter> highlighter;
  std::shared_ptr<IndexReader> indexReader;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @ParametersFactory public static Iterable<Object[]>
  // parameters()
  static std::deque<std::deque<std::any>> parameters();

  TestUnifiedHighlighterStrictPhrases(std::shared_ptr<FieldType> fieldType);

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Before public void doBefore() throws java.io.IOException
  virtual void doBefore() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @After public void doAfter() throws java.io.IOException
  virtual void doAfter() ;

private:
  std::shared_ptr<Document> newDoc(std::deque<std::wstring> &bodyVals);

  void initReaderSearcherHighlighter() ;

  std::shared_ptr<PhraseQuery> newPhraseQuery(const std::wstring &field,
                                              const std::wstring &phrase);

  std::shared_ptr<PhraseQuery> setSlop(std::shared_ptr<PhraseQuery> query,
                                       int slop);

public:
  virtual void testBasics() ;

  virtual void testWithSameTermQuery() ;

private:
  class PassageFormatterAnonymousInnerClass : public PassageFormatter
  {
    GET_CLASS_NAME(PassageFormatterAnonymousInnerClass)
  private:
    std::shared_ptr<TestUnifiedHighlighterStrictPhrases> outerInstance;

  public:
    PassageFormatterAnonymousInnerClass(
        std::shared_ptr<TestUnifiedHighlighterStrictPhrases> outerInstance);

    std::any format(std::deque<std::shared_ptr<Passage>> &passages,
                    const std::wstring &content) override;

  protected:
    std::shared_ptr<PassageFormatterAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<PassageFormatterAnonymousInnerClass>(
          PassageFormatter::shared_from_this());
    }
  };

public:
  virtual void testPhraseNotInDoc() ;

  virtual void testSubPhrases() ;

private:
  class PassageFormatterAnonymousInnerClass2 : public PassageFormatter
  {
    GET_CLASS_NAME(PassageFormatterAnonymousInnerClass2)
  private:
    std::shared_ptr<TestUnifiedHighlighterStrictPhrases> outerInstance;

  public:
    PassageFormatterAnonymousInnerClass2(
        std::shared_ptr<TestUnifiedHighlighterStrictPhrases> outerInstance);

    std::any format(std::deque<std::shared_ptr<Passage>> &passages,
                    const std::wstring &content) override;

  protected:
    std::shared_ptr<PassageFormatterAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<PassageFormatterAnonymousInnerClass2>(
          PassageFormatter::shared_from_this());
    }
  };

public:
  virtual void testSynonyms() ;

  /**
   * Test it does *not* highlight the same term's not next to the span-near.
   * "charlie" in this case. This particular example exercises "Rewrite" plus
   * "MTQ" in the same query.
   */
  virtual void testRewriteAndMtq() ;

  /**
   * Like {@link #testRewriteAndMtq} but no freestanding MTQ
   */
  virtual void testRewrite() ;

  /**
   * Like {@link #testRewriteAndMtq} but no rewrite.
   */
  virtual void testMtq() ;

  virtual void testMultiValued() ;

  virtual void testMaxLen() ;

  virtual void testFilteredOutSpan() ;

  virtual void testMatchNoDocsQuery() ;

  virtual void testPreSpanQueryRewrite() ;

private:
  class UnifiedHighlighterAnonymousInnerClass : public UnifiedHighlighter
  {
    GET_CLASS_NAME(UnifiedHighlighterAnonymousInnerClass)
  private:
    std::shared_ptr<TestUnifiedHighlighterStrictPhrases> outerInstance;

  public:
    UnifiedHighlighterAnonymousInnerClass(
        std::shared_ptr<TestUnifiedHighlighterStrictPhrases> outerInstance,
        std::shared_ptr<IndexSearcher> searcher,
        std::shared_ptr<MockAnalyzer> indexAnalyzer);

  protected:
    std::shared_ptr<std::deque<std::shared_ptr<Query>>>
    preSpanQueryRewrite(std::shared_ptr<Query> query) override;

  protected:
    std::shared_ptr<UnifiedHighlighterAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<UnifiedHighlighterAnonymousInnerClass>(
          UnifiedHighlighter::shared_from_this());
    }
  };

private:
  class MyQuery : public Query
  {
    GET_CLASS_NAME(MyQuery)

  private:
    const std::shared_ptr<Query> wrapped;

  public:
    MyQuery(std::shared_ptr<Query> wrapped);

    std::shared_ptr<Weight>
    createWeight(std::shared_ptr<IndexSearcher> searcher, bool needsScores,
                 float boost)  override;

    std::shared_ptr<Query>
    rewrite(std::shared_ptr<IndexReader> reader)  override;

    std::wstring toString(const std::wstring &field) override;

    bool equals(std::any obj) override;

    virtual int hashCode();

  protected:
    std::shared_ptr<MyQuery> shared_from_this()
    {
      return std::static_pointer_cast<MyQuery>(
          org.apache.lucene.search.Query::shared_from_this());
    }
  };

  // Ported from LUCENE-5455 (fixed in LUCENE-8121).  Also see LUCENE-2287.
public:
  virtual void testNestedSpanQueryHighlight() ;

protected:
  std::shared_ptr<TestUnifiedHighlighterStrictPhrases> shared_from_this()
  {
    return std::static_pointer_cast<TestUnifiedHighlighterStrictPhrases>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/uhighlight/
