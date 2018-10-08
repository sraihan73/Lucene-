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
namespace org::apache::lucene::store
{
class Directory;
}

namespace org::apache::lucene::store
{
class IndexInput;
}
namespace org::apache::lucene::store
{
class IndexOutput;
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
namespace org::apache::lucene::store
{

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/** Base class for per-Directory tests. */

class BaseDirectoryTestCase : public LuceneTestCase
{
  GET_CLASS_NAME(BaseDirectoryTestCase)

  /** Subclass returns the Directory to be tested; if it's
   *  an FS-based directory it should point to the specified
   *  path, else it can ignore it. */
protected:
  virtual std::shared_ptr<Directory>
  getDirectory(std::shared_ptr<Path> path) = 0;

  // first some basic tests for the directory api

public:
  virtual void testCopyFrom() ;

  virtual void testCopyFromDestination() ;

  virtual void testRename() ;

  virtual void testDeleteFile() ;

  virtual void testByte() ;

  virtual void testShort() ;

  virtual void testInt() ;

  virtual void testLong() ;

  virtual void testString() ;

  virtual void testVInt() ;

  virtual void testVLong() ;

  virtual void testZInt() ;

  virtual void testZLong() ;

  virtual void testSetOfStrings() ;

  virtual void testMapOfStrings() ;

  // TODO: fold in some of the testing of o.a.l.index.TestIndexInput in here!
  virtual void testChecksum() ;

  /** Make sure directory throws AlreadyClosedException if
   *  you try to createOutput after closing. */
  virtual void testDetectClose() ;

  virtual void testThreadSafety() ;

  /** LUCENE-1468: once we create an output, we should see
   *  it in the dir listing and be able to open it with
   *  openInput. */
  virtual void testDirectoryFilter() ;

  // LUCENE-2852
  virtual void testSeekToEOFThenBack() ;

  // LUCENE-1196
  virtual void testIllegalEOF() ;

  virtual void testSeekPastEOF() ;

  virtual void testSliceOutOfBounds() ;

  // LUCENE-3382 -- make sure we get exception if the directory really does not
  // exist.
  virtual void testNoDir() ;

  virtual void testCopyBytes() ;

private:
  static char value(int idx);

public:
  static void
  testCopyBytes(std::shared_ptr<Directory> dir) ;

  // LUCENE-3541
  virtual void testCopyBytesWithThreads() ;

  static void testCopyBytesWithThreads(std::shared_ptr<Directory> d) throw(
      std::runtime_error);

public:
  class CopyThread : public Thread
  {
    GET_CLASS_NAME(CopyThread)
  public:
    const std::shared_ptr<IndexInput> src;
    const std::shared_ptr<IndexOutput> dst;

    CopyThread(std::shared_ptr<IndexInput> src,
               std::shared_ptr<IndexOutput> dst);

    void run() override;

  protected:
    std::shared_ptr<CopyThread> shared_from_this()
    {
      return std::static_pointer_cast<CopyThread>(Thread::shared_from_this());
    }
  };

  // this test backdoors the directory via the filesystem. so it must actually
  // use the filesystem
  // TODO: somehow change this test to
public:
  virtual void testFsyncDoesntCreateNewFiles() ;

  // random access APIs

  virtual void testRandomLong() ;

  virtual void testRandomInt() ;

  virtual void testRandomShort() ;

  virtual void testRandomByte() ;

  /** try to stress slices of slices */
  virtual void testSliceOfSlice() ;

  /**
   * This test that writes larger than the size of the buffer output
   * will correctly increment the file pointer.
   */
  virtual void testLargeWrites() ;

  // LUCENE-6084
  virtual void testIndexOutputToString() ;

  virtual void testDoubleCloseDirectory() ;

  virtual void testDoubleCloseOutput() ;

  virtual void testDoubleCloseInput() ;

  virtual void testCreateTempOutput() ;

  virtual void testSeekToEndOfFile() ;

  virtual void testSeekBeyondEndOfFile() ;

  // Make sure the FSDirectory impl properly "emulates" deletions on filesystems
  // (Windows) with buggy deleteFile:
  virtual void testPendingDeletions() ;

  virtual void testListAllIsSorted() ;

protected:
  std::shared_ptr<BaseDirectoryTestCase> shared_from_this()
  {
    return std::static_pointer_cast<BaseDirectoryTestCase>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::store
