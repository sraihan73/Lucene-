using namespace std;

#include "Lucene50CompoundReader.h"
#include "../../index/CorruptIndexException.h"
#include "../../index/IndexFileNames.h"
#include "../../index/SegmentInfo.h"
#include "../../store/IOContext.h"
#include "../../store/IndexInput.h"
#include "../../store/IndexOutput.h"
#include "../../store/Lock.h"
#include "../../util/IOUtils.h"
#include "../CodecUtil.h"
#include "Lucene50CompoundFormat.h"

namespace org::apache::lucene::codecs::lucene50
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using CorruptIndexException = org::apache::lucene::index::CorruptIndexException;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using ChecksumIndexInput = org::apache::lucene::store::ChecksumIndexInput;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using Lock = org::apache::lucene::store::Lock;
using IOUtils = org::apache::lucene::util::IOUtils;

Lucene50CompoundReader::Lucene50CompoundReader(
    shared_ptr<Directory> directory, shared_ptr<SegmentInfo> si,
    shared_ptr<IOContext> context) 
    : directory(directory), segmentName(si->name),
      entries(readEntries(si->getId(), directory, entriesFileName)),
      handle(directory->openInput(dataFileName, context))
{
  wstring dataFileName = IndexFileNames::segmentFileName(
      segmentName, L"", Lucene50CompoundFormat::DATA_EXTENSION);
  wstring entriesFileName = IndexFileNames::segmentFileName(
      segmentName, L"", Lucene50CompoundFormat::ENTRIES_EXTENSION);
  bool success = false;

  int64_t expectedLength =
      CodecUtil::indexHeaderLength(Lucene50CompoundFormat::DATA_CODEC, L"");
  for (auto ent : entries) {
    expectedLength += ent.second->length;
  }
  expectedLength += CodecUtil::footerLength();

  try {
    CodecUtil::checkIndexHeader(handle, Lucene50CompoundFormat::DATA_CODEC,
                                version, version, si->getId(), L"");

    // NOTE: data file is too costly to verify checksum against all the bytes on
    // open, but for now we at least verify proper structure of the checksum
    // footer: which looks for FOOTER_MAGIC + algorithmID. This is cheap and can
    // detect some forms of corruption such as file truncation.
    CodecUtil::retrieveChecksum(handle);

    // We also validate length, because e.g. if you strip 16 bytes off the .cfs
    // we otherwise would not detect it:
    if (handle->length() != expectedLength) {
      throw make_shared<CorruptIndexException>(
          L"length should be " + to_wstring(expectedLength) +
              L" bytes, but is " + to_wstring(handle->length()) + L" instead",
          handle);
    }

    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      IOUtils::closeWhileHandlingException({handle});
    }
  }
}

unordered_map<wstring, std::shared_ptr<FileEntry>>
Lucene50CompoundReader::readEntries(
    std::deque<char> &segmentID, shared_ptr<Directory> dir,
    const wstring &entriesFileName) 
{
  unordered_map<wstring, std::shared_ptr<FileEntry>> mapping;
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.ChecksumIndexInput
  // entriesStream = dir.openChecksumInput(entriesFileName,
  // org.apache.lucene.store.IOContext.READONCE))
  {
    org::apache::lucene::store::ChecksumIndexInput entriesStream =
        dir->openChecksumInput(entriesFileName,
                               org::apache::lucene::store::IOContext::READONCE);
    runtime_error priorE = nullptr;
    try {
      version = CodecUtil::checkIndexHeader(
          entriesStream, Lucene50CompoundFormat::ENTRY_CODEC,
          Lucene50CompoundFormat::VERSION_START,
          Lucene50CompoundFormat::VERSION_CURRENT, segmentID, L"");
      constexpr int numEntries = entriesStream->readVInt();
      mapping = unordered_map<>(numEntries);
      for (int i = 0; i < numEntries; i++) {
        shared_ptr<FileEntry> *const fileEntry = make_shared<FileEntry>();
        const wstring id = entriesStream->readString();
        shared_ptr<FileEntry> previous = mapping.emplace(id, fileEntry);
        if (previous != nullptr) {
          throw make_shared<CorruptIndexException>(
              L"Duplicate cfs entry id=" + id + L" in CFS ", entriesStream);
        }
        fileEntry->offset = entriesStream->readLong();
        fileEntry->length = entriesStream->readLong();
      }
    } catch (const runtime_error &exception) {
      priorE = exception;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      CodecUtil::checkFooter(entriesStream, priorE);
    }
  }
  return mapping;
}

Lucene50CompoundReader::~Lucene50CompoundReader() { IOUtils::close({handle}); }

shared_ptr<IndexInput> Lucene50CompoundReader::openInput(
    const wstring &name, shared_ptr<IOContext> context) 
{
  ensureOpen();
  const wstring id = IndexFileNames::stripSegmentName(name);
  shared_ptr<FileEntry> *const entry = entries[id];
  if (entry == nullptr) {
    wstring datFileName = IndexFileNames::segmentFileName(
        segmentName, L"", Lucene50CompoundFormat::DATA_EXTENSION);
    // C++ TODO: The following line could not be converted:
    throw java.io.FileNotFoundException(L"No sub-file with id " + id +
                                        L" found in compound file \"" +
                                        datFileName + L"\" (fileName=" + name +
                                        L" files: " + entries.keySet() + L")");
  }
  return handle->slice(name, entry->offset, entry->length);
}

std::deque<wstring> Lucene50CompoundReader::listAll()
{
  ensureOpen();
  std::deque<wstring> res =
      entries.keySet().toArray(std::deque<wstring>(entries.size()));

  // Add the segment name
  for (int i = 0; i < res.size(); i++) {
    res[i] = segmentName + res[i];
  }
  return res;
}

void Lucene50CompoundReader::deleteFile(const wstring &name)
{
  throw make_shared<UnsupportedOperationException>();
}

void Lucene50CompoundReader::rename(const wstring &from, const wstring &to)
{
  throw make_shared<UnsupportedOperationException>();
}

void Lucene50CompoundReader::syncMetaData() {}

int64_t
Lucene50CompoundReader::fileLength(const wstring &name) 
{
  ensureOpen();
  shared_ptr<FileEntry> e = entries[IndexFileNames::stripSegmentName(name)];
  if (e == nullptr) {
    // C++ TODO: The following line could not be converted:
    throw java.io.FileNotFoundException(name);
  }
  return e->length;
}

shared_ptr<IndexOutput> Lucene50CompoundReader::createOutput(
    const wstring &name, shared_ptr<IOContext> context) 
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<IndexOutput> Lucene50CompoundReader::createTempOutput(
    const wstring &prefix, const wstring &suffix,
    shared_ptr<IOContext> context) 
{
  throw make_shared<UnsupportedOperationException>();
}

void Lucene50CompoundReader::sync(shared_ptr<deque<wstring>> names)
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<Lock> Lucene50CompoundReader::obtainLock(const wstring &name)
{
  throw make_shared<UnsupportedOperationException>();
}

wstring Lucene50CompoundReader::toString()
{
  return L"CompoundFileDirectory(segment=\"" + segmentName + L"\" in dir=" +
         directory + L")";
}
} // namespace org::apache::lucene::codecs::lucene50