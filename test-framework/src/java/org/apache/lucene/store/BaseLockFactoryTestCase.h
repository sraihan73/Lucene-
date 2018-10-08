#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
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
namespace org::apache::lucene::store
{

using IndexWriter = org::apache::lucene::index::IndexWriter;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/** Base class for per-LockFactory tests. */
class BaseLockFactoryTestCase : public LuceneTestCase
{

  /** Subclass returns the Directory to be tested; if it's
   *  an FS-based directory it should point to the specified
   *  path, else it can ignore it. */
protected:
  virtual std::shared_ptr<Directory>
  getDirectory(std::shared_ptr<Path> path) = 0;

  /** Test obtaining and releasing locks, checking validity */
public:
  virtual void testBasics() ;

  /** Test closing locks twice */
  virtual void testDoubleClose() ;

  /** Test ensureValid returns true after acquire */
  virtual void testValidAfterAcquire() ;

  /** Test ensureValid throws exception after close */
  virtual void testInvalidAfterClose() ;

  virtual void testObtainConcurrently() throw(InterruptedException,
                                              IOException);

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<BaseLockFactoryTestCase> outerInstance;

    std::shared_ptr<org::apache::lucene::store::Directory> directory;
    std::shared_ptr<AtomicBoolean> running;
    std::shared_ptr<AtomicInteger> atomicCounter;
    std::shared_ptr<ReentrantLock> assertingLock;
    int runs = 0;
    std::shared_ptr<CyclicBarrier> barrier;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<BaseLockFactoryTestCase> outerInstance,
        std::shared_ptr<org::apache::lucene::store::Directory> directory,
        std::shared_ptr<AtomicBoolean> running,
        std::shared_ptr<AtomicInteger> atomicCounter,
        std::shared_ptr<ReentrantLock> assertingLock, int runs,
        std::shared_ptr<CyclicBarrier> barrier);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

  // Verify: do stress test, by opening IndexReaders and
  // IndexWriters over & over in 2 threads and making sure
  // no unexpected exceptions are raised:
public:
  virtual void testStressLocks() ;

private:
  void addDoc(std::shared_ptr<IndexWriter> writer) ;

private:
  class WriterThread : public Thread
  {
    GET_CLASS_NAME(WriterThread)
  private:
    std::shared_ptr<BaseLockFactoryTestCase> outerInstance;

    std::shared_ptr<Directory> dir;
    int numIteration = 0;

  public:
    bool hitException = false;
    WriterThread(std::shared_ptr<BaseLockFactoryTestCase> outerInstance,
                 int numIteration, std::shared_ptr<Directory> dir);

  private:
    std::wstring toString(std::shared_ptr<ByteArrayOutputStream> baos);

  public:
    void run() override;

  protected:
    std::shared_ptr<WriterThread> shared_from_this()
    {
      return std::static_pointer_cast<WriterThread>(Thread::shared_from_this());
    }
  };

private:
  class SearcherThread : public Thread
  {
    GET_CLASS_NAME(SearcherThread)
  private:
    std::shared_ptr<Directory> dir;
    int numIteration = 0;

  public:
    bool hitException = false;
    SearcherThread(int numIteration, std::shared_ptr<Directory> dir);
    void run() override;

  protected:
    std::shared_ptr<SearcherThread> shared_from_this()
    {
      return std::static_pointer_cast<SearcherThread>(
          Thread::shared_from_this());
    }
  };

protected:
  std::shared_ptr<BaseLockFactoryTestCase> shared_from_this()
  {
    return std::static_pointer_cast<BaseLockFactoryTestCase>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/store/
