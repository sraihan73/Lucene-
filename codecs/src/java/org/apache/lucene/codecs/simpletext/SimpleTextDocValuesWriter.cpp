using namespace std;

#include "SimpleTextDocValuesWriter.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/DocValuesProducer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/FieldInfo.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexFileNames.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/NumericDocValues.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SortedDocValues.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SortedNumericDocValues.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SortedSetDocValues.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/TermsEnum.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/DocIdSetIterator.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IndexOutput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "SimpleTextUtil.h"

namespace org::apache::lucene::codecs::simpletext
{
using DocValuesConsumer = org::apache::lucene::codecs::DocValuesConsumer;
using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using BinaryDocValues = org::apache::lucene::index::BinaryDocValues;
using DocValuesType = org::apache::lucene::index::DocValuesType;
using EmptyDocValuesProducer =
    org::apache::lucene::index::EmptyDocValuesProducer;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using SortedNumericDocValues =
    org::apache::lucene::index::SortedNumericDocValues;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using IOUtils = org::apache::lucene::util::IOUtils;
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextDocValuesWriter::END =
        make_shared<org::apache::lucene::util::BytesRef>(L"END");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextDocValuesWriter::FIELD =
        make_shared<org::apache::lucene::util::BytesRef>(L"field ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextDocValuesWriter::TYPE =
        make_shared<org::apache::lucene::util::BytesRef>(L"  type ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextDocValuesWriter::MINVALUE =
        make_shared<org::apache::lucene::util::BytesRef>(L"  minvalue ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextDocValuesWriter::PATTERN =
        make_shared<org::apache::lucene::util::BytesRef>(L"  pattern ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextDocValuesWriter::LENGTH =
        make_shared<org::apache::lucene::util::BytesRef>(L"length ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextDocValuesWriter::MAXLENGTH =
        make_shared<org::apache::lucene::util::BytesRef>(L"  maxlength ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextDocValuesWriter::NUMVALUES =
        make_shared<org::apache::lucene::util::BytesRef>(L"  numvalues ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextDocValuesWriter::ORDPATTERN =
        make_shared<org::apache::lucene::util::BytesRef>(L"  ordpattern ");

SimpleTextDocValuesWriter::SimpleTextDocValuesWriter(
    shared_ptr<SegmentWriteState> state, const wstring &ext) 
    : numDocs(state->segmentInfo->maxDoc())
{
  // System.out.println("WRITE: " +
  // IndexFileNames.segmentFileName(state.segmentInfo.name, state.segmentSuffix,
  // ext) + " " + state.segmentInfo.maxDoc() + " docs");
  data = state->directory->createOutput(
      IndexFileNames::segmentFileName(state->segmentInfo->name,
                                      state->segmentSuffix, ext),
      state->context);
}

bool SimpleTextDocValuesWriter::fieldSeen(const wstring &field)
{
  assert((!fieldsSeen->contains(field),
          L"field \"" + field + L"\" was added more than once during flush"));
  fieldsSeen->add(field);
  return true;
}

void SimpleTextDocValuesWriter::addNumericField(
    shared_ptr<FieldInfo> field,
    shared_ptr<DocValuesProducer> valuesProducer) 
{
  assert(fieldSeen(field->name));
  assert(field->getDocValuesType() == DocValuesType::NUMERIC ||
         field->hasNorms());
  writeFieldEntry(field, DocValuesType::NUMERIC);

  // first pass to find min/max
  int64_t minValue = numeric_limits<int64_t>::max();
  int64_t maxValue = numeric_limits<int64_t>::min();
  shared_ptr<NumericDocValues> values = valuesProducer->getNumeric(field);
  int numValues = 0;
  for (int doc = values->nextDoc(); doc != DocIdSetIterator::NO_MORE_DOCS;
       doc = values->nextDoc()) {
    int64_t v = values->longValue();
    minValue = min(minValue, v);
    maxValue = max(maxValue, v);
    numValues++;
  }
  if (numValues != numDocs) {
    minValue = min(minValue, 0);
    maxValue = max(maxValue, 0);
  }

  // write our minimum value to the .dat, all entries are deltas from that
  SimpleTextUtil::write(data, MINVALUE);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  SimpleTextUtil::write(data, Long::toString(minValue), scratch);
  SimpleTextUtil::writeNewline(data);

  // build up our fixed-width "simple text packed ints"
  // format
  shared_ptr<int64_t> maxBig = static_cast<int64_t>(maxValue);
  shared_ptr<int64_t> minBig = static_cast<int64_t>(minValue);
  shared_ptr<int64_t> diffBig = maxBig->subtract(minBig);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  int maxBytesPerValue = diffBig->toString()->length();
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  for (int i = 0; i < maxBytesPerValue; i++) {
    sb->append(L'0');
  }

  // write our pattern to the .dat
  SimpleTextUtil::write(data, PATTERN);
  SimpleTextUtil::write(data, sb->toString(), scratch);
  SimpleTextUtil::writeNewline(data);

  const wstring patternString = sb->toString();

  shared_ptr<DecimalFormat> *const encoder = make_shared<DecimalFormat>(
      patternString, make_shared<DecimalFormatSymbols>(Locale::ROOT));

  int numDocsWritten = 0;

  // second pass to write the values
  values = valuesProducer->getNumeric(field);
  for (int i = 0; i < numDocs; ++i) {
    if (values->docID() < i) {
      values->nextDoc();
      assert(values->docID() >= i);
    }
    int64_t value = values->docID() != i ? 0 : values->longValue();
    assert(value >= minValue);
    shared_ptr<Number> delta = static_cast<int64_t>(value).subtract(
        static_cast<int64_t>(minValue));
    wstring s = encoder->format(delta);
    assert(s.length() == patternString.length());
    SimpleTextUtil::write(data, s, scratch);
    SimpleTextUtil::writeNewline(data);
    if (values->docID() != i) {
      SimpleTextUtil::write(data, L"F", scratch);
    } else {
      SimpleTextUtil::write(data, L"T", scratch);
    }
    SimpleTextUtil::writeNewline(data);
    numDocsWritten++;
    assert(numDocsWritten <= numDocs);
  }

  assert((numDocs == numDocsWritten, L"numDocs=" + to_wstring(numDocs) +
                                         L" numDocsWritten=" +
                                         to_wstring(numDocsWritten)));
}

void SimpleTextDocValuesWriter::addBinaryField(
    shared_ptr<FieldInfo> field,
    shared_ptr<DocValuesProducer> valuesProducer) 
{
  assert(fieldSeen(field->name));
  assert(field->getDocValuesType() == DocValuesType::BINARY);
  doAddBinaryField(field, valuesProducer);
}

void SimpleTextDocValuesWriter::doAddBinaryField(
    shared_ptr<FieldInfo> field,
    shared_ptr<DocValuesProducer> valuesProducer) 
{
  int maxLength = 0;
  shared_ptr<BinaryDocValues> values = valuesProducer->getBinary(field);
  for (int doc = values->nextDoc(); doc != DocIdSetIterator::NO_MORE_DOCS;
       doc = values->nextDoc()) {
    maxLength = max(maxLength, values->binaryValue()->length);
  }
  writeFieldEntry(field, DocValuesType::BINARY);

  // write maxLength
  SimpleTextUtil::write(data, MAXLENGTH);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  SimpleTextUtil::write(data, Integer::toString(maxLength), scratch);
  SimpleTextUtil::writeNewline(data);

  // C++ TODO: There is no native C++ equivalent to 'toString':
  int maxBytesLength = Long::toString(maxLength)->length();
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  for (int i = 0; i < maxBytesLength; i++) {
    sb->append(L'0');
  }
  // write our pattern for encoding lengths
  SimpleTextUtil::write(data, PATTERN);
  SimpleTextUtil::write(data, sb->toString(), scratch);
  SimpleTextUtil::writeNewline(data);
  shared_ptr<DecimalFormat> *const encoder = make_shared<DecimalFormat>(
      sb->toString(), make_shared<DecimalFormatSymbols>(Locale::ROOT));

  values = valuesProducer->getBinary(field);
  int numDocsWritten = 0;
  for (int i = 0; i < numDocs; ++i) {
    if (values->docID() < i) {
      values->nextDoc();
      assert(values->docID() >= i);
    }
    // write length
    constexpr int length =
        values->docID() != i ? 0 : values->binaryValue()->length;
    SimpleTextUtil::write(data, LENGTH);
    SimpleTextUtil::write(data, encoder->format(length), scratch);
    SimpleTextUtil::writeNewline(data);

    // write bytes -- don't use SimpleText.write
    // because it escapes:
    if (values->docID() == i) {
      shared_ptr<BytesRef> value = values->binaryValue();
      data->writeBytes(value->bytes, value->offset, value->length);
    }

    // pad to fit
    for (int j = length; j < maxLength; j++) {
      data->writeByte(static_cast<char>(L' '));
    }
    SimpleTextUtil::writeNewline(data);
    if (values->docID() != i) {
      SimpleTextUtil::write(data, L"F", scratch);
    } else {
      SimpleTextUtil::write(data, L"T", scratch);
    }
    SimpleTextUtil::writeNewline(data);
    numDocsWritten++;
  }

  assert(numDocs == numDocsWritten);
}

void SimpleTextDocValuesWriter::addSortedField(
    shared_ptr<FieldInfo> field,
    shared_ptr<DocValuesProducer> valuesProducer) 
{
  assert(fieldSeen(field->name));
  assert(field->getDocValuesType() == DocValuesType::SORTED);
  writeFieldEntry(field, DocValuesType::SORTED);

  int valueCount = 0;
  int maxLength = -1;
  shared_ptr<TermsEnum> terms = valuesProducer->getSorted(field)->termsEnum();
  for (shared_ptr<BytesRef> value = terms->next(); value != nullptr;
       value = terms->next()) {
    maxLength = max(maxLength, value->length);
    valueCount++;
  }

  // write numValues
  SimpleTextUtil::write(data, NUMVALUES);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  SimpleTextUtil::write(data, Integer::toString(valueCount), scratch);
  SimpleTextUtil::writeNewline(data);

  // write maxLength
  SimpleTextUtil::write(data, MAXLENGTH);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  SimpleTextUtil::write(data, Integer::toString(maxLength), scratch);
  SimpleTextUtil::writeNewline(data);

  // C++ TODO: There is no native C++ equivalent to 'toString':
  int maxBytesLength = Integer::toString(maxLength)->length();
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  for (int i = 0; i < maxBytesLength; i++) {
    sb->append(L'0');
  }

  // write our pattern for encoding lengths
  SimpleTextUtil::write(data, PATTERN);
  SimpleTextUtil::write(data, sb->toString(), scratch);
  SimpleTextUtil::writeNewline(data);
  shared_ptr<DecimalFormat> *const encoder = make_shared<DecimalFormat>(
      sb->toString(), make_shared<DecimalFormatSymbols>(Locale::ROOT));

  // C++ TODO: There is no native C++ equivalent to 'toString':
  int maxOrdBytes = Long::toString(valueCount + 1LL)->length();
  sb->setLength(0);
  for (int i = 0; i < maxOrdBytes; i++) {
    sb->append(L'0');
  }

  // write our pattern for ords
  SimpleTextUtil::write(data, ORDPATTERN);
  SimpleTextUtil::write(data, sb->toString(), scratch);
  SimpleTextUtil::writeNewline(data);
  shared_ptr<DecimalFormat> *const ordEncoder = make_shared<DecimalFormat>(
      sb->toString(), make_shared<DecimalFormatSymbols>(Locale::ROOT));

  // for asserts:
  int valuesSeen = 0;

  terms = valuesProducer->getSorted(field)->termsEnum();
  for (shared_ptr<BytesRef> value = terms->next(); value != nullptr;
       value = terms->next()) {
    // write length
    SimpleTextUtil::write(data, LENGTH);
    SimpleTextUtil::write(data, encoder->format(value->length), scratch);
    SimpleTextUtil::writeNewline(data);

    // write bytes -- don't use SimpleText.write
    // because it escapes:
    data->writeBytes(value->bytes, value->offset, value->length);

    // pad to fit
    for (int i = value->length; i < maxLength; i++) {
      data->writeByte(static_cast<char>(L' '));
    }
    SimpleTextUtil::writeNewline(data);
    valuesSeen++;
    assert(valuesSeen <= valueCount);
  }

  assert(valuesSeen == valueCount);

  shared_ptr<SortedDocValues> values = valuesProducer->getSorted(field);
  for (int i = 0; i < numDocs; ++i) {
    if (values->docID() < i) {
      values->nextDoc();
      assert(values->docID() >= i);
    }
    int ord = -1;
    if (values->docID() == i) {
      ord = values->ordValue();
    }
    SimpleTextUtil::write(data, ordEncoder->format(ord + 1LL), scratch);
    SimpleTextUtil::writeNewline(data);
  }
}

void SimpleTextDocValuesWriter::addSortedNumericField(
    shared_ptr<FieldInfo> field,
    shared_ptr<DocValuesProducer> valuesProducer) 
{
  assert(fieldSeen(field->name));
  assert(field->getDocValuesType() == DocValuesType::SORTED_NUMERIC);
  doAddBinaryField(field,
                   make_shared<EmptyDocValuesProducerAnonymousInnerClass>(
                       shared_from_this(), field, valuesProducer));
}

SimpleTextDocValuesWriter::EmptyDocValuesProducerAnonymousInnerClass::
    EmptyDocValuesProducerAnonymousInnerClass(
        shared_ptr<SimpleTextDocValuesWriter> outerInstance,
        shared_ptr<FieldInfo> field,
        shared_ptr<DocValuesProducer> valuesProducer)
{
  this->outerInstance = outerInstance;
  this->field = field;
  this->valuesProducer = valuesProducer;
}

shared_ptr<BinaryDocValues>
SimpleTextDocValuesWriter::EmptyDocValuesProducerAnonymousInnerClass::getBinary(
    shared_ptr<FieldInfo> field) 
{
  shared_ptr<SortedNumericDocValues> values =
      valuesProducer->getSortedNumeric(field);
  return make_shared<BinaryDocValuesAnonymousInnerClass>(shared_from_this(),
                                                         values);
}

SimpleTextDocValuesWriter::EmptyDocValuesProducerAnonymousInnerClass::
    BinaryDocValuesAnonymousInnerClass::BinaryDocValuesAnonymousInnerClass(
        shared_ptr<EmptyDocValuesProducerAnonymousInnerClass> outerInstance,
        shared_ptr<SortedNumericDocValues> values)
{
  this->outerInstance = outerInstance;
  this->values = values;
  builder = make_shared<StringBuilder>();
}

int SimpleTextDocValuesWriter::EmptyDocValuesProducerAnonymousInnerClass::
    BinaryDocValuesAnonymousInnerClass::nextDoc() 
{
  int doc = values->nextDoc();
  setCurrentDoc();
  return doc;
}

int SimpleTextDocValuesWriter::EmptyDocValuesProducerAnonymousInnerClass::
    BinaryDocValuesAnonymousInnerClass::docID()
{
  return values->docID();
}

int64_t SimpleTextDocValuesWriter::EmptyDocValuesProducerAnonymousInnerClass::
    BinaryDocValuesAnonymousInnerClass::cost()
{
  return values->cost();
}

int SimpleTextDocValuesWriter::EmptyDocValuesProducerAnonymousInnerClass::
    BinaryDocValuesAnonymousInnerClass::advance(int target) 
{
  int doc = values->advance(target);
  setCurrentDoc();
  return doc;
}

bool SimpleTextDocValuesWriter::EmptyDocValuesProducerAnonymousInnerClass::
    BinaryDocValuesAnonymousInnerClass::advanceExact(int target) throw(
        IOException)
{
  if (values->advanceExact(target)) {
    setCurrentDoc();
    return true;
  }
  return false;
}

void SimpleTextDocValuesWriter::EmptyDocValuesProducerAnonymousInnerClass::
    BinaryDocValuesAnonymousInnerClass::setCurrentDoc() 
{
  if (docID() == DocIdSetIterator::NO_MORE_DOCS) {
    return;
  }
  builder->setLength(0);
  for (int i = 0, count = values->docValueCount(); i < count; ++i) {
    if (i > 0) {
      builder->append(L',');
    }
    // C++ TODO: There is no native C++ equivalent to 'toString':
    builder->append(Long::toString(values->nextValue()));
  }
  // C++ TODO: There is no native C++ equivalent to 'toString':
  binaryValue = make_shared<BytesRef>(builder->toString());
}

shared_ptr<BytesRef>
SimpleTextDocValuesWriter::EmptyDocValuesProducerAnonymousInnerClass::
    BinaryDocValuesAnonymousInnerClass::binaryValue() 
{
  return binaryValue;
}

void SimpleTextDocValuesWriter::addSortedSetField(
    shared_ptr<FieldInfo> field,
    shared_ptr<DocValuesProducer> valuesProducer) 
{
  assert(fieldSeen(field->name));
  assert(field->getDocValuesType() == DocValuesType::SORTED_SET);
  writeFieldEntry(field, DocValuesType::SORTED_SET);

  int64_t valueCount = 0;
  int maxLength = 0;
  shared_ptr<TermsEnum> terms =
      valuesProducer->getSortedSet(field)->termsEnum();
  for (shared_ptr<BytesRef> value = terms->next(); value != nullptr;
       value = terms->next()) {
    maxLength = max(maxLength, value->length);
    valueCount++;
  }

  // write numValues
  SimpleTextUtil::write(data, NUMVALUES);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  SimpleTextUtil::write(data, Long::toString(valueCount), scratch);
  SimpleTextUtil::writeNewline(data);

  // write maxLength
  SimpleTextUtil::write(data, MAXLENGTH);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  SimpleTextUtil::write(data, Integer::toString(maxLength), scratch);
  SimpleTextUtil::writeNewline(data);

  // C++ TODO: There is no native C++ equivalent to 'toString':
  int maxBytesLength = Integer::toString(maxLength)->length();
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  for (int i = 0; i < maxBytesLength; i++) {
    sb->append(L'0');
  }

  // write our pattern for encoding lengths
  SimpleTextUtil::write(data, PATTERN);
  SimpleTextUtil::write(data, sb->toString(), scratch);
  SimpleTextUtil::writeNewline(data);
  shared_ptr<DecimalFormat> *const encoder = make_shared<DecimalFormat>(
      sb->toString(), make_shared<DecimalFormatSymbols>(Locale::ROOT));

  // compute ord pattern: this is funny, we encode all values for all docs to
  // find the maximum length
  int maxOrdListLength = 0;
  shared_ptr<StringBuilder> sb2 = make_shared<StringBuilder>();
  shared_ptr<SortedSetDocValues> values = valuesProducer->getSortedSet(field);
  for (int doc = values->nextDoc(); doc != DocIdSetIterator::NO_MORE_DOCS;
       doc = values->nextDoc()) {
    sb2->setLength(0);
    for (int64_t ord = values->nextOrd();
         ord != SortedSetDocValues::NO_MORE_ORDS; ord = values->nextOrd()) {
      if (sb2->length() > 0) {
        sb2->append(L",");
      }
      // C++ TODO: There is no native C++ equivalent to 'toString':
      sb2->append(Long::toString(ord));
    }
    maxOrdListLength = max(maxOrdListLength, sb2->length());
  }

  sb2->setLength(0);
  for (int i = 0; i < maxOrdListLength; i++) {
    sb2->append(L'X');
  }

  // write our pattern for ord lists
  SimpleTextUtil::write(data, ORDPATTERN);
  SimpleTextUtil::write(data, sb2->toString(), scratch);
  SimpleTextUtil::writeNewline(data);

  // for asserts:
  int64_t valuesSeen = 0;

  terms = valuesProducer->getSortedSet(field)->termsEnum();
  for (shared_ptr<BytesRef> value = terms->next(); value != nullptr;
       value = terms->next()) {
    // write length
    SimpleTextUtil::write(data, LENGTH);
    SimpleTextUtil::write(data, encoder->format(value->length), scratch);
    SimpleTextUtil::writeNewline(data);

    // write bytes -- don't use SimpleText.write
    // because it escapes:
    data->writeBytes(value->bytes, value->offset, value->length);

    // pad to fit
    for (int i = value->length; i < maxLength; i++) {
      data->writeByte(static_cast<char>(L' '));
    }
    SimpleTextUtil::writeNewline(data);
    valuesSeen++;
    assert(valuesSeen <= valueCount);
  }

  assert(valuesSeen == valueCount);

  values = valuesProducer->getSortedSet(field);

  // write the ords for each doc comma-separated
  for (int i = 0; i < numDocs; ++i) {
    if (values->docID() < i) {
      values->nextDoc();
      assert(values->docID() >= i);
    }
    sb2->setLength(0);
    if (values->docID() == i) {
      for (int64_t ord = values->nextOrd();
           ord != SortedSetDocValues::NO_MORE_ORDS; ord = values->nextOrd()) {
        if (sb2->length() > 0) {
          sb2->append(L",");
        }
        // C++ TODO: There is no native C++ equivalent to 'toString':
        sb2->append(Long::toString(ord));
      }
    }
    // now pad to fit: these are numbers so spaces work well. reader calls
    // trim()
    int numPadding = maxOrdListLength - sb2->length();
    for (int j = 0; j < numPadding; j++) {
      sb2->append(L' ');
    }
    SimpleTextUtil::write(data, sb2->toString(), scratch);
    SimpleTextUtil::writeNewline(data);
  }
}

void SimpleTextDocValuesWriter::writeFieldEntry(
    shared_ptr<FieldInfo> field, DocValuesType type) 
{
  SimpleTextUtil::write(data, FIELD);
  SimpleTextUtil::write(data, field->name, scratch);
  SimpleTextUtil::writeNewline(data);

  SimpleTextUtil::write(data, TYPE);
  SimpleTextUtil::write(data, type.toString(), scratch);
  SimpleTextUtil::writeNewline(data);
}

SimpleTextDocValuesWriter::~SimpleTextDocValuesWriter()
{
  if (data != nullptr) {
    bool success = false;
    try {
      assert(!fieldsSeen->isEmpty());
      // TODO: sheisty to do this here?
      SimpleTextUtil::write(data, END);
      SimpleTextUtil::writeNewline(data);
      SimpleTextUtil::writeChecksum(data, scratch);
      success = true;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (success) {
        IOUtils::close({data});
      } else {
        IOUtils::closeWhileHandlingException({data});
      }
      data.reset();
    }
  }
}
} // namespace org::apache::lucene::codecs::simpletext