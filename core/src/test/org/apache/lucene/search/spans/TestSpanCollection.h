#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"

#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/document/FieldType.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/index/PostingsEnum.h"
#include  "core/src/java/org/apache/lucene/search/spans/Spans.h"

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
namespace org::apache::lucene::search::spans
{

using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using IndexReader = org::apache::lucene::index::IndexReader;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Term = org::apache::lucene::index::Term;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestSpanCollection : public LuceneTestCase
{
  GET_CLASS_NAME(TestSpanCollection)

protected:
  std::shared_ptr<IndexSearcher> searcher;
  std::shared_ptr<Directory> directory;
  std::shared_ptr<IndexReader> reader;

public:
  static const std::wstring FIELD;

  static std::shared_ptr<FieldType> OFFSETS;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static TestSpanCollection::StaticConstructor staticConstructor;

public:
  void tearDown()  override;

  void setUp()  override;

private:
  class TermCollector : public std::enable_shared_from_this<TermCollector>,
                        public SpanCollector
  {
    GET_CLASS_NAME(TermCollector)

  public:
    const std::shared_ptr<Set<std::shared_ptr<Term>>> terms =
        std::unordered_set<std::shared_ptr<Term>>();

    void collectLeaf(std::shared_ptr<PostingsEnum> postings, int position,
                     std::shared_ptr<Term> term)  override;

    void reset() override;
  };

protected:
  std::deque<std::wstring> docFields = {L"w1 w2 w3 w4 w5", L"w1 w3 w2 w3 zz",
                                         L"w1 xx w2 yy w4",
                                         L"w1 w2 w1 w4 w2 w3"};

private:
  void checkCollectedTerms(std::shared_ptr<Spans> spans,
                           std::shared_ptr<TermCollector> collector,
                           std::deque<Term> &expectedTerms) ;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testNestedNearQuery() throws
  // java.io.IOException
  virtual void testNestedNearQuery() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testOrQuery() throws java.io.IOException
  virtual void testOrQuery() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Test public void testSpanNotQuery() throws
  // java.io.IOException
  virtual void testSpanNotQuery() ;

protected:
  std::shared_ptr<TestSpanCollection> shared_from_this()
  {
    return std::static_pointer_cast<TestSpanCollection>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/spans/
