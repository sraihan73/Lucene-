using namespace std;

#include "Lucene50LiveDocsFormat.h"
#include "../../index/CorruptIndexException.h"
#include "../../index/IndexFileNames.h"
#include "../../index/SegmentCommitInfo.h"
#include "../../store/Directory.h"
#include "../../store/IOContext.h"
#include "../../util/Bits.h"
#include "../../util/FixedBitSet.h"
#include "../CodecUtil.h"

namespace org::apache::lucene::codecs::lucene50
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using LiveDocsFormat = org::apache::lucene::codecs::LiveDocsFormat;
using CorruptIndexException = org::apache::lucene::index::CorruptIndexException;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using SegmentCommitInfo = org::apache::lucene::index::SegmentCommitInfo;
using ChecksumIndexInput = org::apache::lucene::store::ChecksumIndexInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using Bits = org::apache::lucene::util::Bits;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;

Lucene50LiveDocsFormat::Lucene50LiveDocsFormat() {}

const wstring Lucene50LiveDocsFormat::EXTENSION = L"liv";
const wstring Lucene50LiveDocsFormat::CODEC_NAME = L"Lucene50LiveDocs";

shared_ptr<Bits> Lucene50LiveDocsFormat::readLiveDocs(
    shared_ptr<Directory> dir, shared_ptr<SegmentCommitInfo> info,
    shared_ptr<IOContext> context) 
{
  int64_t gen = info->getDelGen();
  wstring name =
      IndexFileNames::fileNameFromGeneration(info->info->name, EXTENSION, gen);
  constexpr int length = info->info->maxDoc();
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.ChecksumIndexInput
  // input = dir.openChecksumInput(name, context))
  {
    org::apache::lucene::store::ChecksumIndexInput input =
        dir->openChecksumInput(name, context);
    runtime_error priorE = nullptr;
    try {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      CodecUtil::checkIndexHeader(input, CODEC_NAME, VERSION_START,
                                  VERSION_CURRENT, info->info->getId(),
                                  Long::toString(gen, Character::MAX_RADIX));
      std::deque<int64_t> data(FixedBitSet::bits2words(length));
      for (int i = 0; i < data.size(); i++) {
        data[i] = input->readLong();
      }
      shared_ptr<FixedBitSet> fbs = make_shared<FixedBitSet>(data, length);
      if (fbs->length() - fbs->cardinality() != info->getDelCount()) {
        throw make_shared<CorruptIndexException>(
            L"bits.deleted=" + to_wstring(fbs->length() - fbs->cardinality()) +
                L" info.delcount=" + to_wstring(info->getDelCount()),
            input);
      }
      return fbs->asReadOnlyBits();
    } catch (const runtime_error &exception) {
      priorE = exception;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      CodecUtil::checkFooter(input, priorE);
    }
  }
  throw make_shared<AssertionError>();
}

void Lucene50LiveDocsFormat::writeLiveDocs(
    shared_ptr<Bits> bits, shared_ptr<Directory> dir,
    shared_ptr<SegmentCommitInfo> info, int newDelCount,
    shared_ptr<IOContext> context) 
{
  int64_t gen = info->getNextDelGen();
  wstring name =
      IndexFileNames::fileNameFromGeneration(info->info->name, EXTENSION, gen);
  int delCount = 0;
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexOutput output =
  // dir.createOutput(name, context))
  {
    org::apache::lucene::store::IndexOutput output =
        dir->createOutput(name, context);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    CodecUtil::writeIndexHeader(output, CODEC_NAME, VERSION_CURRENT,
                                info->info->getId(),
                                Long::toString(gen, Character::MAX_RADIX));
    constexpr int longCount = FixedBitSet::bits2words(bits->length());
    for (int i = 0; i < longCount; ++i) {
      int64_t currentBits = 0;
      for (int j = i << 6, end = min(j + 63, bits->length() - 1); j <= end;
           ++j) {
        if (bits->get(j)) {
          currentBits |= 1LL << j; // mod 64
        } else {
          delCount += 1;
        }
      }
      output->writeLong(currentBits);
    }
    CodecUtil::writeFooter(output);
  }
  if (delCount != info->getDelCount() + newDelCount) {
    throw make_shared<CorruptIndexException>(
        L"bits.deleted=" + to_wstring(delCount) + L" info.delcount=" +
            to_wstring(info->getDelCount()) + L" newdelcount=" +
            to_wstring(newDelCount),
        name);
  }
}

void Lucene50LiveDocsFormat::files(
    shared_ptr<SegmentCommitInfo> info,
    shared_ptr<deque<wstring>> files) 
{
  if (info->hasDeletions()) {
    files->add(IndexFileNames::fileNameFromGeneration(
        info->info->name, EXTENSION, info->getDelGen()));
  }
}
} // namespace org::apache::lucene::codecs::lucene50