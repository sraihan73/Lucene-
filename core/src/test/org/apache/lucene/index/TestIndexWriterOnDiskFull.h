#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/MockDirectoryWrapper.h"

#include  "core/src/java/org/apache/lucene/index/LogMergePolicy.h"
#include  "core/src/java/org/apache/lucene/index/CodecReader.h"
namespace org::apache::lucene::util
{
template <typename T>
class IOSupplier;
}
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

using MockDirectoryWrapper = org::apache::lucene::store::MockDirectoryWrapper;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Tests for IndexWriter when the disk runs out of space
 */
class TestIndexWriterOnDiskFull : public LuceneTestCase
{
  GET_CLASS_NAME(TestIndexWriterOnDiskFull)

  /*
   * Make sure IndexWriter cleans up on hitting a disk
   * full exception in addDocument.
   * TODO: how to do this on windows with FSDirectory?
   */
public:
  virtual void testAddDocumentOnDiskFull() ;

  // TODO: make @Nightly variant that provokes more disk
  // fulls

  // TODO: have test fail if on any given top
  // iter there was not a single IOE hit

  /*
  Test: make sure when we run out of disk space or hit
  random IOExceptions in any of the addIndexes(*) calls
  that 1) index is not corrupt (searcher can open/search
  it) and 2) transactional semantics are followed:
  either all or none of the incoming documents were in
  fact added.
   */
  virtual void testAddIndexOnDiskFull() ;

private:
  class FailTwiceDuringMerge : public MockDirectoryWrapper::Failure
  {
    GET_CLASS_NAME(FailTwiceDuringMerge)
  public:
    bool didFail1 = false;
    bool didFail2 = false;

    void
    eval(std::shared_ptr<MockDirectoryWrapper> dir)  override;

  protected:
    std::shared_ptr<FailTwiceDuringMerge> shared_from_this()
    {
      return std::static_pointer_cast<FailTwiceDuringMerge>(
          org.apache.lucene.store.MockDirectoryWrapper
              .Failure::shared_from_this());
    }
  };

  // LUCENE-2593
public:
  virtual void testCorruptionAfterDiskFullDuringMerge() ;

private:
  class FilterMergePolicyAnonymousInnerClass : public FilterMergePolicy
  {
    GET_CLASS_NAME(FilterMergePolicyAnonymousInnerClass)
  private:
    std::shared_ptr<TestIndexWriterOnDiskFull> outerInstance;

  public:
    FilterMergePolicyAnonymousInnerClass(
        std::shared_ptr<TestIndexWriterOnDiskFull> outerInstance,
        std::shared_ptr<org::apache::lucene::index::LogMergePolicy>
            newLogMergePolicy);

    bool
    keepFullyDeletedSegment(IOSupplier<std::shared_ptr<CodecReader>>
                                readerIOSupplier)  override;

  protected:
    std::shared_ptr<FilterMergePolicyAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FilterMergePolicyAnonymousInnerClass>(
          FilterMergePolicy::shared_from_this());
    }
  };

  // LUCENE-1130: make sure immeidate disk full on creating
  // an IndexWriter (hit during DW.ThreadState.init()) is
  // OK:
public:
  virtual void testImmediateDiskFull() ;

  // TODO: these are also in TestIndexWriter... add a simple doc-writing method
  // like this to LuceneTestCase?
private:
  void addDoc(std::shared_ptr<IndexWriter> writer) ;

  void addDocWithIndex(std::shared_ptr<IndexWriter> writer,
                       int index) ;

protected:
  std::shared_ptr<TestIndexWriterOnDiskFull> shared_from_this()
  {
    return std::static_pointer_cast<TestIndexWriterOnDiskFull>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
