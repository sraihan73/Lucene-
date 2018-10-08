#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

#include  "core/src/java/org/apache/lucene/util/OfflineSorter.h"
#include  "core/src/java/org/apache/lucene/util/SortInfo.h"
#include  "core/src/java/org/apache/lucene/store/IndexOutput.h"
#include  "core/src/java/org/apache/lucene/store/IOContext.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/util/BufferSize.h"
#include  "core/src/java/org/apache/lucene/store/ChecksumIndexInput.h"
#include  "core/src/java/org/apache/lucene/util/ByteSequencesReader.h"

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
namespace org::apache::lucene::util
{

using Directory = org::apache::lucene::store::Directory;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using SortInfo = org::apache::lucene::util::OfflineSorter::SortInfo;

/**
 * Tests for on-disk merge sorting.
 */
class TestOfflineSorter : public LuceneTestCase
{
  GET_CLASS_NAME(TestOfflineSorter)
private:
  std::shared_ptr<Path> tempDir;

public:
  void setUp()  override;

  void tearDown()  override;

  virtual void testEmpty() ;

  virtual void testSingleLine() ;

private:
  std::shared_ptr<ExecutorService> randomExecutorServiceOrNull();

public:
  virtual void testIntermediateMerges() ;

  virtual void testSmallRandom() ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Nightly public void testLargerRandom() throws Exception
  virtual void testLargerRandom() ;

private:
  std::deque<std::deque<char>> generateRandom(int howMuchDataInBytes);

  // Generates same data every time:
  std::deque<std::deque<char>> generateFixed(int howMuchDataInBytes);

public:
  static const std::shared_ptr<Comparator<std::deque<char>>>
      unsignedByteOrderComparator;

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<std::deque<char>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  public:
    ComparatorAnonymousInnerClass();

    int compare(std::deque<char> &left, std::deque<char> &right) override;
  };

  /**
   * Check sorting data on an instance of {@link OfflineSorter}.
   */
private:
  std::shared_ptr<SortInfo>
  checkSort(std::shared_ptr<Directory> dir,
            std::shared_ptr<OfflineSorter> sorter,
            std::deque<std::deque<char>> &data) ;

  /**
   * Make sure two files are byte-byte identical.
   */
  void assertFilesIdentical(std::shared_ptr<Directory> dir,
                            const std::wstring &golden,
                            const std::wstring &sorted) ;

  /** NOTE: closes the provided {@link IndexOutput} */
  void writeAll(std::shared_ptr<IndexOutput> out,
                std::deque<std::deque<char>> &data) ;

public:
  virtual void testRamBuffer();

  virtual void testThreadSafety() ;

private:
  class ThreadAnonymousInnerClass : public Thread
  {
    GET_CLASS_NAME(ThreadAnonymousInnerClass)
  private:
    std::shared_ptr<TestOfflineSorter> outerInstance;

    std::shared_ptr<AtomicBoolean> failed;
    int iters = 0;
    std::shared_ptr<Directory> dir;
    int threadID = 0;

  public:
    ThreadAnonymousInnerClass(std::shared_ptr<TestOfflineSorter> outerInstance,
                              std::shared_ptr<AtomicBoolean> failed, int iters,
                              std::shared_ptr<Directory> dir, int threadID);

    void run() override;

  protected:
    std::shared_ptr<ThreadAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ThreadAnonymousInnerClass>(
          Thread::shared_from_this());
    }
  };

  /** Make sure corruption on the incoming (unsorted) file is caught, even if
   * the corruption didn't confuse OfflineSorter! */
public:
  virtual void testBitFlippedOnInput1() ;

