using namespace std;

#include "SimpleTextStoredFieldsWriter.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/FieldInfo.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/FieldInfos.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexFileNames.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexableField.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/Directory.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IOContext.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IndexOutput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "SimpleTextUtil.h"

namespace org::apache::lucene::codecs::simpletext
{
using StoredFieldsWriter = org::apache::lucene::codecs::StoredFieldsWriter;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using IndexableField = org::apache::lucene::index::IndexableField;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using IOUtils = org::apache::lucene::util::IOUtils;
const wstring SimpleTextStoredFieldsWriter::FIELDS_EXTENSION = L"fld";
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextStoredFieldsWriter::TYPE_STRING =
        make_shared<org::apache::lucene::util::BytesRef>(L"string");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextStoredFieldsWriter::TYPE_BINARY =
        make_shared<org::apache::lucene::util::BytesRef>(L"binary");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextStoredFieldsWriter::TYPE_INT =
        make_shared<org::apache::lucene::util::BytesRef>(L"int");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextStoredFieldsWriter::TYPE_LONG =
        make_shared<org::apache::lucene::util::BytesRef>(L"long");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextStoredFieldsWriter::TYPE_FLOAT =
        make_shared<org::apache::lucene::util::BytesRef>(L"float");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextStoredFieldsWriter::TYPE_DOUBLE =
        make_shared<org::apache::lucene::util::BytesRef>(L"double");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextStoredFieldsWriter::END =
        make_shared<org::apache::lucene::util::BytesRef>(L"END");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextStoredFieldsWriter::DOC =
        make_shared<org::apache::lucene::util::BytesRef>(L"doc ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextStoredFieldsWriter::FIELD =
        make_shared<org::apache::lucene::util::BytesRef>(L"  field ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextStoredFieldsWriter::NAME =
        make_shared<org::apache::lucene::util::BytesRef>(L"    name ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextStoredFieldsWriter::TYPE =
        make_shared<org::apache::lucene::util::BytesRef>(L"    type ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextStoredFieldsWriter::VALUE =
        make_shared<org::apache::lucene::util::BytesRef>(L"    value ");

SimpleTextStoredFieldsWriter::SimpleTextStoredFieldsWriter(
    shared_ptr<Directory> directory, const wstring &segment,
    shared_ptr<IOContext> context) 
    : directory(directory), segment(segment)
{
  bool success = false;
  try {
    out = directory->createOutput(
        IndexFileNames::segmentFileName(segment, L"", FIELDS_EXTENSION),
        context);
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      // C++ TODO: You cannot use 'shared_from_this' in a constructor:
      IOUtils::closeWhileHandlingException({shared_from_this()});
    }
  }
}

void SimpleTextStoredFieldsWriter::startDocument() 
{
  write(DOC);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  write(Integer::toString(numDocsWritten));
  newLine();

  numDocsWritten++;
}

void SimpleTextStoredFieldsWriter::writeField(
    shared_ptr<FieldInfo> info,
    shared_ptr<IndexableField> field) 
{
  write(FIELD);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  write(Integer::toString(info->number));
  newLine();

  write(NAME);
  write(field->name());
  newLine();

  write(TYPE);
  shared_ptr<Number> *const n = field->numericValue();

  if (n != nullptr) {
    if (dynamic_cast<optional<char>>(n) != nullptr ||
        dynamic_cast<optional<short>>(n) != nullptr ||
        dynamic_cast<optional<int>>(n) != nullptr) {
      write(TYPE_INT);
      newLine();

      write(VALUE);
      // C++ TODO: There is no native C++ equivalent to 'toString':
      write(Integer::toString(n->intValue()));
      newLine();
    } else if (dynamic_cast<optional<int64_t>>(n) != nullptr) {
      write(TYPE_LONG);
      newLine();

      write(VALUE);
      // C++ TODO: There is no native C++ equivalent to 'toString':
      write(Long::toString(n->longValue()));
      newLine();
    } else if (dynamic_cast<optional<float>>(n) != nullptr) {
      write(TYPE_FLOAT);
      newLine();

      write(VALUE);
      // C++ TODO: There is no native C++ equivalent to 'toString':
      write(Float::toString(n->floatValue()));
      newLine();
    } else if (dynamic_cast<optional<double>>(n) != nullptr) {
      write(TYPE_DOUBLE);
      newLine();

      write(VALUE);
      // C++ TODO: There is no native C++ equivalent to 'toString':
      write(Double::toString(n->doubleValue()));
      newLine();
    } else {
      throw invalid_argument(L"cannot store numeric type " + n->getClass());
    }
  } else {
    shared_ptr<BytesRef> bytes = field->binaryValue();
    if (bytes != nullptr) {
      write(TYPE_BINARY);
      newLine();

      write(VALUE);
      write(bytes);
      newLine();
    } else if (field->stringValue() == L"") {
      throw invalid_argument(L"field " + field->name() +
                             L" is stored but does not have binaryValue, "
                             L"stringValue nor numericValue");
    } else {
      write(TYPE_STRING);
      newLine();
      write(VALUE);
      write(field->stringValue());
      newLine();
    }
  }
}

void SimpleTextStoredFieldsWriter::finish(shared_ptr<FieldInfos> fis,
                                          int numDocs) 
{
  if (numDocsWritten != numDocs) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    throw runtime_error(
        L"mergeFields produced an invalid result: docCount is " +
        to_wstring(numDocs) + L" but only saw " + to_wstring(numDocsWritten) +
        L" file=" + out->toString() +
        L"; now aborting this merge to prevent index corruption");
  }
  write(END);
  newLine();
  SimpleTextUtil::writeChecksum(out, scratch);
}

SimpleTextStoredFieldsWriter::~SimpleTextStoredFieldsWriter()
{
  try {
    IOUtils::close({out});
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    out.reset();
  }
}

void SimpleTextStoredFieldsWriter::write(const wstring &s) 
{
  SimpleTextUtil::write(out, s, scratch);
}

void SimpleTextStoredFieldsWriter::write(shared_ptr<BytesRef> bytes) throw(
    IOException)
{
  SimpleTextUtil::write(out, bytes);
}

void SimpleTextStoredFieldsWriter::newLine() 
{
  SimpleTextUtil::writeNewline(out);
}
} // namespace org::apache::lucene::codecs::simpletext