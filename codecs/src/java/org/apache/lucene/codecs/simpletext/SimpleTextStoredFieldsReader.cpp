using namespace std;

#include "SimpleTextStoredFieldsReader.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/FieldInfo.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/FieldInfos.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexFileNames.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentInfo.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/StoredFieldVisitor.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/AlreadyClosedException.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/BufferedChecksumIndexInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/ChecksumIndexInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/Directory.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IOContext.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IndexInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/ArrayUtil.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/CharsRefBuilder.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/FutureArrays.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/StringHelper.h"
#include "SimpleTextStoredFieldsWriter.h"
#include "SimpleTextUtil.h"

namespace org::apache::lucene::codecs::simpletext
{
using StoredFieldsReader = org::apache::lucene::codecs::StoredFieldsReader;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using StoredFieldVisitor = org::apache::lucene::index::StoredFieldVisitor;
using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;
using BufferedChecksumIndexInput =
    org::apache::lucene::store::BufferedChecksumIndexInput;
using ChecksumIndexInput = org::apache::lucene::store::ChecksumIndexInput;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using CharsRef = org::apache::lucene::util::CharsRef;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;
using FutureArrays = org::apache::lucene::util::FutureArrays;
using IOUtils = org::apache::lucene::util::IOUtils;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
using StringHelper = org::apache::lucene::util::StringHelper;
using namespace org::apache::lucene::codecs::simpletext;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextStoredFieldsWriter.*;

SimpleTextStoredFieldsReader::SimpleTextStoredFieldsReader(
    shared_ptr<Directory> directory, shared_ptr<SegmentInfo> si,
    shared_ptr<FieldInfos> fn, shared_ptr<IOContext> context) 
    : fieldInfos(fn)
{
  bool success = false;
  try {
    in_ = directory->openInput(
        IndexFileNames::segmentFileName(
            si->name, L"", SimpleTextStoredFieldsWriter::FIELDS_EXTENSION),
        context);
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      try {
        close();
      } catch (const runtime_error &t) {
      } // ensure we throw our original exception
    }
  }
  readIndex(si->maxDoc());
}

SimpleTextStoredFieldsReader::SimpleTextStoredFieldsReader(
    std::deque<int64_t> &offsets, shared_ptr<IndexInput> in_,
    shared_ptr<FieldInfos> fieldInfos)
    : fieldInfos(fieldInfos)
{
  this->offsets = offsets;
  this->in_ = in_;
}

void SimpleTextStoredFieldsReader::readIndex(int size) 
{
  shared_ptr<ChecksumIndexInput> input =
      make_shared<BufferedChecksumIndexInput>(in_);
  offsets = std::deque<int64_t>(size);
  int upto = 0;
  while (!scratch->get().equals(SimpleTextStoredFieldsWriter::END)) {
    SimpleTextUtil::readLine(input, scratch);
    if (StringHelper::startsWith(scratch->get(),
                                 SimpleTextStoredFieldsWriter::DOC)) {
      offsets[upto] = input->getFilePointer();
      upto++;
    }
  }
  SimpleTextUtil::checkFooter(input);
  assert(upto == offsets.size());
}

void SimpleTextStoredFieldsReader::visitDocument(
    int n, shared_ptr<StoredFieldVisitor> visitor) 
{
  in_->seek(offsets[n]);

  while (true) {
    readLine();
    if (StringHelper::startsWith(
            scratch->get(), SimpleTextStoredFieldsWriter::FIELD) == false) {
      break;
    }
    int fieldNumber = parseIntAt(SimpleTextStoredFieldsWriter::FIELD->length);
    shared_ptr<FieldInfo> fieldInfo = fieldInfos->fieldInfo(fieldNumber);
    readLine();
    assert((StringHelper::startsWith(scratch->get(),
                                     SimpleTextStoredFieldsWriter::NAME)));
    readLine();
    assert((StringHelper::startsWith(scratch->get(),
                                     SimpleTextStoredFieldsWriter::TYPE)));

    shared_ptr<BytesRef> *const type;
    if (equalsAt(SimpleTextStoredFieldsWriter::TYPE_STRING, scratch->get(),
                 SimpleTextStoredFieldsWriter::TYPE->length)) {
      type = SimpleTextStoredFieldsWriter::TYPE_STRING;
    } else if (equalsAt(SimpleTextStoredFieldsWriter::TYPE_BINARY,
                        scratch->get(),
                        SimpleTextStoredFieldsWriter::TYPE->length)) {
      type = SimpleTextStoredFieldsWriter::TYPE_BINARY;
    } else if (equalsAt(SimpleTextStoredFieldsWriter::TYPE_INT, scratch->get(),
                        SimpleTextStoredFieldsWriter::TYPE->length)) {
      type = SimpleTextStoredFieldsWriter::TYPE_INT;
    } else if (equalsAt(SimpleTextStoredFieldsWriter::TYPE_LONG, scratch->get(),
                        SimpleTextStoredFieldsWriter::TYPE->length)) {
      type = SimpleTextStoredFieldsWriter::TYPE_LONG;
    } else if (equalsAt(SimpleTextStoredFieldsWriter::TYPE_FLOAT,
                        scratch->get(),
                        SimpleTextStoredFieldsWriter::TYPE->length)) {
      type = SimpleTextStoredFieldsWriter::TYPE_FLOAT;
    } else if (equalsAt(SimpleTextStoredFieldsWriter::TYPE_DOUBLE,
                        scratch->get(),
                        SimpleTextStoredFieldsWriter::TYPE->length)) {
      type = SimpleTextStoredFieldsWriter::TYPE_DOUBLE;
    } else {
      throw runtime_error(L"unknown field type");
    }

    switch (visitor->needsField(fieldInfo)) {
    case SegmentInfo::YES:
      readField(type, fieldInfo, visitor);
      break;
    case SegmentInfo::NO:
      readLine();
      assert((StringHelper::startsWith(scratch->get(),
                                       SimpleTextStoredFieldsWriter::VALUE)));
      break;
    case STOP:
      return;
    }
  }
}

