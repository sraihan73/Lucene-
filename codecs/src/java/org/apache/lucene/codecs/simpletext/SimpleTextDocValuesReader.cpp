using namespace std;

#include "SimpleTextDocValuesReader.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/BinaryDocValues.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/BufferedChecksumIndexInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/ChecksumIndexInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IndexInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/StringHelper.h"
#include "SimpleTextUtil.h"

namespace org::apache::lucene::codecs::simpletext
{
using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using namespace org::apache::lucene::index;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using BufferedChecksumIndexInput =
    org::apache::lucene::store::BufferedChecksumIndexInput;
using ChecksumIndexInput = org::apache::lucene::store::ChecksumIndexInput;
using IndexInput = org::apache::lucene::store::IndexInput;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
using StringHelper = org::apache::lucene::util::StringHelper;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextDocValuesWriter.END; import
//    static
//    org.apache.lucene.codecs.simpletext.SimpleTextDocValuesWriter.FIELD;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextDocValuesWriter.LENGTH;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextDocValuesWriter.MAXLENGTH;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextDocValuesWriter.MINVALUE;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextDocValuesWriter.NUMVALUES;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextDocValuesWriter.ORDPATTERN;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextDocValuesWriter.PATTERN;
//    import static
//    org.apache.lucene.codecs.simpletext.SimpleTextDocValuesWriter.TYPE;

SimpleTextDocValuesReader::SimpleTextDocValuesReader(
    shared_ptr<SegmentReadState> state, const wstring &ext) 
    : maxDoc(state->segmentInfo->maxDoc()),
      data(state->directory->openInput(
          IndexFileNames::segmentFileName(state->segmentInfo->name,
                                          state->segmentSuffix, ext),
          state->context))
{
  // System.out.println("dir=" + state.directory + " seg=" +
  // state.segmentInfo.name + " file=" +
  // IndexFileNames.segmentFileName(state.segmentInfo.name, state.segmentSuffix,
  // ext));
  while (true) {
    readLine();
    // System.out.println("READ field=" + scratch.utf8ToString());
    if (scratch->get().equals(END)) {
      break;
    }
    assert((startsWith(FIELD), scratch->get().utf8ToString()));
    wstring fieldName = stripPrefix(FIELD);
    // System.out.println("  field=" + fieldName);

    shared_ptr<OneField> field = make_shared<OneField>();
    fields.emplace(fieldName, field);

    readLine();
    assert((startsWith(TYPE), scratch->get().utf8ToString()));

    DocValuesType dvType = DocValuesType::valueOf(stripPrefix(TYPE));
    assert(dvType != DocValuesType::NONE);
    if (dvType == DocValuesType::NUMERIC) {
      readLine();
      assert((startsWith(MINVALUE), L"got " + scratch->get().utf8ToString() +
                                        L" field=" + fieldName + L" ext=" +
                                        ext));
      field->minValue =
          StringHelper::fromString<int64_t>(stripPrefix(MINVALUE));
      readLine();
      assert(startsWith(PATTERN));
      field->pattern = stripPrefix(PATTERN);
      field->dataStartFilePointer = data->getFilePointer();
      data->seek(data->getFilePointer() +
                 (1 + field->pattern.length() + 2) * maxDoc);
    } else if (dvType == DocValuesType::BINARY) {
      readLine();
      assert(startsWith(MAXLENGTH));
      field->maxLength = stoi(stripPrefix(MAXLENGTH));
      readLine();
      assert(startsWith(PATTERN));
      field->pattern = stripPrefix(PATTERN);
      field->dataStartFilePointer = data->getFilePointer();
      data->seek(data->getFilePointer() +
                 (9 + field->pattern.length() + field->maxLength + 2) * maxDoc);
    } else if (dvType == DocValuesType::SORTED ||
               dvType == DocValuesType::SORTED_SET) {
      readLine();
      assert(startsWith(NUMVALUES));
      field->numValues =
          StringHelper::fromString<int64_t>(stripPrefix(NUMVALUES));
      readLine();
      assert(startsWith(MAXLENGTH));
      field->maxLength = stoi(stripPrefix(MAXLENGTH));
      readLine();
      assert(startsWith(PATTERN));
      field->pattern = stripPrefix(PATTERN);
      readLine();
      assert(startsWith(ORDPATTERN));
      field->ordPattern = stripPrefix(ORDPATTERN);
      field->dataStartFilePointer = data->getFilePointer();
      data->seek(data->getFilePointer() +
                 (9 + field->pattern.length() + field->maxLength) *
                     field->numValues +
                 (1 + field->ordPattern.length()) * maxDoc);
    } else {
      throw make_shared<AssertionError>();
    }
  }

  // We should only be called from above if at least one
  // field has DVs:
  assert(!fields.empty());
}

shared_ptr<NumericDocValues> SimpleTextDocValuesReader::getNumeric(
    shared_ptr<FieldInfo> fieldInfo) 
{
  function<int64_t(int)> values = getNumericNonIterator(fieldInfo);
  if (values == nullptr) {
    return nullptr;
  } else {
    shared_ptr<DocValuesIterator> docsWithField =
        getNumericDocsWithField(fieldInfo);
    return make_shared<NumericDocValuesAnonymousInnerClass>(
        shared_from_this(), values, docsWithField);
  }
}

SimpleTextDocValuesReader::NumericDocValuesAnonymousInnerClass::
    NumericDocValuesAnonymousInnerClass(
        shared_ptr<SimpleTextDocValuesReader> outerInstance,
        function<int64_t(int)> &values,
        shared_ptr<org::apache::lucene::codecs::simpletext::
                       SimpleTextDocValuesReader::DocValuesIterator>
            docsWithField)
{
  this->outerInstance = outerInstance;
  this->values = values;
  this->docsWithField = docsWithField;
}

int SimpleTextDocValuesReader::NumericDocValuesAnonymousInnerClass::
    nextDoc() 
{
  return docsWithField->nextDoc();
}

int SimpleTextDocValuesReader::NumericDocValuesAnonymousInnerClass::docID()
{
  return docsWithField->docID();
}

int64_t SimpleTextDocValuesReader::NumericDocValuesAnonymousInnerClass::cost()
{
  return docsWithField->cost();
}

int SimpleTextDocValuesReader::NumericDocValuesAnonymousInnerClass::advance(
    int target) 
{
  return docsWithField->advance(target);
}

bool SimpleTextDocValuesReader::NumericDocValuesAnonymousInnerClass::
    advanceExact(int target) 
{
  return docsWithField->advanceExact(target);
}

int64_t SimpleTextDocValuesReader::NumericDocValuesAnonymousInnerClass::
    longValue() 
{
  return values(docsWithField->docID());
}

shared_ptr<internal> function <
    SimpleTextDocValuesReader::TangibleTemplonglong(int) 
{
  shared_ptr<OneField> *const field = fields[fieldInfo::name];
  assert(field != nullptr);

  // SegmentCoreReaders already verifies this field is
  // valid:
  assert(
      (field != nullptr, L"field=" + fieldInfo::name + L" fields=" + fields));

  shared_ptr<IndexInput> *const in_ = data->clone();
  shared_ptr<BytesRefBuilder> *const scratch = make_shared<BytesRefBuilder>();
  shared_ptr<DecimalFormat> *const decoder = make_shared<DecimalFormat>(
      field->pattern, make_shared<DecimalFormatSymbols>(Locale::ROOT));

  decoder->setParseBigDecimal(true);

  return [&](int docID) {
    try {
      // System.out.println(Thread.currentThread().getName() + ": get docID=" +
      // docID + " in=" + in);
      if (docID < 0 || docID >= maxDoc) {
        throw out_of_range(L"docID must be 0 .. " + to_wstring(maxDoc - 1) +
                           L"; got " + docID);
      }
      in_->seek(field->dataStartFilePointer +
                (1 + field->pattern.length() + 2) * docID);
      SimpleTextUtil::readLine(in_, scratch);
      // System.out.println("parsing delta: " + scratch.utf8ToString());
      BigDecimal bd;
      try {
        bd = static_cast<BigDecimal>(
            decoder->parse(scratch->get().utf8ToString()));
      } catch (const ParseException &pe) {
        throw make_shared<CorruptIndexException>(
            L"failed to parse BigDecimal value", in_, pe);
      }
      SimpleTextUtil::readLine(
          in_, scratch); // read the line telling us if it's real or not
      return static_cast<int64_t>(field->minValue)
          .add(bd.toBigIntegerExact())
          .longValue();
    } catch (const IOException &ioe) {
      throw runtime_error(ioe);
    }
  };
}

shared_ptr<DocValuesIterator>
SimpleTextDocValuesReader::getNumericDocsWithField(
    shared_ptr<FieldInfo> fieldInfo) 
{
  shared_ptr<OneField> *const field = fields[fieldInfo->name];
  shared_ptr<IndexInput> *const in_ = data->clone();
  shared_ptr<BytesRefBuilder> *const scratch = make_shared<BytesRefBuilder>();
  return make_shared<DocValuesIteratorAnonymousInnerClass>(shared_from_this(),
                                                           field, in_, scratch);
}

SimpleTextDocValuesReader::DocValuesIteratorAnonymousInnerClass::
    DocValuesIteratorAnonymousInnerClass(
        shared_ptr<SimpleTextDocValuesReader> outerInstance,
        shared_ptr<org::apache::lucene::codecs::simpletext::
                       SimpleTextDocValuesReader::OneField>
            field,
        shared_ptr<IndexInput> in_, shared_ptr<BytesRefBuilder> scratch)
{
  this->outerInstance = outerInstance;
  this->field = field;
  this->in_ = in_;
  this->scratch = scratch;
  doc = -1;
}

int SimpleTextDocValuesReader::DocValuesIteratorAnonymousInnerClass::
    nextDoc() 
{
  return advance(docID() + 1);
}

int SimpleTextDocValuesReader::DocValuesIteratorAnonymousInnerClass::docID()
{
  return doc;
}

int64_t
SimpleTextDocValuesReader::DocValuesIteratorAnonymousInnerClass::cost()
{
  return outerInstance->maxDoc;
}

int SimpleTextDocValuesReader::DocValuesIteratorAnonymousInnerClass::advance(
    int target) 
{
  for (int i = target; i < outerInstance->maxDoc; ++i) {
    in_->seek(field->dataStartFilePointer +
              (1 + field->pattern.length() + 2) * i);
    SimpleTextUtil::readLine(in_, scratch); // data
    SimpleTextUtil::readLine(in_, scratch); // 'T' or 'F'
    if (scratch->byteAt(0) == static_cast<char>(L'T')) {
      return doc = i;
    }
  }
  return doc = DocIdSetIterator::NO_MORE_DOCS;
}

bool SimpleTextDocValuesReader::DocValuesIteratorAnonymousInnerClass::
    advanceExact(int target) 
{
  this->doc = target;
  in_->seek(field->dataStartFilePointer +
            (1 + field->pattern.length() + 2) * target);
  SimpleTextUtil::readLine(in_, scratch); // data
  SimpleTextUtil::readLine(in_, scratch); // 'T' or 'F'
  return scratch->byteAt(0) == static_cast<char>(L'T');
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<BinaryDocValues> SimpleTextDocValuesReader::getBinary(
    shared_ptr<FieldInfo> fieldInfo) 
{
  shared_ptr<OneField> *const field = fields[fieldInfo->name];

  // SegmentCoreReaders already verifies this field is
  // valid:
  assert(field != nullptr);

  shared_ptr<IndexInput> *const in_ = data->clone();
  shared_ptr<BytesRefBuilder> *const scratch = make_shared<BytesRefBuilder>();
  shared_ptr<DecimalFormat> *const decoder = make_shared<DecimalFormat>(
      field->pattern, make_shared<DecimalFormatSymbols>(Locale::ROOT));

  shared_ptr<DocValuesIterator> docsWithField =
      getBinaryDocsWithField(fieldInfo);

  function<BytesRef *(int)> values = make_shared<functionAnonymousInnerClass>(
      shared_from_this(), int, field, in_, scratch, decoder);
  return make_shared<BinaryDocValuesAnonymousInnerClass>(shared_from_this(),
                                                         docsWithField, values);
}

SimpleTextDocValuesReader::functionAnonymousInnerClass::
    functionAnonymousInnerClass(
        shared_ptr<SimpleTextDocValuesReader> outerInstance,
        shared_ptr<UnknownType> int,
        shared_ptr<org::apache::lucene::codecs::simpletext::
                       SimpleTextDocValuesReader::OneField>
            field,
        shared_ptr<IndexInput> in_, shared_ptr<BytesRefBuilder> scratch,
        shared_ptr<DecimalFormat> decoder)
    : function<org::apache::lucene::util::BytesRef *(int)>(int)
{
  this->outerInstance = outerInstance;
  this->field = field;
  this->in_ = in_;
  this->scratch = scratch;
  this->decoder = decoder;
  term = make_shared<BytesRefBuilder>();
}

shared_ptr<BytesRef>
SimpleTextDocValuesReader::functionAnonymousInnerClass::apply(int docID)
{
  try {
    if (docID < 0 || docID >= outerInstance->maxDoc) {
      throw out_of_range(L"docID must be 0 .. " +
                         to_wstring(outerInstance->maxDoc - 1) + L"; got " +
                         to_wstring(docID));
    }
    in_->seek(field->dataStartFilePointer +
              (9 + field->pattern.length() + field->maxLength + 2) * docID);
    SimpleTextUtil::readLine(in_, scratch);
    assert((StringHelper::startsWith(scratch->get(), LENGTH)));
    int len;
    try {
      len = decoder
                ->parse(wstring(scratch->bytes(), LENGTH->length,
                                scratch->length() - LENGTH->length,
                                StandardCharsets::UTF_8))
                .intValue();
    } catch (const ParseException &pe) {
      throw make_shared<CorruptIndexException>(L"failed to parse int length",
                                               in_, pe);
    }
    term::grow(len);
    term->setLength(len);
    in_->readBytes(term::bytes(), 0, len);
    return term->get();
  } catch (const IOException &ioe) {
    throw runtime_error(ioe);
  }
}

SimpleTextDocValuesReader::BinaryDocValuesAnonymousInnerClass::
    BinaryDocValuesAnonymousInnerClass(
        shared_ptr<SimpleTextDocValuesReader> outerInstance,
        shared_ptr<org::apache::lucene::codecs::simpletext::
                       SimpleTextDocValuesReader::DocValuesIterator>
            docsWithField,
        function<BytesRef *(int)> &values)
{
  this->outerInstance = outerInstance;
  this->docsWithField = docsWithField;
  this->values = values;
}

int SimpleTextDocValuesReader::BinaryDocValuesAnonymousInnerClass::
    nextDoc() 
{
  return docsWithField->nextDoc();
}

int SimpleTextDocValuesReader::BinaryDocValuesAnonymousInnerClass::docID()
{
  return docsWithField->docID();
}

int64_t SimpleTextDocValuesReader::BinaryDocValuesAnonymousInnerClass::cost()
{
  return docsWithField->cost();
}

int SimpleTextDocValuesReader::BinaryDocValuesAnonymousInnerClass::advance(
    int target) 
{
  return docsWithField->advance(target);
}

bool SimpleTextDocValuesReader::BinaryDocValuesAnonymousInnerClass::
    advanceExact(int target) 
{
  return docsWithField->advanceExact(target);
}

shared_ptr<BytesRef> SimpleTextDocValuesReader::
    BinaryDocValuesAnonymousInnerClass::binaryValue() 
{
  return values(docsWithField->docID());
}

shared_ptr<DocValuesIterator> SimpleTextDocValuesReader::getBinaryDocsWithField(
    shared_ptr<FieldInfo> fieldInfo) 
{
  shared_ptr<OneField> *const field = fields[fieldInfo->name];
  shared_ptr<IndexInput> *const in_ = data->clone();
  shared_ptr<BytesRefBuilder> *const scratch = make_shared<BytesRefBuilder>();
  shared_ptr<DecimalFormat> *const decoder = make_shared<DecimalFormat>(
      field->pattern, make_shared<DecimalFormatSymbols>(Locale::ROOT));

  return make_shared<DocValuesIteratorAnonymousInnerClass2>(
      shared_from_this(), field, in_, scratch, decoder);
}

SimpleTextDocValuesReader::DocValuesIteratorAnonymousInnerClass2::
    DocValuesIteratorAnonymousInnerClass2(
        shared_ptr<SimpleTextDocValuesReader> outerInstance,
        shared_ptr<org::apache::lucene::codecs::simpletext::
                       SimpleTextDocValuesReader::OneField>
            field,
        shared_ptr<IndexInput> in_, shared_ptr<BytesRefBuilder> scratch,
        shared_ptr<DecimalFormat> decoder)
{
  this->outerInstance = outerInstance;
  this->field = field;
  this->in_ = in_;
  this->scratch = scratch;
  this->decoder = decoder;
  doc = -1;
}

int SimpleTextDocValuesReader::DocValuesIteratorAnonymousInnerClass2::
    nextDoc() 
{
  return advance(docID() + 1);
}

int SimpleTextDocValuesReader::DocValuesIteratorAnonymousInnerClass2::docID()
{
  return doc;
}

int64_t
SimpleTextDocValuesReader::DocValuesIteratorAnonymousInnerClass2::cost()
{
  return outerInstance->maxDoc;
}

int SimpleTextDocValuesReader::DocValuesIteratorAnonymousInnerClass2::advance(
    int target) 
{
  for (int i = target; i < outerInstance->maxDoc; ++i) {
    in_->seek(field->dataStartFilePointer +
              (9 + field->pattern.length() + field->maxLength + 2) * i);
    SimpleTextUtil::readLine(in_, scratch);
    assert((StringHelper::startsWith(scratch->get(), LENGTH)));
    int len;
    try {
      len = decoder
                ->parse(wstring(scratch->bytes(), LENGTH->length,
                                scratch->length() - LENGTH->length,
                                StandardCharsets::UTF_8))
                .intValue();
    } catch (const ParseException &pe) {
      throw make_shared<CorruptIndexException>(L"failed to parse int length",
                                               in_, pe);
    }
    // skip past bytes
    std::deque<char> bytes(len);
    in_->readBytes(bytes, 0, len);
    SimpleTextUtil::readLine(in_, scratch); // newline
    SimpleTextUtil::readLine(in_, scratch); // 'T' or 'F'
    if (scratch->byteAt(0) == static_cast<char>(L'T')) {
      return doc = i;
    }
  }
  return doc = DocIdSetIterator::NO_MORE_DOCS;
}

bool SimpleTextDocValuesReader::DocValuesIteratorAnonymousInnerClass2::
    advanceExact(int target) 
{
  this->doc = target;
  in_->seek(field->dataStartFilePointer +
            (9 + field->pattern.length() + field->maxLength + 2) * target);
  SimpleTextUtil::readLine(in_, scratch);
  assert((StringHelper::startsWith(scratch->get(), LENGTH)));
  int len;
  try {
    len = decoder
              ->parse(wstring(scratch->bytes(), LENGTH->length,
                              scratch->length() - LENGTH->length,
                              StandardCharsets::UTF_8))
              .intValue();
  } catch (const ParseException &pe) {
    throw make_shared<CorruptIndexException>(L"failed to parse int length", in_,
                                             pe);
  }
  // skip past bytes
  std::deque<char> bytes(len);
  in_->readBytes(bytes, 0, len);
  SimpleTextUtil::readLine(in_, scratch); // newline
  SimpleTextUtil::readLine(in_, scratch); // 'T' or 'F'
  return scratch->byteAt(0) == static_cast<char>(L'T');
}

shared_ptr<SortedDocValues> SimpleTextDocValuesReader::getSorted(
    shared_ptr<FieldInfo> fieldInfo) 
{
  shared_ptr<OneField> *const field = fields[fieldInfo->name];

  // SegmentCoreReaders already verifies this field is
  // valid:
  assert(field != nullptr);

  shared_ptr<IndexInput> *const in_ = data->clone();
  shared_ptr<BytesRefBuilder> *const scratch = make_shared<BytesRefBuilder>();
  shared_ptr<DecimalFormat> *const decoder = make_shared<DecimalFormat>(
      field->pattern, make_shared<DecimalFormatSymbols>(Locale::ROOT));
  shared_ptr<DecimalFormat> *const ordDecoder = make_shared<DecimalFormat>(
      field->ordPattern, make_shared<DecimalFormatSymbols>(Locale::ROOT));

  return make_shared<SortedDocValuesAnonymousInnerClass>(
      shared_from_this(), field, in_, scratch, decoder, ordDecoder);
}

SimpleTextDocValuesReader::SortedDocValuesAnonymousInnerClass::
    SortedDocValuesAnonymousInnerClass(
        shared_ptr<SimpleTextDocValuesReader> outerInstance,
        shared_ptr<org::apache::lucene::codecs::simpletext::
                       SimpleTextDocValuesReader::OneField>
            field,
        shared_ptr<IndexInput> in_, shared_ptr<BytesRefBuilder> scratch,
        shared_ptr<DecimalFormat> decoder, shared_ptr<DecimalFormat> ordDecoder)
{
  this->outerInstance = outerInstance;
  this->field = field;
  this->in_ = in_;
  this->scratch = scratch;
  this->decoder = decoder;
  this->ordDecoder = ordDecoder;
  doc = -1;
  term = make_shared<BytesRefBuilder>();
}

int SimpleTextDocValuesReader::SortedDocValuesAnonymousInnerClass::
    nextDoc() 
{
  return advance(docID() + 1);
}

int SimpleTextDocValuesReader::SortedDocValuesAnonymousInnerClass::docID()
{
  return doc;
}

int64_t SimpleTextDocValuesReader::SortedDocValuesAnonymousInnerClass::cost()
{
  return outerInstance->maxDoc;
}

int SimpleTextDocValuesReader::SortedDocValuesAnonymousInnerClass::advance(
    int target) 
{
  for (int i = target; i < outerInstance->maxDoc; ++i) {
    in_->seek(field->dataStartFilePointer +
              field->numValues *
                  (9 + field->pattern.length() + field->maxLength) +
              i * (1 + field->ordPattern.length()));
    SimpleTextUtil::readLine(in_, scratch);
    try {
      ord = static_cast<int>(
                ordDecoder->parse(scratch->get().utf8ToString()).longValue()) -
            1;
    } catch (const ParseException &pe) {
      throw make_shared<CorruptIndexException>(L"failed to parse ord", in_, pe);
    }
    if (ord >= 0) {
      return doc = i;
    }
  }
  return doc = DocIdSetIterator::NO_MORE_DOCS;
}

bool SimpleTextDocValuesReader::SortedDocValuesAnonymousInnerClass::
    advanceExact(int target) 
{
  this->doc = target;
  in_->seek(field->dataStartFilePointer +
            field->numValues *
                (9 + field->pattern.length() + field->maxLength) +
            target * (1 + field->ordPattern.length()));
  SimpleTextUtil::readLine(in_, scratch);
  try {
    ord = static_cast<int>(
              ordDecoder->parse(scratch->get().utf8ToString()).longValue()) -
          1;
  } catch (const ParseException &pe) {
    throw make_shared<CorruptIndexException>(L"failed to parse ord", in_, pe);
  }
  return ord >= 0;
}

int SimpleTextDocValuesReader::SortedDocValuesAnonymousInnerClass::ordValue()
{
  return ord;
}

shared_ptr<BytesRef>
SimpleTextDocValuesReader::SortedDocValuesAnonymousInnerClass::lookupOrd(
    int ord) 
{
  if (ord < 0 || ord >= field->numValues) {
    throw out_of_range(L"ord must be 0 .. " + to_wstring(field->numValues - 1) +
                       L"; got " + to_wstring(ord));
  }
  in_->seek(field->dataStartFilePointer +
            ord * (9 + field->pattern.length() + field->maxLength));
  SimpleTextUtil::readLine(in_, scratch);
  assert((StringHelper::startsWith(scratch->get(), LENGTH),
          L"got " + scratch->get().utf8ToString() + L" in=" + in_));
  int len;
  try {
    len = decoder
              ->parse(wstring(scratch->bytes(), LENGTH->length,
                              scratch->length() - LENGTH->length,
                              StandardCharsets::UTF_8))
              .intValue();
  } catch (const ParseException &pe) {
    throw make_shared<CorruptIndexException>(L"failed to parse int length", in_,
                                             pe);
  }
  term::grow(len);
  term->setLength(len);
  in_->readBytes(term::bytes(), 0, len);
  return term->get();
}

int SimpleTextDocValuesReader::SortedDocValuesAnonymousInnerClass::
    getValueCount()
{
  return static_cast<int>(field->numValues);
}

shared_ptr<SortedNumericDocValues> SimpleTextDocValuesReader::getSortedNumeric(
    shared_ptr<FieldInfo> field) 
{
  shared_ptr<BinaryDocValues> *const binary = getBinary(field);
  return make_shared<SortedNumericDocValuesAnonymousInnerClass>(
      shared_from_this(), binary);
}

SimpleTextDocValuesReader::SortedNumericDocValuesAnonymousInnerClass::
    SortedNumericDocValuesAnonymousInnerClass(
        shared_ptr<SimpleTextDocValuesReader> outerInstance,
        shared_ptr<org::apache::lucene::index::BinaryDocValues> binary)
{
  this->outerInstance = outerInstance;
  this->binary = binary;
}

int SimpleTextDocValuesReader::SortedNumericDocValuesAnonymousInnerClass::
    nextDoc() 
{
  int doc = binary->nextDoc();
  setCurrentDoc();
  return doc;
}

int SimpleTextDocValuesReader::SortedNumericDocValuesAnonymousInnerClass::
    docID()
{
  return binary->docID();
}

int64_t
SimpleTextDocValuesReader::SortedNumericDocValuesAnonymousInnerClass::cost()
{
  return binary->cost();
}

int SimpleTextDocValuesReader::SortedNumericDocValuesAnonymousInnerClass::
    advance(int target) 
{
  int doc = binary->advance(target);
  setCurrentDoc();
  return doc;
}

bool SimpleTextDocValuesReader::SortedNumericDocValuesAnonymousInnerClass::
    advanceExact(int target) 
{
  if (binary->advanceExact(target)) {
    setCurrentDoc();
    return true;
  }
  return false;
}

void SimpleTextDocValuesReader::SortedNumericDocValuesAnonymousInnerClass::
    setCurrentDoc() 
{
  if (docID() == DocIdSetIterator::NO_MORE_DOCS) {
    return;
  }
  wstring csv = binary->binaryValue()->utf8ToString();
  if (csv.length() == 0) {
    values = std::deque<int64_t>(0);
  } else {
    std::deque<wstring> s = csv.split(L",");
    values = std::deque<int64_t>(s.size());
    for (int i = 0; i < values->length; i++) {
      values[i] = StringHelper::fromString<int64_t>(s[i]);
    }
  }
  index = 0;
}

int64_t SimpleTextDocValuesReader::SortedNumericDocValuesAnonymousInnerClass::
    nextValue() 
{
  return values[index++];
}

int SimpleTextDocValuesReader::SortedNumericDocValuesAnonymousInnerClass::
    docValueCount()
{
  return values->length;
}

shared_ptr<SortedSetDocValues> SimpleTextDocValuesReader::getSortedSet(
    shared_ptr<FieldInfo> fieldInfo) 
{
  shared_ptr<OneField> *const field = fields[fieldInfo->name];

  // SegmentCoreReaders already verifies this field is
  // valid:
  assert(field != nullptr);

  shared_ptr<IndexInput> *const in_ = data->clone();
  shared_ptr<BytesRefBuilder> *const scratch = make_shared<BytesRefBuilder>();
  shared_ptr<DecimalFormat> *const decoder = make_shared<DecimalFormat>(
      field->pattern, make_shared<DecimalFormatSymbols>(Locale::ROOT));

  return make_shared<SortedSetDocValuesAnonymousInnerClass>(
      shared_from_this(), field, in_, scratch, decoder);
}

SimpleTextDocValuesReader::SortedSetDocValuesAnonymousInnerClass::
    SortedSetDocValuesAnonymousInnerClass(
        shared_ptr<SimpleTextDocValuesReader> outerInstance,
        shared_ptr<org::apache::lucene::codecs::simpletext::
                       SimpleTextDocValuesReader::OneField>
            field,
        shared_ptr<IndexInput> in_, shared_ptr<BytesRefBuilder> scratch,
        shared_ptr<DecimalFormat> decoder)
{
  this->outerInstance = outerInstance;
  this->field = field;
  this->in_ = in_;
  this->scratch = scratch;
  this->decoder = decoder;
  currentOrds = std::deque<wstring>(0);
  currentIndex = 0;
  term = make_shared<BytesRefBuilder>();
  doc = -1;
}

int SimpleTextDocValuesReader::SortedSetDocValuesAnonymousInnerClass::
    nextDoc() 
{
  return advance(doc + 1);
}

int SimpleTextDocValuesReader::SortedSetDocValuesAnonymousInnerClass::docID()
{
  return doc;
}

int64_t
SimpleTextDocValuesReader::SortedSetDocValuesAnonymousInnerClass::cost()
{
  return outerInstance->maxDoc;
}

int SimpleTextDocValuesReader::SortedSetDocValuesAnonymousInnerClass::advance(
    int target) 
{
  for (int i = target; i < outerInstance->maxDoc; ++i) {
    in_->seek(field->dataStartFilePointer +
              field->numValues *
                  (9 + field->pattern.length() + field->maxLength) +
              i * (1 + field->ordPattern.length()));
    SimpleTextUtil::readLine(in_, scratch);
    wstring ordList = scratch->get().utf8ToString()->trim();
    if (ordList.isEmpty() == false) {
      currentOrds = ordList.split(L",");
      currentIndex = 0;
      return doc = i;
    }
  }
  return doc = DocIdSetIterator::NO_MORE_DOCS;
}

bool SimpleTextDocValuesReader::SortedSetDocValuesAnonymousInnerClass::
    advanceExact(int target) 
{
  in_->seek(field->dataStartFilePointer +
            field->numValues *
                (9 + field->pattern.length() + field->maxLength) +
            target * (1 + field->ordPattern.length()));
  SimpleTextUtil::readLine(in_, scratch);
  wstring ordList = scratch->get().utf8ToString()->trim();
  doc = target;
  if (ordList.isEmpty() == false) {
    currentOrds = ordList.split(L",");
    currentIndex = 0;
    return true;
  }
  return false;
}

int64_t SimpleTextDocValuesReader::SortedSetDocValuesAnonymousInnerClass::
    nextOrd() 
{
  if (currentIndex == currentOrds->length) {
    return NO_MORE_ORDS;
  } else {
    return static_cast<int64_t>(currentOrds[currentIndex++]);
  }
}

shared_ptr<BytesRef>
SimpleTextDocValuesReader::SortedSetDocValuesAnonymousInnerClass::lookupOrd(
    int64_t ord) 
{
  if (ord < 0 || ord >= field->numValues) {
    throw out_of_range(L"ord must be 0 .. " + to_wstring(field->numValues - 1) +
                       L"; got " + to_wstring(ord));
  }
  in_->seek(field->dataStartFilePointer +
            ord * (9 + field->pattern.length() + field->maxLength));
  SimpleTextUtil::readLine(in_, scratch);
  assert((StringHelper::startsWith(scratch->get(), LENGTH),
          L"got " + scratch->get().utf8ToString() + L" in=" + in_));
  int len;
  try {
    len = decoder
              ->parse(wstring(scratch->bytes(), LENGTH->length,
                              scratch->length() - LENGTH->length,
                              StandardCharsets::UTF_8))
              .intValue();
  } catch (const ParseException &pe) {
    throw make_shared<CorruptIndexException>(L"failed to parse int length", in_,
                                             pe);
  }
  term::grow(len);
  term->setLength(len);
  in_->readBytes(term::bytes(), 0, len);
  return term->get();
}

int64_t SimpleTextDocValuesReader::SortedSetDocValuesAnonymousInnerClass::
    getValueCount()
{
  return field->numValues;
}

SimpleTextDocValuesReader::~SimpleTextDocValuesReader() { delete data; }

void SimpleTextDocValuesReader::readLine() 
{
  SimpleTextUtil::readLine(data, scratch);
  // System.out.println("line: " + scratch.utf8ToString());
}

bool SimpleTextDocValuesReader::startsWith(shared_ptr<BytesRef> prefix)
{
  return StringHelper::startsWith(scratch->get(), prefix);
}

wstring SimpleTextDocValuesReader::stripPrefix(shared_ptr<BytesRef> prefix)
{
  return wstring(scratch->bytes(), prefix->length,
                 scratch->length() - prefix->length, StandardCharsets::UTF_8);
}

int64_t SimpleTextDocValuesReader::ramBytesUsed()
{
  return BASE_RAM_BYTES_USED + RamUsageEstimator::sizeOf(scratch->bytes()) +
         fields.size() * (RamUsageEstimator::NUM_BYTES_OBJECT_REF * 2LL +
                          OneField::BASE_RAM_BYTES_USED);
}

wstring SimpleTextDocValuesReader::toString()
{
  return getClass().getSimpleName() + L"(fields=" + fields.size() + L")";
}

void SimpleTextDocValuesReader::checkIntegrity() 
{
  shared_ptr<BytesRefBuilder> scratch = make_shared<BytesRefBuilder>();
  shared_ptr<IndexInput> clone = data->clone();
  clone->seek(0);
  // checksum is fixed-width encoded with 20 bytes, plus 1 byte for newline (the
  // space is included in SimpleTextUtil.CHECKSUM):
  int64_t footerStartPos =
      data->length() - (SimpleTextUtil::CHECKSUM->length + 21);
  shared_ptr<ChecksumIndexInput> input =
      make_shared<BufferedChecksumIndexInput>(clone);
  while (true) {
    SimpleTextUtil::readLine(input, scratch);
    if (input->getFilePointer() >= footerStartPos) {
      // Make sure we landed at precisely the right location:
      if (input->getFilePointer() != footerStartPos) {
        throw make_shared<CorruptIndexException>(
            L"SimpleText failure: footer does not start at expected position "
            L"current=" +
                to_wstring(input->getFilePointer()) + L" vs expected=" +
                to_wstring(footerStartPos),
            input);
      }
      SimpleTextUtil::checkFooter(input);
      break;
    }
  }
}
} // namespace org::apache::lucene::codecs::simpletext