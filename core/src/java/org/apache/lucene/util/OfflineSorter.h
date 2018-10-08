#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>
#include <queue>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/util/ByteSequencesReader.h"
#include  "core/src/java/org/apache/lucene/util/Partition.h"
#include  "core/src/java/org/apache/lucene/util/SortableBytesRefArray.h"
#include  "core/src/java/org/apache/lucene/store/IndexOutput.h"
#include  "core/src/java/org/apache/lucene/util/ByteSequencesWriter.h"
#include  "core/src/java/org/apache/lucene/store/ChecksumIndexInput.h"
#include  "core/src/java/org/apache/lucene/util/BytesRefBuilder.h"

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

using ChecksumIndexInput = org::apache::lucene::store::ChecksumIndexInput;
using Directory = org::apache::lucene::store::Directory;
using IndexOutput = org::apache::lucene::store::IndexOutput;

/**
 * On-disk sorting of byte arrays. Each byte array (entry) is a composed of the
 * following fields: <ul> <li>(two bytes) length of the following byte array,
 *   <li>exactly the above count of bytes for the sequence to be sorted.
 * </ul>
 *
 * @see #sort(std::wstring)
 * @lucene.experimental
 * @lucene.internal
 */
class OfflineSorter : public std::enable_shared_from_this<OfflineSorter>
{
  GET_CLASS_NAME(OfflineSorter)

  /** Convenience constant for megabytes */
public:
  static constexpr int64_t MB = 1024 * 1024;
  /** Convenience constant for gigabytes */
  static const int64_t GB = MB * 1024;

  /**
   * Minimum recommended buffer size for sorting.
   */
  static constexpr int64_t MIN_BUFFER_SIZE_MB = 32;

  /**
   * Absolute minimum required buffer size for sorting.
   */
  static const int64_t ABSOLUTE_MIN_SORT_BUFFER_SIZE = MB / 2;

private:
  static const std::wstring MIN_BUFFER_SIZE_MSG;

  /**
   * Maximum number of temporary files before doing an intermediate merge.
   */
public:
  static constexpr int MAX_TEMPFILES = 10;

private:
  const std::shared_ptr<Directory> dir;
  const int valueLength;
  const std::wstring tempFileNamePrefix;

  const std::shared_ptr<ExecutorService> exec;
  const std::shared_ptr<Semaphore> partitionsInRAM;

  /**
   * A bit more descriptive unit for constructors.
   *
   * @see #automatic()
   * @see #megabytes(long)
   */
public:
  class BufferSize final : public std::enable_shared_from_this<BufferSize>
  {
    GET_CLASS_NAME(BufferSize)
  public:
    const int bytes;

  private:
    BufferSize(int64_t bytes);

    /**
     * Creates a {@link BufferSize} in MB. The given
     * values must be &gt; 0 and &lt; 2048.
     */
  public:
    static std::shared_ptr<BufferSize> megabytes(int64_t mb);

    /**
     * Approximately half of the currently available free heap, but no less
     * than {@link #ABSOLUTE_MIN_SORT_BUFFER_SIZE}. However if current heap
     * allocation is insufficient or if there is a large portion of unallocated
     * heap-space available for sorting consult with max allowed heap size.
     */
    static std::shared_ptr<BufferSize> automatic();
  };

  /**
   * Sort info (debugging mostly).
   */
public:
  class SortInfo : public std::enable_shared_from_this<SortInfo>
  {
    GET_CLASS_NAME(SortInfo)
  private:
    std::shared_ptr<OfflineSorter> outerInstance;

    /** number of temporary files created when merging partitions */
  public:
    int tempMergeFiles = 0;
    /** number of partition merges */
    int mergeRounds = 0;
    /** number of lines of data read */
    int lineCount = 0;
    /** time spent merging sorted partitions (in milliseconds) */
    const std::shared_ptr<AtomicLong> mergeTimeMS =
        std::make_shared<AtomicLong>();
    /** time spent sorting data (in milliseconds) */
    const std::shared_ptr<AtomicLong> sortTimeMS =
        std::make_shared<AtomicLong>();
    /** total time spent (in milliseconds) */
    int64_t totalTimeMS = 0;
    /** time spent in i/o read (in milliseconds) */
    int64_t readTimeMS = 0;
    /** read buffer size (in bytes) */
    const int64_t bufferSize = outerInstance->ramBufferSize->bytes;

    /** create a new SortInfo (with empty statistics) for debugging */
    SortInfo(std::shared_ptr<OfflineSorter> outerInstance);

    virtual std::wstring toString();
  };

private:
  const std::shared_ptr<BufferSize> ramBufferSize;

public:
  std::shared_ptr<SortInfo> sortInfo;

private:
  int maxTempFiles = 0;
  const std::shared_ptr<Comparator<std::shared_ptr<BytesRef>>> comparator;