private:
  class FilterDirectoryAnonymousInnerClass : public FilterDirectory
  {
    GET_CLASS_NAME(FilterDirectoryAnonymousInnerClass)
  private:
    std::shared_ptr<TestOfflineSorter> outerInstance;

    std::shared_ptr<Directory> dir0;

  public:
    FilterDirectoryAnonymousInnerClass(
        std::shared_ptr<TestOfflineSorter> outerInstance,
        std::shared_ptr<Directory> dir0);

    std::shared_ptr<IndexOutput> createTempOutput(
        const std::wstring &prefix, const std::wstring &suffix,
        std::shared_ptr<IOContext> context)  override;

  protected:
    std::shared_ptr<FilterDirectoryAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FilterDirectoryAnonymousInnerClass>(
          org.apache.lucene.store.FilterDirectory::shared_from_this());
    }
  };

  /** Make sure corruption on the incoming (unsorted) file is caught, if the
   * corruption did confuse OfflineSorter! */
public:
  virtual void testBitFlippedOnInput2() ;

private:
  class FilterDirectoryAnonymousInnerClass2 : public FilterDirectory
  {
    GET_CLASS_NAME(FilterDirectoryAnonymousInnerClass2)
  private:
    std::shared_ptr<TestOfflineSorter> outerInstance;

    std::shared_ptr<Directory> dir0;

  public:
    FilterDirectoryAnonymousInnerClass2(
        std::shared_ptr<TestOfflineSorter> outerInstance,
        std::shared_ptr<Directory> dir0);

    std::shared_ptr<IndexOutput> createTempOutput(
        const std::wstring &prefix, const std::wstring &suffix,
        std::shared_ptr<IOContext> context)  override;

  private:
    class CorruptingIndexOutputAnonymousInnerClass
        : public CorruptingIndexOutput
    {
      GET_CLASS_NAME(CorruptingIndexOutputAnonymousInnerClass)
    private:
      std::shared_ptr<FilterDirectoryAnonymousInnerClass2> outerInstance;

      std::shared_ptr<IndexOutput> out;

    public:
      CorruptingIndexOutputAnonymousInnerClass(
          std::shared_ptr<FilterDirectoryAnonymousInnerClass2> outerInstance,
          std::shared_ptr<Directory> dir0, std::shared_ptr<IndexOutput> out);

    protected:
      void corruptFile()  override;

    protected:
      std::shared_ptr<CorruptingIndexOutputAnonymousInnerClass>
      shared_from_this()
      {
        return std::static_pointer_cast<
            CorruptingIndexOutputAnonymousInnerClass>(
            org.apache.lucene.store.CorruptingIndexOutput::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<FilterDirectoryAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<FilterDirectoryAnonymousInnerClass2>(
          org.apache.lucene.store.FilterDirectory::shared_from_this());
    }
  };

  /** Make sure corruption on a temp file (partition) is caught, even if the
   * corruption didn't confuse OfflineSorter! */
public:
  virtual void testBitFlippedOnPartition1() ;

private:
  class FilterDirectoryAnonymousInnerClass3 : public FilterDirectory
  {
    GET_CLASS_NAME(FilterDirectoryAnonymousInnerClass3)
  private:
    std::shared_ptr<TestOfflineSorter> outerInstance;

    std::shared_ptr<Directory> dir0;

  public:
    FilterDirectoryAnonymousInnerClass3(
        std::shared_ptr<TestOfflineSorter> outerInstance,
        std::shared_ptr<Directory> dir0);

    bool corrupted = false;

    std::shared_ptr<IndexOutput> createTempOutput(
        const std::wstring &prefix, const std::wstring &suffix,
        std::shared_ptr<IOContext> context)  override;

  protected:
    std::shared_ptr<FilterDirectoryAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<FilterDirectoryAnonymousInnerClass3>(
          org.apache.lucene.store.FilterDirectory::shared_from_this());
    }
  };

  /** Make sure corruption on a temp file (partition) is caught, if the
   * corruption did confuse OfflineSorter! */
public:
  virtual void testBitFlippedOnPartition2() ;

private:
  class FilterDirectoryAnonymousInnerClass4 : public FilterDirectory
  {
    GET_CLASS_NAME(FilterDirectoryAnonymousInnerClass4)
  private:
    std::shared_ptr<TestOfflineSorter> outerInstance;

    std::shared_ptr<Directory> dir0;

  public:
    FilterDirectoryAnonymousInnerClass4(
        std::shared_ptr<TestOfflineSorter> outerInstance,
        std::shared_ptr<Directory> dir0);

    bool corrupted = false;

    std::shared_ptr<IndexOutput> createTempOutput(
        const std::wstring &prefix, const std::wstring &suffix,
        std::shared_ptr<IOContext> context)  override;

  private:
    class CorruptingIndexOutputAnonymousInnerClass2
        : public CorruptingIndexOutput
    {
      GET_CLASS_NAME(CorruptingIndexOutputAnonymousInnerClass2)
    private:
      std::shared_ptr<FilterDirectoryAnonymousInnerClass4> outerInstance;

      std::shared_ptr<IndexOutput> out;

    public:
      CorruptingIndexOutputAnonymousInnerClass2(
          std::shared_ptr<FilterDirectoryAnonymousInnerClass4> outerInstance,
          std::shared_ptr<Directory> dir0, std::shared_ptr<IndexOutput> out);

    protected:
      void corruptFile()  override;

    protected:
      std::shared_ptr<CorruptingIndexOutputAnonymousInnerClass2>
      shared_from_this()
      {
        return std::static_pointer_cast<
            CorruptingIndexOutputAnonymousInnerClass2>(
            org.apache.lucene.store.CorruptingIndexOutput::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<FilterDirectoryAnonymousInnerClass4> shared_from_this()
    {
      return std::static_pointer_cast<FilterDirectoryAnonymousInnerClass4>(
          org.apache.lucene.store.FilterDirectory::shared_from_this());
    }
  };

public:
  virtual void testFixedLengthHeap() ;

  virtual void testFixedLengthLiesLiesLies() ;

  // OfflineSorter should not call my BytesSequencesReader.next() again after it
  // already returned null:
  virtual void testOverNexting() ;

private:
  class OfflineSorterAnonymousInnerClass : public OfflineSorter
  {
    GET_CLASS_NAME(OfflineSorterAnonymousInnerClass)
  private:
    std::shared_ptr<TestOfflineSorter> outerInstance;

  public:
    OfflineSorterAnonymousInnerClass(
        std::shared_ptr<TestOfflineSorter> outerInstance,
        std::shared_ptr<Directory> dir,
        std::shared_ptr<Comparator<std::shared_ptr<BytesRef>>>
            DEFAULT_COMPARATOR,
        std::shared_ptr<BufferSize> megabytes,
        std::shared_ptr<UnknownType> BYTES);

  protected:
    std::shared_ptr<ByteSequencesReader>
    getReader(std::shared_ptr<ChecksumIndexInput> in_,
              const std::wstring &name)  override;

  private:
    class ByteSequencesReaderAnonymousInnerClass : public ByteSequencesReader
    {
      GET_CLASS_NAME(ByteSequencesReaderAnonymousInnerClass)
    private:
      std::shared_ptr<OfflineSorterAnonymousInnerClass> outerInstance;

      std::shared_ptr<
          org::apache::lucene::util::OfflineSorter::ByteSequencesReader>
          other;

    public:
      ByteSequencesReaderAnonymousInnerClass(
          std::shared_ptr<OfflineSorterAnonymousInnerClass> outerInstance,
          std::shared_ptr<ChecksumIndexInput> in_, const std::wstring &name,
          std::shared_ptr<
              org::apache::lucene::util::OfflineSorter::ByteSequencesReader>
              other);

    private:
      bool alreadyEnded = false;

    public:
      std::shared_ptr<BytesRef> next()  override;

      virtual ~ByteSequencesReaderAnonymousInnerClass();

    protected:
      std::shared_ptr<ByteSequencesReaderAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<ByteSequencesReaderAnonymousInnerClass>(
            ByteSequencesReader::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<OfflineSorterAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<OfflineSorterAnonymousInnerClass>(
          OfflineSorter::shared_from_this());
    }
  };

public:
  virtual void testInvalidFixedLength() ;

protected:
  std::shared_ptr<TestOfflineSorter> shared_from_this()
  {
    return std::static_pointer_cast<TestOfflineSorter>(
        LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/
