using namespace std;

#include "OfflineSorter.h"

namespace org::apache::lucene::util
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using ChecksumIndexInput = org::apache::lucene::store::ChecksumIndexInput;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using TrackingDirectoryWrapper =
    org::apache::lucene::store::TrackingDirectoryWrapper;
const wstring OfflineSorter::MIN_BUFFER_SIZE_MSG =
    L"At least 0.5MB RAM buffer is needed";

OfflineSorter::BufferSize::BufferSize(int64_t bytes)
    : bytes(static_cast<int>(bytes))
{
  if (bytes > numeric_limits<int>::max()) {
    throw invalid_argument(L"Buffer too large for Java (" +
                           (numeric_limits<int>::max() / MB) + L"mb max): " +
                           to_wstring(bytes));
  }

  if (bytes < ABSOLUTE_MIN_SORT_BUFFER_SIZE) {
    throw invalid_argument(MIN_BUFFER_SIZE_MSG + L": " + to_wstring(bytes));
  }
}

shared_ptr<BufferSize> OfflineSorter::BufferSize::megabytes(int64_t mb)
{
  return make_shared<BufferSize>(mb * MB);
}

shared_ptr<BufferSize> OfflineSorter::BufferSize::automatic()
{
  shared_ptr<Runtime> rt = Runtime::getRuntime();

  // take sizes in "conservative" order
  constexpr int64_t max = rt->maxMemory();     // max allocated
  constexpr int64_t total = rt->totalMemory(); // currently allocated
  constexpr int64_t free =
      rt->freeMemory(); // unused portion of currently allocated
  constexpr int64_t totalAvailableBytes = max - total + free;

  // by free mem (attempting to not grow the heap for this)
  int64_t sortBufferByteSize = free / 2;
  constexpr int64_t minBufferSizeBytes = MIN_BUFFER_SIZE_MB * MB;
  if (sortBufferByteSize < minBufferSizeBytes ||
      totalAvailableBytes >
          10 * minBufferSizeBytes) { // lets see if we need/should to grow the
                                     // heap
    if (totalAvailableBytes / 2 >
        minBufferSizeBytes) { // there is enough mem for a reasonable buffer
      sortBufferByteSize = totalAvailableBytes / 2; // grow the heap
    } else {
      // heap seems smallish lets be conservative fall back to the free/2
      sortBufferByteSize =
          max(ABSOLUTE_MIN_SORT_BUFFER_SIZE, sortBufferByteSize);
    }
  }
  return make_shared<BufferSize>(min(
      static_cast<int64_t>(numeric_limits<int>::max()), sortBufferByteSize));
}

OfflineSorter::SortInfo::SortInfo(shared_ptr<OfflineSorter> outerInstance)
    : outerInstance(outerInstance)
{
}

wstring OfflineSorter::SortInfo::toString()
{
  return wstring::format(
      Locale::ROOT,
      L"time=%.2f sec. total (%.2f reading, %.2f sorting, %.2f merging), "
      L"lines=%d, temp files=%d, merges=%d, soft ram limit=%.2f MB",
      totalTimeMS / 1000.0, readTimeMS / 1000.0, sortTimeMS->get() / 1000.0,
      mergeTimeMS->get() / 1000.0, lineCount, tempMergeFiles, mergeRounds,
      static_cast<double>(bufferSize) / MB);
}

const shared_ptr<java::util::Comparator<std::shared_ptr<BytesRef>>>
    OfflineSorter::DEFAULT_COMPARATOR = java::util::Comparator::naturalOrder();

OfflineSorter::OfflineSorter(
    shared_ptr<Directory> dir,
    const wstring &tempFileNamePrefix) 
    : OfflineSorter(dir, tempFileNamePrefix, DEFAULT_COMPARATOR,
                    BufferSize::automatic(), MAX_TEMPFILES, -1, nullptr, 0)
{
}

OfflineSorter::OfflineSorter(shared_ptr<Directory> dir,
                             const wstring &tempFileNamePrefix,
                             shared_ptr<Comparator<std::shared_ptr<BytesRef>>>
                                 comparator) 
    : OfflineSorter(dir, tempFileNamePrefix, comparator,
                    BufferSize::automatic(), MAX_TEMPFILES, -1, nullptr, 0)
{
}

