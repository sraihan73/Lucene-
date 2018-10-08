using namespace std;

#include "Lucene50CompoundFormat.h"
#include "../../index/IndexFileNames.h"
#include "../../index/SegmentInfo.h"
#include "../../store/Directory.h"
#include "../../store/IOContext.h"
#include "../CodecUtil.h"
#include "Lucene50CompoundReader.h"

namespace org::apache::lucene::codecs::lucene50
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using CompoundFormat = org::apache::lucene::codecs::CompoundFormat;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using ChecksumIndexInput = org::apache::lucene::store::ChecksumIndexInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexOutput = org::apache::lucene::store::IndexOutput;

Lucene50CompoundFormat::Lucene50CompoundFormat() {}

shared_ptr<Directory> Lucene50CompoundFormat::getCompoundReader(
    shared_ptr<Directory> dir, shared_ptr<SegmentInfo> si,
    shared_ptr<IOContext> context) 
{
  return make_shared<Lucene50CompoundReader>(dir, si, context);
}

void Lucene50CompoundFormat::write(
    shared_ptr<Directory> dir, shared_ptr<SegmentInfo> si,
    shared_ptr<IOContext> context) 
{
  wstring dataFile =
      IndexFileNames::segmentFileName(si->name, L"", DATA_EXTENSION);
  wstring entriesFile =
      IndexFileNames::segmentFileName(si->name, L"", ENTRIES_EXTENSION);

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexOutput data =
  // dir.createOutput(dataFile, context); org.apache.lucene.store.IndexOutput
  // entries = dir.createOutput(entriesFile, context))
  {
    org::apache::lucene::store::IndexOutput data =
        dir->createOutput(dataFile, context);
    org::apache::lucene::store::IndexOutput entries =
        dir->createOutput(entriesFile, context);
    CodecUtil::writeIndexHeader(data, DATA_CODEC, VERSION_CURRENT, si->getId(),
                                L"");
    CodecUtil::writeIndexHeader(entries, ENTRY_CODEC, VERSION_CURRENT,
                                si->getId(), L"");

    // write number of files
    entries->writeVInt(si->files()->size());
    for (auto file : si->files()) {

      // write bytes for file
      int64_t startOffset = data->getFilePointer();
      // C++ NOTE: The following 'try with resources' block is replaced by its
      // C++ equivalent: ORIGINAL LINE: try
      // (org.apache.lucene.store.ChecksumIndexInput in =
      // dir.openChecksumInput(file, org.apache.lucene.store.IOContext.READONCE))
      {
        org::apache::lucene::store::ChecksumIndexInput in_ =
            dir->openChecksumInput(
                file, org::apache::lucene::store::IOContext::READONCE);

        // just copies the index header, verifying that its id matches what we
        // expect
        CodecUtil::verifyAndCopyIndexHeader(in_, data, si->getId());

        // copy all bytes except the footer
        int64_t numBytesToCopy =
            in_->length() - CodecUtil::footerLength() - in_->getFilePointer();
        data->copyBytes(in_, numBytesToCopy);

        // verify footer (checksum) matches for the incoming file we are copying
        int64_t checksum = CodecUtil::checkFooter(in_);

        // this is poached from CodecUtil.writeFooter, but we need to use our
        // own checksum, not data.getChecksum(), but I think adding a public
        // method to CodecUtil to do that is somewhat dangerous:
        data->writeInt(CodecUtil::FOOTER_MAGIC);
        data->writeInt(0);
        data->writeLong(checksum);
      }
      int64_t endOffset = data->getFilePointer();

      int64_t length = endOffset - startOffset;

      // write entry for file
      entries->writeString(IndexFileNames::stripSegmentName(file));
      entries->writeLong(startOffset);
      entries->writeLong(length);
    }

    CodecUtil::writeFooter(data);
    CodecUtil::writeFooter(entries);
  }
}

const wstring Lucene50CompoundFormat::DATA_EXTENSION = L"cfs";
const wstring Lucene50CompoundFormat::ENTRIES_EXTENSION = L"cfe";
const wstring Lucene50CompoundFormat::DATA_CODEC = L"Lucene50CompoundData";
const wstring Lucene50CompoundFormat::ENTRY_CODEC = L"Lucene50CompoundEntries";
} // namespace org::apache::lucene::codecs::lucene50