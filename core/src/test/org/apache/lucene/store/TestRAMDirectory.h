#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <algorithm>
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

/**
 * JUnit testcase to test RAMDirectory. RAMDirectory itself is used in many
 * testcases, but not one of them uses an different constructor other than the
 * default constructor.
 */
class TestRAMDirectory : public BaseDirectoryTestCase
{
  GET_CLASS_NAME(TestRAMDirectory)

protected:
  std::shared_ptr<Directory> getDirectory(std::shared_ptr<Path> path) override;

  // add enough document so that the index will be larger than
  // RAMDirectory.READ_BUFFER_SIZE
private:
  static constexpr int DOCS_TO_ADD = 500;

  std::shared_ptr<Path> buildIndex() ;

  // LUCENE-1468
public:
  virtual void testCopySubdir() ;

  virtual void testRAMDirectory() ;

private:
  static constexpr int NUM_THREADS = 10;
  static constexpr int DOCS_PER_THREAD = 40;

public:
  virtual void testRAMDirectorySize() ;

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<TestRAMDirectory> outerInstance;

    std::shared_ptr<IndexWriter> writer;
    int num = 0;

  public:
    ThreadAnonymousInnerClass(std::shared_ptr<TestRAMDirectory> outerInstance,
                              std::shared_ptr<IndexWriter> writer, int num);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

public:
  virtual void testShouldThrowEOFException() ;

protected:
  std::shared_ptr<TestRAMDirectory> shared_from_this()
  {
    return std::static_pointer_cast<TestRAMDirectory>(
        BaseDirectoryTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/store/