OfflineSorter::OfflineSorter(
    shared_ptr<Directory> dir, const wstring &tempFileNamePrefix,
    shared_ptr<Comparator<std::shared_ptr<BytesRef>>> comparator,
    shared_ptr<BufferSize> ramBufferSize, int maxTempfiles, int valueLength,
    shared_ptr<ExecutorService> exec, int maxPartitionsInRAM)
    : dir(dir), valueLength(valueLength),
      tempFileNamePrefix(tempFileNamePrefix),
      partitionsInRAM(make_shared<Semaphore>(maxPartitionsInRAM)),
      ramBufferSize(ramBufferSize), comparator(comparator)
{
  if (exec != nullptr) {
    this->exec = exec;
    if (maxPartitionsInRAM <= 0) {
      throw invalid_argument(L"maxPartitionsInRAM must be > 0; got " +
                             to_wstring(maxPartitionsInRAM));
    }
  } else {
    this->exec = make_shared<SameThreadExecutorService>();
    maxPartitionsInRAM = 1;
  }

  if (ramBufferSize->bytes < ABSOLUTE_MIN_SORT_BUFFER_SIZE) {
    throw invalid_argument(MIN_BUFFER_SIZE_MSG + L": " +
                           to_wstring(ramBufferSize->bytes));
  }

  if (maxTempfiles < 2) {
    throw invalid_argument(L"maxTempFiles must be >= 2");
  }

  if (valueLength != -1 &&
      (valueLength == 0 || valueLength > numeric_limits<short>::max())) {
    throw invalid_argument(L"valueLength must be 1 .. " +
                           numeric_limits<short>::max() + L"; got: " +
                           to_wstring(valueLength));
  }

  this->maxTempFiles = maxTempfiles;
}

shared_ptr<Directory> OfflineSorter::getDirectory() { return dir; }

wstring OfflineSorter::getTempFileNamePrefix() { return tempFileNamePrefix; }

wstring OfflineSorter::sort(const wstring &inputFileName) 
{

  sortInfo = make_shared<SortInfo>(shared_from_this());
  int64_t startMS = System::currentTimeMillis();

  deque<Future<std::shared_ptr<Partition>>> segments =
      deque<Future<std::shared_ptr<Partition>>>();
  std::deque<int> levelCounts(1);

  // So we can remove any partially written temp files on exception:
  shared_ptr<TrackingDirectoryWrapper> trackingDir =
      make_shared<TrackingDirectoryWrapper>(dir);

  bool success = false;
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (ByteSequencesReader is =
  // getReader(dir.openChecksumInput(inputFileName,
  // org.apache.lucene.store.IOContext.READONCE), inputFileName))
  {
    ByteSequencesReader is = getReader(
        dir->openChecksumInput(inputFileName,
                               org::apache::lucene::store::IOContext::READONCE),
        inputFileName);
    try {
      while (true) {
        shared_ptr<Partition> part = readPartition(is);
        if (part->count == 0) {
          if (partitionsInRAM != nullptr) {
            partitionsInRAM->release();
          }
          assert(part->exhausted);
          break;
        }

        shared_ptr<Callable<std::shared_ptr<Partition>>> job =
            make_shared<SortPartitionTask>(shared_from_this(), trackingDir,
                                           part);

        segments.push_back(exec->submit(job));
        sortInfo->tempMergeFiles++;
        sortInfo->lineCount += part->count;
        levelCounts[0]++;

        // Handle intermediate merges; we need a while loop to "cascade" the
        // merge when necessary:
        int mergeLevel = 0;
        while (levelCounts[mergeLevel] == maxTempFiles) {
          mergePartitions(trackingDir, segments);
          if (mergeLevel + 2 > levelCounts.size()) {
            levelCounts = ArrayUtil::grow(levelCounts, mergeLevel + 2);
          }
          levelCounts[mergeLevel + 1]++;
          levelCounts[mergeLevel] = 0;
          mergeLevel++;
        }

        if (part->exhausted) {
          break;
        }
      }

      // TODO: we shouldn't have to do this?  Can't we return a merged reader to
      // the caller, who often consumes the result just once, instead?

      // Merge all partitions down to 1 (basically a forceMerge(1)):
      while (segments.size() > 1) {
        mergePartitions(trackingDir, segments);
      }

      wstring result;
      if (segments.empty()) {
        // C++ NOTE: The following 'try with resources' block is replaced by its
        // C++ equivalent: ORIGINAL LINE: try
        // (org.apache.lucene.store.IndexOutput out =
        // trackingDir.createTempOutput(tempFileNamePrefix, "sort",
        // org.apache.lucene.store.IOContext.DEFAULT))
        {
          org::apache::lucene::store::IndexOutput out =
              trackingDir->createTempOutput(
                  tempFileNamePrefix, L"sort",
                  org::apache::lucene::store::IOContext::DEFAULT);
          // Write empty file footer
          CodecUtil::writeFooter(out);
          result = out->getName();
        }
      } else {
        result = getPartition(segments[0])->fileName;
      }

      // We should be explicitly removing all intermediate files ourselves
      // unless there is an exception:
      assert(trackingDir->getCreatedFiles()->size() == 1 &&
             trackingDir->getCreatedFiles()->contains(result));

      sortInfo->totalTimeMS = System::currentTimeMillis() - startMS;

      CodecUtil::checkFooter(is->in_);

      success = true;

      return result;

    } catch (const InterruptedException &ie) {
      throw make_shared<ThreadInterruptedException>(ie);
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (success == false) {
        IOUtils::deleteFilesIgnoringExceptions(
            {trackingDir, trackingDir->getCreatedFiles()});
      }
    }
  }
}

void OfflineSorter::verifyChecksum(
    runtime_error priorException,
    shared_ptr<ByteSequencesReader> reader) 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.ChecksumIndexInput
  // in = dir.openChecksumInput(reader.name,
  // org.apache.lucene.store.IOContext.READONCE))
  {
    org::apache::lucene::store::ChecksumIndexInput in_ = dir->openChecksumInput(
        reader->name, org::apache::lucene::store::IOContext::READONCE);
    CodecUtil::checkFooter(in_, priorException);
  }
}

