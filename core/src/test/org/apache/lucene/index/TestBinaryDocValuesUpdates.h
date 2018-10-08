#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class BinaryDocValues;
}

namespace org::apache::lucene::util
{
class BytesRef;
}
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
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

class TestBinaryDocValuesUpdates : public LuceneTestCase
{
  GET_CLASS_NAME(TestBinaryDocValuesUpdates)

public:
  static int64_t
  getValue(std::shared_ptr<BinaryDocValues> bdv) ;

  // encodes a long into a BytesRef as VLong so that we get varying number of
  // bytes when we update
  static std::shared_ptr<BytesRef> toBytes(int64_t value);

private:
  std::shared_ptr<Document> doc(int id);

public:
  virtual void testUpdatesAreFlushed() ;

  virtual void testSimple() ;

  virtual void testUpdateFewSegments() ;

  virtual void testReopen() ;

  virtual void testUpdatesAndDeletes() ;

  virtual void testUpdatesWithDeletes() ;

  virtual void testMultipleDocValuesTypes() ;

  virtual void testMultipleBinaryDocValues() ;

  virtual void testDocumentWithNoValue() ;

  virtual void testUpdateNonBinaryDocValuesField() ;

  virtual void testDifferentDVFormatPerField() ;

private:
  class AssertingCodecAnonymousInnerClass : public AssertingCodec
  {
    GET_CLASS_NAME(AssertingCodecAnonymousInnerClass)
  private:
    std::shared_ptr<TestBinaryDocValuesUpdates> outerInstance;

  public:
    AssertingCodecAnonymousInnerClass(
        std::shared_ptr<TestBinaryDocValuesUpdates> outerInstance);

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
    std::shared_ptr<BytesRef> value;
    const int64_t sortValue;
    const int id;
    bool deleted = false;

    OneSortDoc(int id, std::shared_ptr<BytesRef> value, int64_t sortValue);

    int compareTo(std::shared_ptr<OneSortDoc> other) override;
  };

public:
  virtual void testSortedIndex() ;

  virtual void testManyReopensAndFields() ;

  virtual void testUpdateSegmentWithNoDocValues() ;

  virtual void
  testUpdateSegmentWithPostingButNoDocValues() ;

  virtual void
  testUpdateBinaryDVFieldWithSameNameAsPostingField() ;

  virtual void testStressMultiThreading() ;

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<TestBinaryDocValuesUpdates> outerInstance;

    std::shared_ptr<org::apache::lucene::index::IndexWriter> writer;
    int numFields = 0;
    int numDocs = 0;
    std::shared_ptr<CountDownLatch> done;
    std::shared_ptr<AtomicInteger> numUpdates;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<TestBinaryDocValuesUpdates> outerInstance,
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
    std::shared_ptr<TestBinaryDocValuesUpdates> outerInstance;

  public:
    AssertingCodecAnonymousInnerClass(
        std::shared_ptr<TestBinaryDocValuesUpdates> outerInstance);

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
    std::shared_ptr<TestBinaryDocValuesUpdates> outerInstance;

  public:
    AssertingCodecAnonymousInnerClass2(
        std::shared_ptr<TestBinaryDocValuesUpdates> outerInstance);

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

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Nightly public void testTonsOfUpdates() throws Exception
  virtual void testTonsOfUpdates() ;

  virtual void testUpdatesOrder() ;

  virtual void testUpdateAllDeletedSegment() ;

  virtual void testUpdateTwoNonexistingTerms() ;

  virtual void testIOContext() ;

protected:
  std::shared_ptr<TestBinaryDocValuesUpdates> shared_from_this()
  {
    return std::static_pointer_cast<TestBinaryDocValuesUpdates>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
