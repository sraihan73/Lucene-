#pragma once
#include "stringhelper.h"
#include <iostream>
#include <deque>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/IndexWriter.h"

#include  "core/src/java/org/apache/lucene/index/SegmentCommitInfo.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"

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

/** JUnit adaptation of an older test case DocTest. */
class TestDoc : public LuceneTestCase
{
  GET_CLASS_NAME(TestDoc)

private:
  std::shared_ptr<Path> workDir;
  std::shared_ptr<Path> indexDir;
  std::deque<std::shared_ptr<Path>> files;

  /** Set the test case. This test case needs
   *  a few text files created in the current working directory.
   */
public:
  void setUp()  override;

private:
  std::shared_ptr<Path>
  createOutput(const std::wstring &name,
               const std::wstring &text) ;

  /** This test executes a number of merges and compares the contents of
   *  the segments created when using compound file or not using one.
   *
   *  TODO: the original test used to print the segment contents to System.out
   *        for visual validation. To have the same effect, a new method
   *        checkSegment(std::wstring name, ...) should be created that would
   *        assert various things about the segment.
   */
public:
  virtual void testIndexAndMerge() ;

private:
  std::shared_ptr<SegmentCommitInfo>
  indexDoc(std::shared_ptr<IndexWriter> writer,
           const std::wstring &fileName) ;

  std::shared_ptr<SegmentCommitInfo>
  merge(std::shared_ptr<Directory> dir, std::shared_ptr<SegmentCommitInfo> si1,
        std::shared_ptr<SegmentCommitInfo> si2, const std::wstring &merged,
        bool useCompoundFile) ;

  void
  printSegment(std::shared_ptr<PrintWriter> out,
               std::shared_ptr<SegmentCommitInfo> si) ;

protected:
  std::shared_ptr<TestDoc> shared_from_this()
  {
    return std::static_pointer_cast<TestDoc>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