void OfflineSorter::mergePartitions(
    shared_ptr<Directory> trackingDir,
    deque<Future<std::shared_ptr<Partition>>> &segments) 
{
  int64_t start = System::currentTimeMillis();
  deque<Future<std::shared_ptr<Partition>>> segmentsToMerge;
  if (segments.size() > maxTempFiles) {
    segmentsToMerge =
        segments.subList(segments.size() - maxTempFiles, segments.size());
  } else {
    segmentsToMerge = segments;
  }

  sortInfo->mergeRounds++;

  shared_ptr<MergePartitionsTask> task = make_shared<MergePartitionsTask>(
      shared_from_this(), trackingDir, deque<>(segmentsToMerge));

  segmentsToMerge.clear();
  segments.push_back(exec->submit(task));

  sortInfo->tempMergeFiles++;
}

OfflineSorter::Partition::Partition(shared_ptr<SortableBytesRefArray> buffer,
                                    bool exhausted)
    : buffer(buffer), exhausted(exhausted), count(buffer->size()), fileName(L"")
{
}

OfflineSorter::Partition::Partition(const wstring &fileName, int64_t count)
    : buffer(this->buffer.reset()), exhausted(true), count(count),
      fileName(fileName)
{
}

shared_ptr<Partition> OfflineSorter::readPartition(
    shared_ptr<ByteSequencesReader> reader) throw(IOException,
                                                  InterruptedException)
{
  if (partitionsInRAM != nullptr) {
    partitionsInRAM->acquire();
  }
  bool success = false;
  try {
    int64_t start = System::currentTimeMillis();
    shared_ptr<SortableBytesRefArray> buffer;
    bool exhausted = false;
    int count;
    if (valueLength != -1) {
      // fixed length case
      buffer = make_shared<FixedLengthBytesRefArray>(valueLength);
      int limit = ramBufferSize->bytes / valueLength;
      for (int i = 0; i < limit; i++) {
        shared_ptr<BytesRef> item = nullptr;
        try {
          item = reader->next();
        } catch (const runtime_error &t) {
          verifyChecksum(t, reader);
        }
        if (item == nullptr) {
          exhausted = true;
          break;
        }
        buffer->append(item);
      }
    } else {
      shared_ptr<Counter> bufferBytesUsed = Counter::newCounter();
      buffer = make_shared<BytesRefArray>(bufferBytesUsed);
      while (true) {
        shared_ptr<BytesRef> item = nullptr;
        try {
          item = reader->next();
        } catch (const runtime_error &t) {
          verifyChecksum(t, reader);
        }
        if (item == nullptr) {
          exhausted = true;
          break;
        }
        buffer->append(item);
        // Account for the created objects.
        // (buffer slots do not account to buffer size.)
        if (bufferBytesUsed->get() > ramBufferSize->bytes) {
          break;
        }
      }
    }
    sortInfo->readTimeMS += System::currentTimeMillis() - start;
    success = true;
    return make_shared<Partition>(buffer, exhausted);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success == false && partitionsInRAM != nullptr) {
      partitionsInRAM->release();
    }
  }
}

