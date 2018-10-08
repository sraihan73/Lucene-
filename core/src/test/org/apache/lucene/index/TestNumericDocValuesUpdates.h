#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::document
{
class Document;
}

namespace org::apache::lucene::codecs
{
class DocValuesFormat;
}
namespace org::apache::lucene::index
{
class IndexWriter;
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

using Document = org::apache::lucene::document::Document;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("resource") public class
// TestNumericDocValuesUpdates extends org.apache.lucene.util.LuceneTestCase
class TestNumericDocValuesUpdates : public LuceneTestCase
{

private:
  std::shared_ptr<Document> doc(int id);

  std::shared_ptr<Document> doc(int id, int64_t val);

public:
  virtual void testMultipleUpdatesSameDoc() ;

  virtual void testBiasedMixOfRandomUpdates() ;

  virtual void testUpdatesAreFlushed() ;

  virtual void testSimple() ;

  virtual void testUpdateFewSegments() ;

  virtual void testReopen() ;

  virtual void testUpdatesAndDeletes() ;

  virtual void testUpdatesWithDeletes() ;

  virtual void testMultipleDocValuesTypes() ;

  virtual void testMultipleNumericDocValues() ;

  virtual void testDocumentWithNoValue() ;

  virtual void testUpdateNonNumericDocValuesField() ;

  virtual void testDifferentDVFormatPerField() ;

private:
  class AssertingCodecAnonymousInnerClass : public AssertingCodec
  {
    GET_CLASS_NAME(AssertingCodecAnonymousInnerClass)
  private:
    std::shared_ptr<TestNumericDocValuesUpdates> outerInstance;

  public:
    AssertingCodecAnonymousInnerClass(
        std::shared_ptr<TestNumericDocValuesUpdates> outerInstance);

    std::shared_ptr<DocValuesFormat>
    getDocValuesFormatForField(const std::wstring &field) override;

  protected:
    std::shared_ptr<AssertingCodecAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AssertingCodecAnonymousInnerClass>(
          org.apache.lucene.codecs.asserting
              .AssertingCodec::shared_from_this());
    }
  };

public:
  virtual void testUpdateSameDocMultipleTimes() ;

  virtual void testSegmentMerges() ;

  virtual void testUpdateDocumentByMultipleTerms() ;

public:
  class OneSortDoc : public std::enable_shared_from_this<OneSortDoc>,
                     public Comparable<std::shared_ptr<OneSortDoc>>
  {
    GET_CLASS_NAME(OneSortDoc)
  public:
    int64_t value = 0;
    const int64_t sortValue;
    const int id;
    bool deleted = false;

    OneSortDoc(int id, int64_t value, int64_t sortValue);

    int compareTo(std::shared_ptr<OneSortDoc> other) override;
  };

public:
  virtual void testSortedIndex() ;

  virtual void testManyReopensAndFields() ;

  virtual void testUpdateSegmentWithNoDocValues() ;

  virtual void testUpdateSegmentWithNoDocValues2() ;

  virtual void
  testUpdateSegmentWithPostingButNoDocValues() ;

  virtual void testUpdateNumericDVFieldWithSameNameAsPostingField() throw(
      std::runtime_error);

  virtual void testStressMultiThreading() ;

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<TestNumericDocValuesUpdates> outerInstance;

    std::shared_ptr<org::apache::lucene::index::IndexWriter> writer;
    int numFields = 0;
    int numDocs = 0;
    std::shared_ptr<CountDownLatch> done;
    std::shared_ptr<AtomicInteger> numUpdates;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<TestNumericDocValuesUpdates> outerInstance,
        std::wstring L"UpdateThread-" + i,
        std::shared_ptr<org::apache::lucene::index::IndexWriter> writer,
        int numFields, int numDocs, std::shared_ptr<CountDownLatch> done,
        std::shared_ptr<AtomicInteger> numUpdates);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

public:
  virtual void
  testUpdateDifferentDocsInDifferentGens() ;

  virtual void testChangeCodec() ;

private:
  class AssertingCodecAnonymousInnerClass : public AssertingCodec
  {
    GET_CLASS_NAME(AssertingCodecAnonymousInnerClass)
  private:
    std::shared_ptr<TestNumericDocValuesUpdates> outerInstance;

  public:
    AssertingCodecAnonymousInnerClass(
        std::shared_ptr<TestNumericDocValuesUpdates> outerInstance);

    std::shared_ptr<DocValuesFormat>
    getDocValuesFormatForField(const std::wstring &field) override;

  protected:
    std::shared_ptr<AssertingCodecAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AssertingCodecAnonymousInnerClass>(
          org.apache.lucene.codecs.asserting
              .AssertingCodec::shared_from_this());
    }
  };

private:
  class AssertingCodecAnonymousInnerClass2 : public AssertingCodec
  {
    GET_CLASS_NAME(AssertingCodecAnonymousInnerClass2)
  private:
    std::shared_ptr<TestNumericDocValuesUpdates> outerInstance;

  public:
    AssertingCodecAnonymousInnerClass2(
        std::shared_ptr<TestNumericDocValuesUpdates> outerInstance);

    std::shared_ptr<DocValuesFormat>
    getDocValuesFormatForField(const std::wstring &field) override;

  protected:
    std::shared_ptr<AssertingCodecAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<AssertingCodecAnonymousInnerClass2>(
          org.apache.lucene.codecs.asserting
              .AssertingCodec::shared_from_this());
    }
  };

public:
  virtual void testAddIndexes() ;

  virtual void testDeleteUnusedUpdatesFiles() ;

  virtual void testTonsOfUpdates() ;

  virtual void testUpdatesOrder() ;

  virtual void testUpdateAllDeletedSegment() ;

  virtual void testUpdateTwoNonexistingTerms() ;

  virtual void testIOContext() ;

protected:
  std::shared_ptr<TestNumericDocValuesUpdates> shared_from_this()
  {
    return std::static_pointer_cast<TestNumericDocValuesUpdates>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
