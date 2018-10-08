using namespace std;

#include "SimpleTextUtil.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/CorruptIndexException.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/ChecksumIndexInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/DataInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/DataOutput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IndexOutput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/StringHelper.h"

namespace org::apache::lucene::codecs::simpletext
{
using CorruptIndexException = org::apache::lucene::index::CorruptIndexException;
using ChecksumIndexInput = org::apache::lucene::store::ChecksumIndexInput;
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using StringHelper = org::apache::lucene::util::StringHelper;
const shared_ptr<org::apache::lucene::util::BytesRef> SimpleTextUtil::CHECKSUM =
    make_shared<org::apache::lucene::util::BytesRef>(L"checksum ");

void SimpleTextUtil::write(
    shared_ptr<DataOutput> out, const wstring &s,
    shared_ptr<BytesRefBuilder> scratch) 
{
  scratch->copyChars(s, 0, s.length());
  write(out, scratch->get());
}

void SimpleTextUtil::write(shared_ptr<DataOutput> out,
                           shared_ptr<BytesRef> b) 
{
  for (int i = 0; i < b->length; i++) {
    constexpr char bx = b->bytes[b->offset + i];
    if (bx == NEWLINE || bx == ESCAPE) {
      out->writeByte(ESCAPE);
    }
    out->writeByte(bx);
  }
}

void SimpleTextUtil::writeNewline(shared_ptr<DataOutput> out) 
{
  out->writeByte(NEWLINE);
}

void SimpleTextUtil::readLine(
    shared_ptr<DataInput> in_,
    shared_ptr<BytesRefBuilder> scratch) 
{
  int upto = 0;
  while (true) {
    char b = in_->readByte();
    scratch->grow(1 + upto);
    if (b == ESCAPE) {
      scratch->setByteAt(upto++, in_->readByte());
    } else {
      if (b == NEWLINE) {
        break;
      } else {
        scratch->setByteAt(upto++, b);
      }
    }
  }
  scratch->setLength(upto);
}

void SimpleTextUtil::writeChecksum(
    shared_ptr<IndexOutput> out,
    shared_ptr<BytesRefBuilder> scratch) 
{
  // Pad with zeros so different checksum values use the
  // same number of bytes
  // (BaseIndexFileFormatTestCase.testMergeStability cares):
  wstring checksum =
      wstring::format(Locale::ROOT, L"%020d", out->getChecksum());
  write(out, CHECKSUM);
  write(out, checksum, scratch);
  writeNewline(out);
}

void SimpleTextUtil::checkFooter(shared_ptr<ChecksumIndexInput> input) throw(
    IOException)
{
  shared_ptr<BytesRefBuilder> scratch = make_shared<BytesRefBuilder>();
  wstring expectedChecksum =
      wstring::format(Locale::ROOT, L"%020d", input->getChecksum());
  readLine(input, scratch);
  if (StringHelper::startsWith(scratch->get(), CHECKSUM) == false) {
    throw make_shared<CorruptIndexException>(
        L"SimpleText failure: expected checksum line but got " +
            scratch->get().utf8ToString(),
        input);
  }
  wstring actualChecksum =
      (make_shared<BytesRef>(scratch->bytes(), CHECKSUM->length,
                             scratch->length() - CHECKSUM->length))
          ->utf8ToString();
  if (expectedChecksum != actualChecksum) {
    throw make_shared<CorruptIndexException>(L"SimpleText checksum failure: " +
                                                 actualChecksum + L" != " +
                                                 expectedChecksum,
                                             input);
  }
  if (input->length() != input->getFilePointer()) {
    throw make_shared<CorruptIndexException>(
        L"Unexpected stuff at the end of file, please be careful with your "
        L"text editor!",
        input);
  }
}

shared_ptr<BytesRef> SimpleTextUtil::fromBytesRefString(const wstring &s)
{
  if (s.length() < 2) {
    throw invalid_argument(L"string " + s +
                           L" was not created from BytesRef.toString?");
  }
  if (s[0] != L'[' || s[s.length() - 1] != L']') {
    throw invalid_argument(L"string " + s +
                           L" was not created from BytesRef.toString?");
  }
  std::deque<wstring> parts = s.substr(1, (s.length() - 1) - 1)->split(L" ");
  std::deque<char> bytes(parts.size());
  for (int i = 0; i < parts.size(); i++) {
    // C++ TODO: Only single-argument parse and valueOf methods are converted:
    // ORIGINAL LINE: bytes[i] = (byte) Integer.parseInt(parts[i], 16);
    bytes[i] = static_cast<char>(Integer::valueOf(parts[i], 16));
  }

  return make_shared<BytesRef>(bytes);
}
} // namespace org::apache::lucene::codecs::simpletext