#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <deque>

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

/**
 * Runs TestNRTThreads in a separate process, crashes the JRE in the middle
 * of execution, then runs checkindex to make sure it's not corrupt.
 */
class TestIndexWriterOnJRECrash : public TestNRTThreads
{
  GET_CLASS_NAME(TestIndexWriterOnJRECrash)
private:
  std::shared_ptr<Path> tempDir;

public:
  void setUp()  override;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Override @Nightly public void testNRTThreads() throws
  // Exception
  void testNRTThreads()  override;

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<TestIndexWriterOnJRECrash> outerInstance;

    int crashTime = 0;

  public:
    ThreadAnonymousInnerClass(
        std::shared_ptr<TestIndexWriterOnJRECrash> outerInstance,
        int crashTime);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

  /** fork ourselves in a new jvm. sets -Dtests.crashmode=true */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressForbidden(reason = "ProcessBuilder requires
  // java.io.File for CWD") public void forkTest() throws Exception
  virtual void forkTest() ;

  /** A pipe thread. It'd be nice to reuse guava's implementation for this... */
public:
  class ThreadPumper : public std::enable_shared_from_this<ThreadPumper>
  {
    GET_CLASS_NAME(ThreadPumper)
  public:
    static std::shared_ptr<Thread> start(std::shared_ptr<InputStream> from,
                                         std::shared_ptr<OutputStream> to);

  private:
    class ThreadAnonymousInnerClass2 : public Thread
    {
      GET_CLASS_NAME(ThreadAnonymousInnerClass2)
    private:
      std::shared_ptr<InputStream> from;
      std::shared_ptr<OutputStream> to;

    public:
      ThreadAnonymousInnerClass2(std::shared_ptr<InputStream> from,
                                 std::shared_ptr<OutputStream> to);

      void run() override;

    protected:
      std::shared_ptr<ThreadAnonymousInnerClass2> shared_from_this()
      {
        return std::static_pointer_cast<ThreadAnonymousInnerClass2>(
            Thread::shared_from_this());
      }
    };
  };

  /**
   * Recursively looks for indexes underneath <code>file</code>,
   * and runs checkindex on them. returns true if it found any indexes.
   */
public:
  virtual bool checkIndexes(std::shared_ptr<Path> path) ;

private:
  class SimpleFileVisitorAnonymousInnerClass
      : public SimpleFileVisitor<std::shared_ptr<Path>>
  {
    GET_CLASS_NAME(SimpleFileVisitorAnonymousInnerClass)
  private:
    std::shared_ptr<TestIndexWriterOnJRECrash> outerInstance;

    std::shared_ptr<AtomicBoolean> found;

  public:
    SimpleFileVisitorAnonymousInnerClass(
        std::shared_ptr<TestIndexWriterOnJRECrash> outerInstance,
        std::shared_ptr<AtomicBoolean> found);

    std::shared_ptr<FileVisitResult> postVisitDirectory(
        std::shared_ptr<Path> dirPath,
        std::shared_ptr<IOException> exc)  override;

  protected:
    std::shared_ptr<SimpleFileVisitorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SimpleFileVisitorAnonymousInnerClass>(
          java.nio.file
              .SimpleFileVisitor<java.nio.file.Path>::shared_from_this());
    }
  };

  /**
   * currently, this only works/tested on Sun and IBM.
   */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressForbidden(reason = "We need Unsafe to actually
  // crush :-)") public void crashJRE()
  virtual void crashJRE();

protected:
  std::shared_ptr<TestIndexWriterOnJRECrash> shared_from_this()
  {
    return std::static_pointer_cast<TestIndexWriterOnJRECrash>(
        TestNRTThreads::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
