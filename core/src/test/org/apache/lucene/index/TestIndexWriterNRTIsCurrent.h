#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/DirectoryReader.h"

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

class TestIndexWriterNRTIsCurrent : public LuceneTestCase
{
  GET_CLASS_NAME(TestIndexWriterNRTIsCurrent)

public:
  class ReaderHolder : public std::enable_shared_from_this<ReaderHolder>
  {
    GET_CLASS_NAME(ReaderHolder)
  public:
    // C++ TODO: 'volatile' has a different meaning in C++:
    // ORIGINAL LINE: volatile DirectoryReader reader;
    std::shared_ptr<DirectoryReader> reader;
    // C++ TODO: 'volatile' has a different meaning in C++:
    // ORIGINAL LINE: volatile bool stop = false;
    bool stop = false;
  };

public:
  virtual void testIsCurrentWithThreads() throw(IOException,
                                                InterruptedException);

public:
  class WriterThread : public Thread
  {
    GET_CLASS_NAME(WriterThread)
  private:
    const std::shared_ptr<ReaderHolder> holder;
    const std::shared_ptr<IndexWriter> writer;
    const int numOps;
    bool countdown = true;
    const std::shared_ptr<CountDownLatch> latch;

  public:
    std::runtime_error failed;

    WriterThread(std::shared_ptr<ReaderHolder> holder,
                 std::shared_ptr<IndexWriter> writer, int numOps,
                 std::shared_ptr<Random> random,
                 std::shared_ptr<CountDownLatch> latch);

    void run() override;

  protected:
    std::shared_ptr<WriterThread> shared_from_this()
    {
      return std::static_pointer_cast<WriterThread>(Thread::shared_from_this());
    }
  };

public:
  class ReaderThread final : public Thread
  {
    GET_CLASS_NAME(ReaderThread)
  private:
    const std::shared_ptr<ReaderHolder> holder;
    const std::shared_ptr<CountDownLatch> latch;

  public:
    std::runtime_error failed;

    ReaderThread(std::shared_ptr<ReaderHolder> holder,
                 std::shared_ptr<CountDownLatch> latch);

    void run() override;

  protected:
    std::shared_ptr<ReaderThread> shared_from_this()
    {
      return std::static_pointer_cast<ReaderThread>(Thread::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestIndexWriterNRTIsCurrent> shared_from_this()
  {
    return std::static_pointer_cast<TestIndexWriterNRTIsCurrent>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