OfflineSorter::FileAndTop::FileAndTop(int fd, shared_ptr<BytesRef> firstLine)
    : fd(fd)
{
  this->current = firstLine;
}

shared_ptr<ByteSequencesWriter>
OfflineSorter::getWriter(shared_ptr<IndexOutput> out,
                         int64_t itemCount) 
{
  return make_shared<ByteSequencesWriter>(out);
}

shared_ptr<ByteSequencesReader>
OfflineSorter::getReader(shared_ptr<ChecksumIndexInput> in_,
                         const wstring &name) 
{
  return make_shared<ByteSequencesReader>(in_, name);
}

OfflineSorter::ByteSequencesWriter::ByteSequencesWriter(
    shared_ptr<IndexOutput> out)
    : out(out)
{
}

void OfflineSorter::ByteSequencesWriter::write(shared_ptr<BytesRef> ref) throw(
    IOException)
{
  assert(ref != nullptr);
  write(ref->bytes, ref->offset, ref->length);
}

void OfflineSorter::ByteSequencesWriter::write(std::deque<char> &bytes) throw(
    IOException)
{
  write(bytes, 0, bytes.size());
}

void OfflineSorter::ByteSequencesWriter::write(std::deque<char> &bytes,
                                               int off,
                                               int len) 
{
  assert(bytes.size() > 0);
  assert(off >= 0 && off + len <= bytes.size());
  assert(len >= 0);
  if (len > numeric_limits<short>::max()) {
    throw invalid_argument(L"len must be <= " + numeric_limits<short>::max() +
                           L"; got " + to_wstring(len));
  }
  out->writeShort(static_cast<short>(len));
  out->writeBytes(bytes, off, len);
}

OfflineSorter::ByteSequencesWriter::~ByteSequencesWriter() { delete out; }

OfflineSorter::ByteSequencesReader::ByteSequencesReader(
    shared_ptr<ChecksumIndexInput> in_, const wstring &name)
    : name(name), in_(in_), end(in_->length() - CodecUtil::footerLength())
{
}

shared_ptr<BytesRef>
OfflineSorter::ByteSequencesReader::next() 
{
  if (in_->getFilePointer() >= end) {
    return nullptr;
  }

  short length = in_->readShort();
  ref->grow(length);
  ref->setLength(length);
  in_->readBytes(ref->bytes(), 0, length);
  return ref->get();
}

OfflineSorter::ByteSequencesReader::~ByteSequencesReader() { delete in_; }

shared_ptr<Comparator<std::shared_ptr<BytesRef>>> OfflineSorter::getComparator()
{
  return comparator;
}

OfflineSorter::SortPartitionTask::SortPartitionTask(
    shared_ptr<OfflineSorter> outerInstance, shared_ptr<Directory> dir,
    shared_ptr<Partition> part)
    : dir(dir), part(part), outerInstance(outerInstance)
{
}

shared_ptr<Partition>
OfflineSorter::SortPartitionTask::call() 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexOutput tempFile
  // = dir.createTempOutput(tempFileNamePrefix, "sort",
  // org.apache.lucene.store.IOContext.DEFAULT); ByteSequencesWriter out =
  // getWriter(tempFile, part.buffer.size());)
  {
    org::apache::lucene::store::IndexOutput tempFile =
        dir->createTempOutput(outerInstance->tempFileNamePrefix, L"sort",
                              org::apache::lucene::store::IOContext::DEFAULT);
    ByteSequencesWriter out =
        outerInstance->getWriter(tempFile, part->buffer->size());
    try {

      shared_ptr<BytesRef> spare;

      int64_t startMS = System::currentTimeMillis();
      shared_ptr<BytesRefIterator> iter =
          part->buffer->iterator(outerInstance->comparator);
      outerInstance->sortInfo->sortTimeMS->addAndGet(
          System::currentTimeMillis() - startMS);

      int count = 0;
      while ((spare = iter->next()) != nullptr) {
        out->write(spare);
        count++;
      }

      assert(count == part->count);

      CodecUtil::writeFooter(out->out);
      part->buffer->clear();

      return make_shared<Partition>(tempFile->getName(), part->count);
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (outerInstance->partitionsInRAM != nullptr) {
        outerInstance->partitionsInRAM->release();
      }
    }
  }
}

