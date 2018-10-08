#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

#include  "core/src/java/org/apache/lucene/index/IndexWriter.h"
#include  "core/src/java/org/apache/lucene/document/Document.h"

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

using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestIndexWriterMerging : public LuceneTestCase
{
  GET_CLASS_NAME(TestIndexWriterMerging)

  /**
   * Tests that index merging (specifically addIndexes(Directory...)) doesn't
   * change the index order of documents.
   */
public:
  virtual void testLucene() ;

private:
  bool verifyIndex(std::shared_ptr<Directory> directory,
                   int startAt) ;

  void fillIndex(std::shared_ptr<Random> random, std::shared_ptr<Directory> dir,
                 int start, int numDocs) ;

  // LUCENE-325: test forceMergeDeletes, when 2 singular merges
  // are required
public:
  virtual void testForceMergeDeletes() ;

  // LUCENE-325: test forceMergeDeletes, when many adjacent merges are required
  virtual void testForceMergeDeletes2() ;

  // LUCENE-325: test forceMergeDeletes without waiting, when
  // many adjacent merges are required
  virtual void testForceMergeDeletes3() ;

  // Just intercepts all merges & verifies that we are never
  // merging a segment with >= 20 (maxMergeDocs) docs
private:
  class MyMergeScheduler : public MergeScheduler
  {
    GET_CLASS_NAME(MyMergeScheduler)
  public:
    // C++ WARNING: The following method was originally marked 'synchronized':
    void merge(std::shared_ptr<IndexWriter> writer, MergeTrigger trigger,
               bool newMergesFound)  override;

    virtual ~MyMergeScheduler();

  protected:
    std::shared_ptr<MyMergeScheduler> shared_from_this()
    {
      return std::static_pointer_cast<MyMergeScheduler>(
          MergeScheduler::shared_from_this());
    }
  };

  // LUCENE-1013
public:
  virtual void testSetMaxMergeDocs() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Slow public void testNoWaitClose() throws Throwable
  virtual void testNoWaitClose() ;

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<TestIndexWriterMerging> outerInstance;

    std::shared_ptr<Document> doc;
    std::shared_ptr<org::apache::lucene::index::IndexWriter> finalWriter;
    std::shared_ptr<AtomicReference<std::runtime_error>> failure;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<TestIndexWriterMerging> outerInstance,
        std::shared_ptr<Document> doc,
        std::shared_ptr<org::apache::lucene::index::IndexWriter> finalWriter,
        std::shared_ptr<AtomicReference<std::runtime_error>> failure);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestIndexWriterMerging> shared_from_this()
  {
    return std::static_pointer_cast<TestIndexWriterMerging>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