  /** Default comparator: sorts in binary (codepoint) order */
public:
  static const std::shared_ptr<Comparator<std::shared_ptr<BytesRef>>>
      DEFAULT_COMPARATOR;

  /**
   * Defaults constructor.
   *
   * @see BufferSize#automatic()
   */
  OfflineSorter(std::shared_ptr<Directory> dir,
                const std::wstring &tempFileNamePrefix) ;

  /**
   * Defaults constructor with a custom comparator.
   *
   * @see BufferSize#automatic()
   */
  OfflineSorter(std::shared_ptr<Directory> dir,
                const std::wstring &tempFileNamePrefix,
                std::shared_ptr<Comparator<std::shared_ptr<BytesRef>>>
                    comparator) ;

  /**
   * All-details constructor.  If {@code valueLength} is -1 (the default), the
length of each value differs; otherwise,
   * all values have the specified length.  If you pass a non-null {@code
ExecutorService} then it will be
   * used to run sorting operations that can be run concurrently, and
maxPartitionsInRAM is the maximum
   * concurrent in-memory partitions.  Thus the maximum possible RAM used by
this class while sorting is
   * {@code maxPartitionsInRAM * ramBufferSize}.
GET_CLASS_NAME(while)
   */
  OfflineSorter(
      std::shared_ptr<Directory> dir, const std::wstring &tempFileNamePrefix,
      std::shared_ptr<Comparator<std::shared_ptr<BytesRef>>> comparator,
      std::shared_ptr<BufferSize> ramBufferSize, int maxTempfiles,
      int valueLength, std::shared_ptr<ExecutorService> exec,
      int maxPartitionsInRAM);

  /** Returns the {@link Directory} we use to create temp files. */
  virtual std::shared_ptr<Directory> getDirectory();

  /** Returns the temp file name prefix passed to {@link
   * Directory#createTempOutput} to generate temporary files. */
  virtual std::wstring getTempFileNamePrefix();

  /**
   * Sort input to a new temp file, returning its name.
   */
  virtual std::wstring
  sort(const std::wstring &inputFileName) ;

  /** Called on exception, to check whether the checksum is also corrupt in this
   * source, and add that information (checksum matched or didn't) as a
   * suppressed exception. */
private:
  void verifyChecksum(
      std::runtime_error priorException,
      std::shared_ptr<ByteSequencesReader> reader) ;

  /** Merge the most recent {@code maxTempFile} partitions into a new partition.
   */
public:
  virtual void mergePartitions(std::shared_ptr<Directory> trackingDir,
                               std::deque<Future<std::shared_ptr<Partition>>>
                                   &segments) ;

  /** Holds one partition of items, either loaded into memory or based on a
   * file. */
private:
  class Partition : public std::enable_shared_from_this<Partition>
  {
    GET_CLASS_NAME(Partition)
  public:
    const std::shared_ptr<SortableBytesRefArray> buffer;
    const bool exhausted;
    const int64_t count;
    const std::wstring fileName;

    /** A partition loaded into memory. */
    Partition(std::shared_ptr<SortableBytesRefArray> buffer, bool exhausted);

    /** An on-disk partition. */
    Partition(const std::wstring &fileName, int64_t count);
  };

  /** Read in a single partition of data, setting isExhausted[0] to true if
   * there are no more items. */
public:
  virtual std::shared_ptr<Partition> readPartition(
      std::shared_ptr<ByteSequencesReader> reader) throw(IOException,
                                                         InterruptedException);

public:
  class FileAndTop : public std::enable_shared_from_this<FileAndTop>
  {
    GET_CLASS_NAME(FileAndTop)
  public:
    const int fd;
    std::shared_ptr<BytesRef> current;

    FileAndTop(int fd, std::shared_ptr<BytesRef> firstLine);
  };

  /** Subclasses can override to change how byte sequences are written to disk.
   */
protected:
  virtual std::shared_ptr<ByteSequencesWriter>
  getWriter(std::shared_ptr<IndexOutput> out,
            int64_t itemCount) ;

  /** Subclasses can override to change how byte sequences are read from disk.
   */
  virtual std::shared_ptr<ByteSequencesReader>
  getReader(std::shared_ptr<ChecksumIndexInput> in_,
            const std::wstring &name) ;

