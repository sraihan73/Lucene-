#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

#include  "core/src/java/org/apache/lucene/index/IndexWriter.h"

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

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressCodecs({ "SimpleText", "Memory", "Direct" })
// @TimeoutSuite(millis = 8 * TimeUnits.HOUR) public class TestIndexWriterMaxDocs
// extends org.apache.lucene.util.LuceneTestCase
class TestIndexWriterMaxDocs : public LuceneTestCase
{
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Monster("takes over two hours") public void
  // testExactlyAtTrueLimit() throws Exception
  virtual void testExactlyAtTrueLimit() ;

  virtual void testAddDocument() ;

  virtual void testAddDocuments() ;

  virtual void testUpdateDocument() ;

  virtual void testUpdateDocuments() ;

  virtual void testReclaimedDeletes() ;

  // Tests that 100% deleted segments (which IW "specializes" by dropping
  // entirely) are not mis-counted
  virtual void testReclaimedDeletesWholeSegments() ;

  virtual void testAddIndexes() ;

  // Make sure MultiReader lets you search exactly the limit number of docs:
  virtual void testMultiReaderExactLimit() ;

  // Make sure MultiReader is upset if you exceed the limit
  virtual void testMultiReaderBeyondLimit() ;

  /**
   * LUCENE-6299: Test if addindexes(Dir[]) prevents exceeding max docs.
   */
  virtual void testAddTooManyIndexesDir() ;

private:
  class FilterDirectoryAnonymousInnerClass : public FilterDirectory
  {
    GET_CLASS_NAME(FilterDirectoryAnonymousInnerClass)
  private:
    std::shared_ptr<TestIndexWriterMaxDocs> outerInstance;

  public:
    FilterDirectoryAnonymousInnerClass(
        std::shared_ptr<TestIndexWriterMaxDocs> outerInstance,
        std::shared_ptr<Directory> dir);

  protected:
    std::shared_ptr<FilterDirectoryAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FilterDirectoryAnonymousInnerClass>(
          org.apache.lucene.store.FilterDirectory::shared_from_this());
    }
  };

  /**
   * LUCENE-6299: Test if addindexes(CodecReader[]) prevents exceeding max docs.
   */
public:
  virtual void testAddTooManyIndexesCodecReader() ;

  virtual void testTooLargeMaxDocs() ;

  // LUCENE-6299
  virtual void testDeleteAll() ;

  // LUCENE-6299
  virtual void testDeleteAllAfterFlush() ;

  // LUCENE-6299
  virtual void testDeleteAllAfterCommit() ;

  // LUCENE-6299
  virtual void testDeleteAllMultipleThreads() ;

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<TestIndexWriterMaxDocs> outerInstance;

    std::shared_ptr<org::apache::lucene::index::IndexWriter> w;
    std::shared_ptr<CountDownLatch> startingGun;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<TestIndexWriterMaxDocs> outerInstance,
        std::shared_ptr<org::apache::lucene::index::IndexWriter> w,
        std::shared_ptr<CountDownLatch> startingGun);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

  // LUCENE-6299
public:
  virtual void testDeleteAllAfterClose() ;

  // LUCENE-6299
  virtual void testAcrossTwoIndexWriters() ;

  // LUCENE-6299
  virtual void testCorruptIndexExceptionTooLarge() ;

  // LUCENE-6299
  virtual void
  testCorruptIndexExceptionTooLargeWriter() ;

protected:
  std::shared_ptr<TestIndexWriterMaxDocs> shared_from_this()
  {
    return std::static_pointer_cast<TestIndexWriterMaxDocs>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
