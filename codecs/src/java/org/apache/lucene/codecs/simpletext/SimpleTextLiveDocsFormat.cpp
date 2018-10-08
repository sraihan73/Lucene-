using namespace std;

#include "SimpleTextLiveDocsFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexFileNames.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentCommitInfo.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/ChecksumIndexInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/Directory.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IOContext.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IndexOutput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/ArrayUtil.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/CharsRefBuilder.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/StringHelper.h"
#include "SimpleTextUtil.h"

namespace org::apache::lucene::codecs::simpletext
{
using LiveDocsFormat = org::apache::lucene::codecs::LiveDocsFormat;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using SegmentCommitInfo = org::apache::lucene::index::SegmentCommitInfo;
using ChecksumIndexInput = org::apache::lucene::store::ChecksumIndexInput;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;
using IOUtils = org::apache::lucene::util::IOUtils;
using StringHelper = org::apache::lucene::util::StringHelper;
const wstring SimpleTextLiveDocsFormat::LIVEDOCS_EXTENSION = L"liv";
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextLiveDocsFormat::SIZE =
        make_shared<org::apache::lucene::util::BytesRef>(L"size ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextLiveDocsFormat::DOC =
        make_shared<org::apache::lucene::util::BytesRef>(L"  doc ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextLiveDocsFormat::END =
        make_shared<org::apache::lucene::util::BytesRef>(L"END");

shared_ptr<Bits> SimpleTextLiveDocsFormat::readLiveDocs(
    shared_ptr<Directory> dir, shared_ptr<SegmentCommitInfo> info,
    shared_ptr<IOContext> context) 
{
  assert(info->hasDeletions());
  shared_ptr<BytesRefBuilder> scratch = make_shared<BytesRefBuilder>();
  shared_ptr<CharsRefBuilder> scratchUTF16 = make_shared<CharsRefBuilder>();

  wstring fileName = IndexFileNames::fileNameFromGeneration(
      info->info->name, LIVEDOCS_EXTENSION, info->getDelGen());
  shared_ptr<ChecksumIndexInput> in_ = nullptr;
  bool success = false;
  try {
    in_ = dir->openChecksumInput(fileName, context);

    SimpleTextUtil::readLine(in_, scratch);
    assert((StringHelper::startsWith(scratch->get(), SIZE)));
    int size = parseIntAt(scratch->get(), SIZE->length, scratchUTF16);

    shared_ptr<BitSet> bits = make_shared<BitSet>(size);

    SimpleTextUtil::readLine(in_, scratch);
    while (!scratch->get().equals(END)) {
      assert((StringHelper::startsWith(scratch->get(), DOC)));
      int docid = parseIntAt(scratch->get(), DOC->length, scratchUTF16);
      bits->set(docid);
      SimpleTextUtil::readLine(in_, scratch);
    }

    SimpleTextUtil::checkFooter(in_);

    success = true;
    return make_shared<SimpleTextBits>(bits, size);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success) {
      IOUtils::close({in_});
    } else {
      IOUtils::closeWhileHandlingException({in_});
    }
  }
}

int SimpleTextLiveDocsFormat::parseIntAt(shared_ptr<BytesRef> bytes, int offset,
                                         shared_ptr<CharsRefBuilder> scratch)
{
  scratch->copyUTF8Bytes(bytes->bytes, bytes->offset + offset,
                         bytes->length - offset);
  return ArrayUtil::parseInt(scratch->chars(), 0, scratch->length());
}

void SimpleTextLiveDocsFormat::writeLiveDocs(
    shared_ptr<Bits> bits, shared_ptr<Directory> dir,
    shared_ptr<SegmentCommitInfo> info, int newDelCount,
    shared_ptr<IOContext> context) 
{
  int size = bits->length();
  shared_ptr<BytesRefBuilder> scratch = make_shared<BytesRefBuilder>();

  wstring fileName = IndexFileNames::fileNameFromGeneration(
      info->info->name, LIVEDOCS_EXTENSION, info->getNextDelGen());
  shared_ptr<IndexOutput> out = nullptr;
  bool success = false;
  try {
    out = dir->createOutput(fileName, context);
    SimpleTextUtil::write(out, SIZE);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    SimpleTextUtil::write(out, Integer::toString(size), scratch);
    SimpleTextUtil::writeNewline(out);

    for (int i = 0; i < size; ++i) {
      if (bits->get(i)) {
        SimpleTextUtil::write(out, DOC);
        // C++ TODO: There is no native C++ equivalent to 'toString':
        SimpleTextUtil::write(out, Integer::toString(i), scratch);
        SimpleTextUtil::writeNewline(out);
      }
    }

    SimpleTextUtil::write(out, END);
    SimpleTextUtil::writeNewline(out);
    SimpleTextUtil::writeChecksum(out, scratch);
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success) {
      IOUtils::close({out});
    } else {
      IOUtils::closeWhileHandlingException({out});
    }
  }
}

void SimpleTextLiveDocsFormat::files(
    shared_ptr<SegmentCommitInfo> info,
    shared_ptr<deque<wstring>> files) 
{
  if (info->hasDeletions()) {
    files->add(IndexFileNames::fileNameFromGeneration(
        info->info->name, LIVEDOCS_EXTENSION, info->getDelGen()));
  }
}

SimpleTextLiveDocsFormat::SimpleTextBits::SimpleTextBits(
    shared_ptr<BitSet> bits, int size)
    : bits(bits), size(size)
{
}

bool SimpleTextLiveDocsFormat::SimpleTextBits::get(int index)
{
  return bits->get(index);
}

int SimpleTextLiveDocsFormat::SimpleTextBits::length() { return size; }
} // namespace org::apache::lucene::codecs::simpletext