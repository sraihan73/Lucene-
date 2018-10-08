#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class IndexWriter;
}

namespace org::apache::lucene::document
{
class Document;
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
namespace org::apache::lucene::index
{

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 *
 * Tests DocValues integration into IndexWriter
 *
 */
class TestDocValuesIndexing : public LuceneTestCase
{
  GET_CLASS_NAME(TestDocValuesIndexing)
  /*
   * - add test for multi segment case with deletes
   * - add multithreaded tests / integrate into stress indexing?
   */

public:
  virtual void testAddIndexes() ;

  virtual void testMultiValuedDocValuesField() ;

  virtual void testDifferentTypedDocValuesField() ;

  virtual void testDifferentTypedDocValuesField2() ;

  // LUCENE-3870
  virtual void testLengthPrefixAcrossTwoPages() ;

  virtual void testDocValuesUnstored() ;

  // Same field in one document as different types:
  virtual void testMixedTypesSameDocument() ;

  // Two documents with same field as different types:
  virtual void testMixedTypesDifferentDocuments() ;

  virtual void testAddSortedTwice() ;

  virtual void testAddBinaryTwice() ;

  virtual void testAddNumericTwice() ;

  virtual void testTooLargeSortedBytes() ;

  virtual void testTooLargeTermSortedSetBytes() ;

  // Two documents across segments
  virtual void testMixedTypesDifferentSegments() ;

  // Add inconsistent document after deleteAll
  virtual void testMixedTypesAfterDeleteAll() ;

  // Add inconsistent document after reopening IW w/ create
  virtual void testMixedTypesAfterReopenCreate() ;

  virtual void testMixedTypesAfterReopenAppend1() ;

  virtual void testMixedTypesAfterReopenAppend2() ;

  virtual void testMixedTypesAfterReopenAppend3() ;

  // Two documents with same field as different types, added
  // from separate threads:
  virtual void testMixedTypesDifferentThreads() ;

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<TestDocValuesIndexing> outerInstance;

    std::shared_ptr<org::apache::lucene::index::IndexWriter> w;
    std::shared_ptr<CountDownLatch> startingGun;
    std::shared_ptr<AtomicBoolean> hitExc;
    std::shared_ptr<Document> doc;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<TestDocValuesIndexing> outerInstance,
        std::shared_ptr<org::apache::lucene::index::IndexWriter> w,
        std::shared_ptr<CountDownLatch> startingGun,
        std::shared_ptr<AtomicBoolean> hitExc, std::shared_ptr<Document> doc);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

  // Adding documents via addIndexes
public:
  virtual void testMixedTypesViaAddIndexes() ;

  virtual void testIllegalTypeChange() ;

  virtual void testIllegalTypeChangeAcrossSegments() ;

  virtual void testTypeChangeAfterCloseAndDeleteAll() ;

  virtual void testTypeChangeAfterDeleteAll() ;

  virtual void
  testTypeChangeAfterCommitAndDeleteAll() ;

  virtual void testTypeChangeAfterOpenCreate() ;

  virtual void testTypeChangeViaAddIndexes() ;

  virtual void testTypeChangeViaAddIndexesIR() ;

  virtual void testTypeChangeViaAddIndexes2() ;

  virtual void testTypeChangeViaAddIndexesIR2() ;

  virtual void testDocsWithField() ;

  virtual void
  testSameFieldNameForPostingAndDocValue() ;

  // LUCENE-6049
  virtual void testExcIndexingDocBeforeDocValues() ;

private:
  class TokenStreamAnonymousInnerClass : public TokenStream
  {
    GET_CLASS_NAME(TokenStreamAnonymousInnerClass)
  private:
    std::shared_ptr<TestDocValuesIndexing> outerInstance;

  public:
    TokenStreamAnonymousInnerClass(
        std::shared_ptr<TestDocValuesIndexing> outerInstance);

    bool incrementToken() override;

  protected:
    std::shared_ptr<TokenStreamAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<TokenStreamAnonymousInnerClass>(
          org.apache.lucene.analysis.TokenStream::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestDocValuesIndexing> shared_from_this()
  {
    return std::static_pointer_cast<TestDocValuesIndexing>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
