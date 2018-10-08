#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>

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
namespace org::apache::lucene::mockfile
{

/** Basic tests for WindowsFS */
class TestWindowsFS : public MockFileSystemTestCase
{
  GET_CLASS_NAME(TestWindowsFS)

public:
  void setUp()  override;

protected:
  std::shared_ptr<Path> wrap(std::shared_ptr<Path> path) override;

  /** Test Files.delete fails if a file has an open inputstream against it */
public:
  virtual void testDeleteOpenFile() ;

  /** Test Files.deleteIfExists fails if a file has an open inputstream against
   * it */
  virtual void testDeleteIfExistsOpenFile() ;

  /** Test Files.rename fails if a file has an open inputstream against it */
  // TODO: what does windows do here?
  virtual void testRenameOpenFile() ;

  virtual void testOpenDeleteConcurrently() throw(IOException,
                                                  std::runtime_error);

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<TestWindowsFS> outerInstance;

    std::shared_ptr<Path> file;
    std::shared_ptr<CyclicBarrier> barrier;
    std::shared_ptr<AtomicBoolean> stopped;

  public:
    ThreadAnonymousInnerClass(std::shared_ptr<TestWindowsFS> outerInstance,
                              std::shared_ptr<Path> file,
                              std::shared_ptr<CyclicBarrier> barrier,
                              std::shared_ptr<AtomicBoolean> stopped);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

public:
  virtual void testMove() ;

protected:
  std::shared_ptr<TestWindowsFS> shared_from_this()
  {
    return std::static_pointer_cast<TestWindowsFS>(
        MockFileSystemTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/mockfile/