  /**
   * Utility class to emit length-prefixed byte[] entries to an output stream
for sorting.
   * Complementary to {@link ByteSequencesReader}.  You must use {@link
CodecUtil#writeFooter} GET_CLASS_NAME(to)
   * to write a footer at the end of the input file.
   */
public:
  class ByteSequencesWriter
      : public std::enable_shared_from_this<ByteSequencesWriter>
  {
    GET_CLASS_NAME(ByteSequencesWriter)
  protected:
    const std::shared_ptr<IndexOutput> out;

    // TODO: this should optimize the fixed width case as well

    /** Constructs a ByteSequencesWriter to the provided DataOutput */
  public:
    ByteSequencesWriter(std::shared_ptr<IndexOutput> out);

    /**
     * Writes a BytesRef.
     * @see #write(byte[], int, int)
     */
    void write(std::shared_ptr<BytesRef> ref) ;

    /**
     * Writes a byte array.
     * @see #write(byte[], int, int)
     */
    void write(std::deque<char> &bytes) ;

    /**
     * Writes a byte array.
     * <p>
     * The length is written as a <code>short</code>, followed
     * by the bytes.
     */
    virtual void write(std::deque<char> &bytes, int off,
                       int len) ;

    /**
     * Closes the provided {@link IndexOutput}.
     */
    virtual ~ByteSequencesWriter();
  };

  /**
   * Utility class to read length-prefixed byte[] entries from an input.
   * Complementary to {@link ByteSequencesWriter}.
GET_CLASS_NAME(to)
   */
public:
  class ByteSequencesReader
      : public std::enable_shared_from_this<ByteSequencesReader>,
        public BytesRefIterator
  {
    GET_CLASS_NAME(ByteSequencesReader)
  protected:
    const std::wstring name;
    const std::shared_ptr<ChecksumIndexInput> in_;
    const int64_t end;

  private:
    const std::shared_ptr<BytesRefBuilder> ref =
        std::make_shared<BytesRefBuilder>();

    /** Constructs a ByteSequencesReader from the provided IndexInput */
  public:
    ByteSequencesReader(std::shared_ptr<ChecksumIndexInput> in_,
                        const std::wstring &name);

    /**
     * Reads the next entry into the provided {@link BytesRef}. The internal
     * storage is resized if needed.
     *
     * @return Returns <code>false</code> if EOF occurred when trying to read
     * the header of the next sequence. Returns <code>true</code> otherwise.
     * @throws EOFException if the file ends before the full sequence is read.
     */
    std::shared_ptr<BytesRef> next()  override;

    /**
     * Closes the provided {@link IndexInput}.
     */
    virtual ~ByteSequencesReader();
  };

  /** Returns the comparator in use to sort entries */
public:
  virtual std::shared_ptr<Comparator<std::shared_ptr<BytesRef>>>
  getComparator();

  /** Sorts one in-memory partition, writes it to disk, and returns the
   * resulting file-based partition. */
private:
  class SortPartitionTask
      : public std::enable_shared_from_this<SortPartitionTask>,
        public Callable<std::shared_ptr<Partition>>
  {
    GET_CLASS_NAME(SortPartitionTask)
  private:
    std::shared_ptr<OfflineSorter> outerInstance;

    const std::shared_ptr<Directory> dir;
    const std::shared_ptr<Partition> part;

  public:
    SortPartitionTask(std::shared_ptr<OfflineSorter> outerInstance,
                      std::shared_ptr<Directory> dir,
                      std::shared_ptr<Partition> part);

    std::shared_ptr<Partition> call()  override;
  };

private:
  std::shared_ptr<Partition>
  getPartition(std::shared_ptr<Future<std::shared_ptr<Partition>>>
                   future) ;

  /** Merges multiple file-based partitions to a single on-disk partition. */
private:
  class MergePartitionsTask
      : public std::enable_shared_from_this<MergePartitionsTask>,
        public Callable<std::shared_ptr<Partition>>
  {
    GET_CLASS_NAME(MergePartitionsTask)
  private:
    std::shared_ptr<OfflineSorter> outerInstance;

    const std::shared_ptr<Directory> dir;
    const std::deque<Future<std::shared_ptr<Partition>>> segmentsToMerge;

  public:
    MergePartitionsTask(
        std::shared_ptr<OfflineSorter> outerInstance,
        std::shared_ptr<Directory> dir,
        std::deque<Future<std::shared_ptr<Partition>>> &segmentsToMerge);

    std::shared_ptr<Partition> call()  override;

  private:
    class PriorityQueueAnonymousInnerClass
        : public PriorityQueue<std::shared_ptr<FileAndTop>>
    {
      GET_CLASS_NAME(PriorityQueueAnonymousInnerClass)
    private:
      std::shared_ptr<MergePartitionsTask> outerInstance;

    public:
      PriorityQueueAnonymousInnerClass(
          std::shared_ptr<MergePartitionsTask> outerInstance,
          std::shared_ptr<UnknownType> size);

    protected:
      bool lessThan(std::shared_ptr<FileAndTop> a,
                    std::shared_ptr<FileAndTop> b) override;

    protected:
      std::shared_ptr<PriorityQueueAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<PriorityQueueAnonymousInnerClass>(
            PriorityQueue<FileAndTop>::shared_from_this());
      }
    };
  };
};

} // #include  "core/src/java/org/apache/lucene/util/