shared_ptr<Partition> OfflineSorter::getPartition(
    shared_ptr<Future<std::shared_ptr<Partition>>> future) 
{
  try {
    return future->get();
  } catch (const InterruptedException &ie) {
    throw make_shared<ThreadInterruptedException>(ie);
  } catch (const ExecutionException &ee) {
    // Theoretically cause can be null; guard against that.
    runtime_error cause = ee->getCause();
    throw IOUtils::rethrowAlways(cause != nullptr ? cause : ee);
  }
}

OfflineSorter::MergePartitionsTask::MergePartitionsTask(
    shared_ptr<OfflineSorter> outerInstance, shared_ptr<Directory> dir,
    deque<Future<std::shared_ptr<Partition>>> &segmentsToMerge)
    : dir(dir), segmentsToMerge(segmentsToMerge), outerInstance(outerInstance)
{
}

shared_ptr<Partition>
OfflineSorter::MergePartitionsTask::call() 
{
  int64_t totalCount = 0;
  for (auto segment : segmentsToMerge) {
    totalCount += outerInstance->getPartition(segment)->count;
  }

  shared_ptr<PriorityQueue<std::shared_ptr<FileAndTop>>> queue =
      make_shared<PriorityQueueAnonymousInnerClass>(shared_from_this(),
                                                    segmentsToMerge.size());

  std::deque<std::shared_ptr<ByteSequencesReader>> streams(
      segmentsToMerge.size());

  wstring newSegmentName = L"";

  int64_t startMS = System::currentTimeMillis();
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (ByteSequencesWriter writer =
  // getWriter(dir.createTempOutput(tempFileNamePrefix, "sort",
  // org.apache.lucene.store.IOContext.DEFAULT), totalCount))
  {
    ByteSequencesWriter writer = outerInstance->getWriter(
        dir->createTempOutput(outerInstance->tempFileNamePrefix, L"sort",
                              org::apache::lucene::store::IOContext::DEFAULT),
        totalCount);
    try {

      newSegmentName = writer->out->getName();

      // Open streams and read the top for each file
      for (int i = 0; i < segmentsToMerge.size(); i++) {
        shared_ptr<Partition> segment =
            outerInstance->getPartition(segmentsToMerge[i]);
        streams[i] = outerInstance->getReader(
            dir->openChecksumInput(segment->fileName, IOContext::READONCE),
            segment->fileName);

        shared_ptr<BytesRef> item = nullptr;
        try {
          item = streams[i]->next();
        } catch (const runtime_error &t) {
          outerInstance->verifyChecksum(t, streams[i]);
        }
        assert(item != nullptr);
        queue->insertWithOverflow(make_shared<FileAndTop>(i, item));
      }

      // Unix utility sort() uses ordered array of files to pick the next line
      // from, updating it as it reads new lines. The PQ used here is a more
      // elegant solution and has a nicer theoretical complexity bound :) The
      // entire sorting process is I/O bound anyway so it shouldn't make much of
      // a difference (didn't check).
      shared_ptr<FileAndTop> top;
      while ((top = queue->top()) != nullptr) {
        writer->write(top->current);
        try {
          top->current = streams[top->fd]->next();
        } catch (const runtime_error &t) {
          outerInstance->verifyChecksum(t, streams[top->fd]);
        }

        if (top->current != nullptr) {
          queue->updateTop();
        } else {
          queue->pop();
        }
      }

      CodecUtil::writeFooter(writer->out);

      for (auto reader : streams) {
        CodecUtil::checkFooter(reader->in_);
      }

      outerInstance->sortInfo->mergeTimeMS->addAndGet(
          System::currentTimeMillis() - startMS);
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      IOUtils::close(streams);
    }
  }
  deque<wstring> toDelete = deque<wstring>();
  for (auto segment : segmentsToMerge) {
    toDelete.push_back(outerInstance->getPartition(segment)->fileName);
  }
  IOUtils::deleteFiles(dir, toDelete);

  return make_shared<Partition>(newSegmentName, totalCount);
}

OfflineSorter::MergePartitionsTask::PriorityQueueAnonymousInnerClass::
    PriorityQueueAnonymousInnerClass(
        shared_ptr<MergePartitionsTask> outerInstance,
        shared_ptr<UnknownType> size)
    : PriorityQueue<FileAndTop>(size)
{
  this->outerInstance = outerInstance;
}

bool OfflineSorter::MergePartitionsTask::PriorityQueueAnonymousInnerClass::
    lessThan(shared_ptr<FileAndTop> a, shared_ptr<FileAndTop> b)
{
  return outerInstance->outerInstance->comparator.compare(a->current,
                                                          b->current) < 0;
}
} // namespace org::apache::lucene::util