void SimpleTextStoredFieldsReader::readField(
    shared_ptr<BytesRef> type, shared_ptr<FieldInfo> fieldInfo,
    shared_ptr<StoredFieldVisitor> visitor) 
{
  readLine();
  assert((StringHelper::startsWith(scratch->get(),
                                   SimpleTextStoredFieldsWriter::VALUE)));
  if (type == SimpleTextStoredFieldsWriter::TYPE_STRING) {
    std::deque<char> bytes(scratch->length() -
                            SimpleTextStoredFieldsWriter::VALUE->length);
    System::arraycopy(scratch->bytes(),
                      SimpleTextStoredFieldsWriter::VALUE->length, bytes, 0,
                      bytes.size());
    visitor->stringField(fieldInfo, bytes);
  } else if (type == SimpleTextStoredFieldsWriter::TYPE_BINARY) {
    std::deque<char> copy(scratch->length() -
                           SimpleTextStoredFieldsWriter::VALUE->length);
    System::arraycopy(scratch->bytes(),
                      SimpleTextStoredFieldsWriter::VALUE->length, copy, 0,
                      copy.size());
    visitor->binaryField(fieldInfo, copy);
  } else if (type == SimpleTextStoredFieldsWriter::TYPE_INT) {
    scratchUTF16->copyUTF8Bytes(
        scratch->bytes(), SimpleTextStoredFieldsWriter::VALUE->length,
        scratch->length() - SimpleTextStoredFieldsWriter::VALUE->length);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    visitor->intField(fieldInfo, stoi(scratchUTF16->toString()));
  } else if (type == SimpleTextStoredFieldsWriter::TYPE_LONG) {
    scratchUTF16->copyUTF8Bytes(
        scratch->bytes(), SimpleTextStoredFieldsWriter::VALUE->length,
        scratch->length() - SimpleTextStoredFieldsWriter::VALUE->length);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    visitor->longField(fieldInfo, StringHelper::fromString<int64_t>(
                                      scratchUTF16->toString()));
  } else if (type == SimpleTextStoredFieldsWriter::TYPE_FLOAT) {
    scratchUTF16->copyUTF8Bytes(
        scratch->bytes(), SimpleTextStoredFieldsWriter::VALUE->length,
        scratch->length() - SimpleTextStoredFieldsWriter::VALUE->length);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    visitor->floatField(fieldInfo, stof(scratchUTF16->toString()));
  } else if (type == SimpleTextStoredFieldsWriter::TYPE_DOUBLE) {
    scratchUTF16->copyUTF8Bytes(
        scratch->bytes(), SimpleTextStoredFieldsWriter::VALUE->length,
        scratch->length() - SimpleTextStoredFieldsWriter::VALUE->length);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    visitor->doubleField(fieldInfo, stod(scratchUTF16->toString()));
  }
}

shared_ptr<StoredFieldsReader> SimpleTextStoredFieldsReader::clone()
{
  if (in_ == nullptr) {
    throw make_shared<AlreadyClosedException>(L"this FieldsReader is closed");
  }
  return make_shared<SimpleTextStoredFieldsReader>(offsets, in_->clone(),
                                                   fieldInfos);
}

SimpleTextStoredFieldsReader::~SimpleTextStoredFieldsReader()
{
  try {
    IOUtils::close({in_});
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    in_.reset();
    offsets.clear();
  }
}

void SimpleTextStoredFieldsReader::readLine() 
{
  SimpleTextUtil::readLine(in_, scratch);
}

int SimpleTextStoredFieldsReader::parseIntAt(int offset)
{
  scratchUTF16->copyUTF8Bytes(scratch->bytes(), offset,
                              scratch->length() - offset);
  return ArrayUtil::parseInt(scratchUTF16->chars(), 0, scratchUTF16->length());
}

bool SimpleTextStoredFieldsReader::equalsAt(shared_ptr<BytesRef> a,
                                            shared_ptr<BytesRef> b, int bOffset)
{
  return a->length == b->length - bOffset &&
         FutureArrays::equals(a->bytes, a->offset, a->offset + a->length,
                              b->bytes, b->offset + bOffset,
                              b->offset + b->length);
}

int64_t SimpleTextStoredFieldsReader::ramBytesUsed()
{
  return BASE_RAM_BYTES_USED + RamUsageEstimator::sizeOf(offsets) +
         RamUsageEstimator::sizeOf(scratch->bytes()) +
         RamUsageEstimator::sizeOf(scratchUTF16->chars());
}

wstring SimpleTextStoredFieldsReader::toString()
{
  return getClass().getSimpleName();
}

void SimpleTextStoredFieldsReader::checkIntegrity()  {}
} // namespace org::apache::lucene::codecs::simpletext