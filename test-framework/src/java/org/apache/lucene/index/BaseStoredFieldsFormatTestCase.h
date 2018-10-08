#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/document/Document.h"

#include  "core/src/java/org/apache/lucene/index/DirectoryReader.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/index/LeafReader.h"
#include  "core/src/java/org/apache/lucene/index/StoredFieldVisitor.h"

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

/**
 * Base class aiming at testing {@link StoredFieldsFormat stored fields
formats}.
 * To test a new format, all you need is to register a new {@link Codec} which
GET_CLASS_NAME(aiming)
 * uses it and extend this class and override {@link #getCodec()}.
 * @lucene.experimental
 */
class BaseStoredFieldsFormatTestCase : public BaseIndexFileFormatTestCase
{
  GET_CLASS_NAME(BaseStoredFieldsFormatTestCase)

protected:
  void addRandomFields(std::shared_ptr<Document> d) override;

public:
  virtual void testRandomStoredFields() ;

  // LUCENE-1727: make sure doc fields are stored in order
  virtual void testStoredFieldsOrder() ;

  // LUCENE-1219
  virtual void testBinaryFieldOffsetLength() ;

  virtual void testNumericField() ;

  virtual void testIndexedBit() ;

  virtual void testReadSkip() ;

  virtual void testEmptyDocs() ;

  virtual void testConcurrentReads() ;

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<BaseStoredFieldsFormatTestCase> outerInstance;

    int numDocs = 0;
    std::shared_ptr<org::apache::lucene::index::DirectoryReader> rd;
    std::shared_ptr<IndexSearcher> searcher;
    int readsPerThread = 0;
    std::shared_ptr<AtomicReference<std::runtime_error>> ex;
    int i = 0;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<BaseStoredFieldsFormatTestCase> outerInstance,
        int numDocs,
        std::shared_ptr<org::apache::lucene::index::DirectoryReader> rd,
        std::shared_ptr<IndexSearcher> searcher, int readsPerThread,
        std::shared_ptr<AtomicReference<std::runtime_error>> ex, int i);

    std::deque<int> queries;

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

private:
  std::deque<char> randomByteArray(int length, int max);

public:
  virtual void testWriteReadMerge() ;

  /** A dummy filter reader that reverse the order of documents in stored
   * fields. */
private:
  class DummyFilterLeafReader : public FilterLeafReader
  {
    GET_CLASS_NAME(DummyFilterLeafReader)

  public:
    DummyFilterLeafReader(std::shared_ptr<LeafReader> in_);

    void document(int docID, std::shared_ptr<StoredFieldVisitor> visitor) throw(
        IOException) override;

    std::shared_ptr<CacheHelper> getCoreCacheHelper() override;

    std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

  protected:
    std::shared_ptr<DummyFilterLeafReader> shared_from_this()
    {
      return std::static_pointer_cast<DummyFilterLeafReader>(
          FilterLeafReader::shared_from_this());
    }
  };

private:
  class DummyFilterDirectoryReader : public FilterDirectoryReader
  {
    GET_CLASS_NAME(DummyFilterDirectoryReader)

  public:
    DummyFilterDirectoryReader(std::shared_ptr<DirectoryReader> in_) throw(
        IOException);

  private:
    class SubReaderWrapperAnonymousInnerClass : public SubReaderWrapper
    {
      GET_CLASS_NAME(SubReaderWrapperAnonymousInnerClass)
    public:
      SubReaderWrapperAnonymousInnerClass();

      std::shared_ptr<LeafReader>
      wrap(std::shared_ptr<LeafReader> reader) override;

    protected:
      std::shared_ptr<SubReaderWrapperAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<SubReaderWrapperAnonymousInnerClass>(
            SubReaderWrapper::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<DirectoryReader> doWrapDirectoryReader(
        std::shared_ptr<DirectoryReader> in_)  override;

  public:
    std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

  protected:
    std::shared_ptr<DummyFilterDirectoryReader> shared_from_this()
    {
      return std::static_pointer_cast<DummyFilterDirectoryReader>(
          FilterDirectoryReader::shared_from_this());
    }
  };

public:
  virtual void testMergeFilterReader() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Nightly public void testBigDocuments() throws
  // java.io.IOException
  virtual void testBigDocuments() ;

  virtual void testBulkMergeWithDeletes() ;

  /** mix up field numbers, merge, and check that data is correct */
  virtual void testMismatchedFields() ;

protected:
  std::shared_ptr<BaseStoredFieldsFormatTestCase> shared_from_this()
  {
    return std::static_pointer_cast<BaseStoredFieldsFormatTestCase>(
        BaseIndexFileFormatTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
