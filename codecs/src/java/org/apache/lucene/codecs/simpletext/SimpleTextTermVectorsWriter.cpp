using namespace std;

#include "SimpleTextTermVectorsWriter.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/FieldInfo.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/FieldInfos.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexFileNames.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/Directory.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IOContext.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IndexOutput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "SimpleTextUtil.h"

namespace org::apache::lucene::codecs::simpletext
{
using TermVectorsWriter = org::apache::lucene::codecs::TermVectorsWriter;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using IOUtils = org::apache::lucene::util::IOUtils;
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextTermVectorsWriter::END =
        make_shared<org::apache::lucene::util::BytesRef>(L"END");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextTermVectorsWriter::DOC =
        make_shared<org::apache::lucene::util::BytesRef>(L"doc ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextTermVectorsWriter::NUMFIELDS =
        make_shared<org::apache::lucene::util::BytesRef>(L"  numfields ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextTermVectorsWriter::FIELD =
        make_shared<org::apache::lucene::util::BytesRef>(L"  field ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextTermVectorsWriter::FIELDNAME =
        make_shared<org::apache::lucene::util::BytesRef>(L"    name ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextTermVectorsWriter::FIELDPOSITIONS =
        make_shared<org::apache::lucene::util::BytesRef>(L"    positions ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextTermVectorsWriter::FIELDOFFSETS =
        make_shared<org::apache::lucene::util::BytesRef>(L"    offsets   ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextTermVectorsWriter::FIELDPAYLOADS =
        make_shared<org::apache::lucene::util::BytesRef>(L"    payloads  ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextTermVectorsWriter::FIELDTERMCOUNT =
        make_shared<org::apache::lucene::util::BytesRef>(L"    numterms ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextTermVectorsWriter::TERMTEXT =
        make_shared<org::apache::lucene::util::BytesRef>(L"    term ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextTermVectorsWriter::TERMFREQ =
        make_shared<org::apache::lucene::util::BytesRef>(L"      freq ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextTermVectorsWriter::POSITION =
        make_shared<org::apache::lucene::util::BytesRef>(L"      position ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextTermVectorsWriter::PAYLOAD =
        make_shared<org::apache::lucene::util::BytesRef>(L"        payload ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextTermVectorsWriter::STARTOFFSET =
        make_shared<org::apache::lucene::util::BytesRef>(
            L"        startoffset ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextTermVectorsWriter::ENDOFFSET =
        make_shared<org::apache::lucene::util::BytesRef>(L"        endoffset ");
const wstring SimpleTextTermVectorsWriter::VECTORS_EXTENSION = L"vec";

SimpleTextTermVectorsWriter::SimpleTextTermVectorsWriter(
    shared_ptr<Directory> directory, const wstring &segment,
    shared_ptr<IOContext> context) 
    : directory(directory), segment(segment)
{
  bool success = false;
  try {
    out = directory->createOutput(
        IndexFileNames::segmentFileName(segment, L"", VECTORS_EXTENSION),
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

void SimpleTextTermVectorsWriter::startDocument(int numVectorFields) throw(
    IOException)
{
  write(DOC);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  write(Integer::toString(numDocsWritten));
  newLine();

  write(NUMFIELDS);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  write(Integer::toString(numVectorFields));
  newLine();
  numDocsWritten++;
}

void SimpleTextTermVectorsWriter::startField(shared_ptr<FieldInfo> info,
                                             int numTerms, bool positions,
                                             bool offsets,
                                             bool payloads) 
{
  write(FIELD);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  write(Integer::toString(info->number));
  newLine();

  write(FIELDNAME);
  write(info->name);
  newLine();

  write(FIELDPOSITIONS);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  write(Boolean::toString(positions));
  newLine();

  write(FIELDOFFSETS);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  write(Boolean::toString(offsets));
  newLine();

  write(FIELDPAYLOADS);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  write(Boolean::toString(payloads));
  newLine();

  write(FIELDTERMCOUNT);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  write(Integer::toString(numTerms));
  newLine();

  this->positions = positions;
  this->offsets = offsets;
  this->payloads = payloads;
}

void SimpleTextTermVectorsWriter::startTerm(shared_ptr<BytesRef> term,
                                            int freq) 
{
  write(TERMTEXT);
  write(term);
  newLine();

  write(TERMFREQ);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  write(Integer::toString(freq));
  newLine();
}

void SimpleTextTermVectorsWriter::addPosition(
    int position, int startOffset, int endOffset,
    shared_ptr<BytesRef> payload) 
{
  assert(positions || offsets);

  if (positions) {
    write(POSITION);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    write(Integer::toString(position));
    newLine();

    if (payloads) {
      write(PAYLOAD);
      if (payload != nullptr) {
        assert(payload->length > 0);
        write(payload);
      }
      newLine();
    }
  }

  if (offsets) {
    write(STARTOFFSET);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    write(Integer::toString(startOffset));
    newLine();

    write(ENDOFFSET);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    write(Integer::toString(endOffset));
    newLine();
  }
}

void SimpleTextTermVectorsWriter::finish(shared_ptr<FieldInfos> fis,
                                         int numDocs) 
{
  if (numDocsWritten != numDocs) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    throw runtime_error(
        L"mergeVectors produced an invalid result: mergedDocs is " +
        to_wstring(numDocs) + L" but vec numDocs is " +
        to_wstring(numDocsWritten) + L" file=" + out->toString() +
        L"; now aborting this merge to prevent index corruption");
  }
  write(END);
  newLine();
  SimpleTextUtil::writeChecksum(out, scratch);
}

SimpleTextTermVectorsWriter::~SimpleTextTermVectorsWriter()
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

void SimpleTextTermVectorsWriter::write(const wstring &s) 
{
  SimpleTextUtil::write(out, s, scratch);
}

void SimpleTextTermVectorsWriter::write(shared_ptr<BytesRef> bytes) throw(
    IOException)
{
  SimpleTextUtil::write(out, bytes);
}

void SimpleTextTermVectorsWriter::newLine() 
{
  SimpleTextUtil::writeNewline(out);
}
} // namespace org::apache::lucene::codecs::simpletext