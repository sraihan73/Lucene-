#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/IOContext.h"

#include  "core/src/java/org/apache/lucene/store/BaseDirectoryWrapper.h"
#include  "core/src/java/org/apache/lucene/store/IndexOutput.h"
#include  "core/src/java/org/apache/lucene/index/SegmentInfo.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/store/IndexInput.h"
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

using Document = org::apache::lucene::document::Document;
using Directory = org::apache::lucene::store::Directory;
using IndexInput = org::apache::lucene::store::IndexInput;

/**
 * Abstract class to do basic tests for a compound format.
 * NOTE: This test focuses on the compound impl, nothing else.
 * The [stretch] goal is for this test to be
 * so thorough in testing a new CompoundFormat that if this
 * test passes, then all Lucene/Solr tests should also pass.  Ie,
 * if there is some bug in a given CompoundFormat that this
 * test fails to catch then this test needs to be improved! */
class BaseCompoundFormatTestCase : public BaseIndexFileFormatTestCase
{
  GET_CLASS_NAME(BaseCompoundFormatTestCase)

  // test that empty CFS is empty
public:
  virtual void testEmpty() ;

  /**
   * This test creates compound file based on a single file.
   * Files of different sizes are tested: 0, 1, 10, 100 bytes.
   */
  virtual void testSingleFile() ;

  /**
   * This test creates compound file based on two files.
   */
  virtual void testTwoFiles() ;

  // test that a second call to close() behaves according to Closeable
  virtual void testDoubleClose() ;

  // LUCENE-5724: things like NRTCachingDir rely upon IOContext being properly
  // passed down
  virtual void testPassIOContext() ;

private:
  class FilterDirectoryAnonymousInnerClass : public FilterDirectory
  {
    GET_CLASS_NAME(FilterDirectoryAnonymousInnerClass)
  private:
    std::shared_ptr<BaseCompoundFormatTestCase> outerInstance;

    std::shared_ptr<IOContext> myContext;

  public:
    FilterDirectoryAnonymousInnerClass(
        std::shared_ptr<BaseCompoundFormatTestCase> outerInstance,
        std::shared_ptr<org::apache::lucene::store::BaseDirectoryWrapper>
            newDirectory,
        std::shared_ptr<IOContext> myContext);

    std::shared_ptr<IndexOutput> createOutput(
        const std::wstring &name,
        std::shared_ptr<IOContext> context)  override;

  protected:
    std::shared_ptr<FilterDirectoryAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FilterDirectoryAnonymousInnerClass>(
          org.apache.lucene.store.FilterDirectory::shared_from_this());
    }
  };

  // LUCENE-5724: actually test we play nice with NRTCachingDir and massive file
public:
  virtual void testLargeCFS() ;

  // Just tests that we can open all files returned by listAll
  virtual void testListAll() ;

  // test that cfs reader is read-only
  virtual void testCreateOutputDisabled() ;

  // test that cfs reader is read-only
  virtual void testDeleteFileDisabled() ;

  // test that cfs reader is read-only
  virtual void testRenameFileDisabled() ;

  // test that cfs reader is read-only
  virtual void testSyncDisabled() ;

  // test that cfs reader is read-only
  virtual void testMakeLockDisabled() ;

  /**
   * This test creates a compound file based on a large number of files of
   * various length. The file content is generated randomly. The sizes range
   * from 0 to 1Mb. Some of the sizes are selected to test the buffering
   * logic in the file reading code. For this the chunk variable is set to
   * the length of the buffer used internally by the compound file logic.
   */
  virtual void testRandomFiles() ;

  // Make sure we don't somehow use more than 1 descriptor
  // when reading a CFS with many subs:
  virtual void testManySubFiles() ;

  virtual void testClonedStreamsClosing() ;

  /** This test opens two files from a compound stream and verifies that
   *  their file positions are independent of each other.
   */
  virtual void testRandomAccess() ;

  /** This test opens two files from a compound stream and verifies that
   *  their file positions are independent of each other.
   */
  virtual void testRandomAccessClones() ;

  virtual void testFileNotFound() ;

  virtual void testReadPastEOF() ;

  /** Returns a new fake segment */
protected:
  static std::shared_ptr<SegmentInfo>
  newSegmentInfo(std::shared_ptr<Directory> dir, const std::wstring &name);

  /** Creates a file of the specified size with random data. */
  static void createRandomFile(std::shared_ptr<Directory> dir,
                               const std::wstring &name, int size,
                               std::deque<char> &segId) ;

  /** Creates a file of the specified size with sequential data. The first
   *  byte is written as the start byte provided. All subsequent bytes are
   *  computed as start + offset where offset is the number of the byte.
   */
  static void
  createSequenceFile(std::shared_ptr<Directory> dir, const std::wstring &name,
                     char start, int size, std::deque<char> &segID,
                     const std::wstring &segSuffix) ;

  static void
  assertSameStreams(const std::wstring &msg,
                    std::shared_ptr<IndexInput> expected,
                    std::shared_ptr<IndexInput> test) ;

  static void assertSameStreams(const std::wstring &msg,
                                std::shared_ptr<IndexInput> expected,
                                std::shared_ptr<IndexInput> actual,
                                int64_t seekTo) ;

  static void
  assertSameSeekBehavior(const std::wstring &msg,
                         std::shared_ptr<IndexInput> expected,
                         std::shared_ptr<IndexInput> actual) ;

  static void assertEqualArrays(const std::wstring &msg,
                                std::deque<char> &expected,
                                std::deque<char> &test, int start, int len);

  /**
   * Setup a large compound file with a number of components, each of
   * which is a sequential file (so that we can easily tell that we are
   * reading in the right byte). The methods sets up 20 files - _123.0 to
   * _123.19, the size of each file is 1000 bytes.
   */
  static std::shared_ptr<Directory>
  createLargeCFS(std::shared_ptr<Directory> dir) ;

  void addRandomFields(std::shared_ptr<Document> doc) override;

public:
  void testMergeStability()  override;

  // LUCENE-6311: make sure the resource name inside a compound file confesses
  // that it's inside a compound file
  virtual void testResourceNameInsideCompoundFile() ;

  virtual void testMissingCodecHeadersAreCaught() ;

  virtual void testCorruptFilesAreCaught() ;

protected:
  std::shared_ptr<BaseCompoundFormatTestCase> shared_from_this()
  {
    return std::static_pointer_cast<BaseCompoundFormatTestCase>(
        BaseIndexFileFormatTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
