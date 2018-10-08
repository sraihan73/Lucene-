#pragma once
#include "stringhelper.h"
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

#include  "core/src/java/org/apache/lucene/index/IndexWriter.h"
#include  "core/src/java/org/apache/lucene/index/SegmentInfos.h"
#include  "core/src/java/org/apache/lucene/util/InfoStream.h"
#include  "core/src/java/org/apache/lucene/store/MockDirectoryWrapper.h"

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
using InfoStream = org::apache::lucene::util::InfoStream;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/*
  Verify we can read the pre-2.1 file format, do searches
  against it, and add documents to it.
*/

class TestIndexFileDeleter : public LuceneTestCase
{
  GET_CLASS_NAME(TestIndexFileDeleter)

public:
  virtual void testDeleteLeftoverFiles() ;

private:
  static std::shared_ptr<Set<std::wstring>>
  difFiles(std::deque<std::wstring> &files1,
           std::deque<std::wstring> &files2);

  std::wstring asString(std::deque<std::wstring> &l);

public:
  virtual void copyFile(std::shared_ptr<Directory> dir, const std::wstring &src,
                        const std::wstring &dest) ;

private:
  void addDoc(std::shared_ptr<IndexWriter> writer, int id) ;

public:
  virtual void testVirusScannerDoesntCorruptIndex() ;

  virtual void testNoSegmentsDotGenInflation() ;

  virtual void testSegmentsInflation() ;

  virtual void testSegmentNameInflation() ;

  virtual void testGenerationInflation() ;

  virtual void testTrashyFile() ;

  virtual void testTrashyGenFile() ;

  // IFD's inflater is "raw" and expects to only see codec files,
  // and rightfully so, it filters them out.
  static void inflateGens(std::shared_ptr<SegmentInfos> sis,
                          std::shared_ptr<std::deque<std::wstring>> files,
                          std::shared_ptr<InfoStream> stream);

  // LUCENE-5919
  virtual void testExcInDecRef() ;

private:
  class FailureAnonymousInnerClass : public MockDirectoryWrapper::Failure
  {
    GET_CLASS_NAME(FailureAnonymousInnerClass)
  private:
    std::shared_ptr<TestIndexFileDeleter> outerInstance;

    std::shared_ptr<MockDirectoryWrapper> dir;
    std::shared_ptr<AtomicBoolean> doFailExc;

  public:
    FailureAnonymousInnerClass(
        std::shared_ptr<TestIndexFileDeleter> outerInstance,
        std::shared_ptr<MockDirectoryWrapper> dir,
        std::shared_ptr<AtomicBoolean> doFailExc);

    void
    eval(std::shared_ptr<MockDirectoryWrapper> dir)  override;

  protected:
    std::shared_ptr<FailureAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FailureAnonymousInnerClass>(
          org.apache.lucene.store.MockDirectoryWrapper
              .Failure::shared_from_this());
    }
  };

private:
  class ConcurrentMergeSchedulerAnonymousInnerClass
      : public ConcurrentMergeScheduler
  {
    GET_CLASS_NAME(ConcurrentMergeSchedulerAnonymousInnerClass)
  private:
    std::shared_ptr<TestIndexFileDeleter> outerInstance;

    std::shared_ptr<MockDirectoryWrapper> dir;

  public:
    ConcurrentMergeSchedulerAnonymousInnerClass(
        std::shared_ptr<TestIndexFileDeleter> outerInstance,
        std::shared_ptr<MockDirectoryWrapper> dir);

  protected:
    void handleMergeException(std::shared_ptr<Directory> dir,
                              std::runtime_error exc) override;

  protected:
    std::shared_ptr<ConcurrentMergeSchedulerAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          ConcurrentMergeSchedulerAnonymousInnerClass>(
          ConcurrentMergeScheduler::shared_from_this());
    }
  };

  // LUCENE-6835: make sure best-effort to not create an "apparently but not
  // really" corrupt index is working:
public:
  virtual void testExcInDeleteFile() ;

private:
  class FailureAnonymousInnerClass2 : public MockDirectoryWrapper::Failure
  {
    GET_CLASS_NAME(FailureAnonymousInnerClass2)
  private:
    std::shared_ptr<TestIndexFileDeleter> outerInstance;

    std::shared_ptr<MockDirectoryWrapper> dir;
    std::shared_ptr<AtomicBoolean> doFailExc;

  public:
    FailureAnonymousInnerClass2(
        std::shared_ptr<TestIndexFileDeleter> outerInstance,
        std::shared_ptr<MockDirectoryWrapper> dir,
        std::shared_ptr<AtomicBoolean> doFailExc);

    void
    eval(std::shared_ptr<MockDirectoryWrapper> dir)  override;

  protected:
    std::shared_ptr<FailureAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<FailureAnonymousInnerClass2>(
          org.apache.lucene.store.MockDirectoryWrapper
              .Failure::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestIndexFileDeleter> shared_from_this()
  {
    return std::static_pointer_cast<TestIndexFileDeleter>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
