using namespace std;

#include "LegacyDocValuesIterables.h"
#include "../index/BinaryDocValues.h"
#include "../index/FieldInfo.h"
#include "../index/NumericDocValues.h"
#include "../index/SortedDocValues.h"
#include "../index/SortedNumericDocValues.h"
#include "../index/SortedSetDocValues.h"
#include "../util/BytesRef.h"
#include "DocValuesProducer.h"
#include "NormsProducer.h"

namespace org::apache::lucene::codecs
{
using BinaryDocValues = org::apache::lucene::index::BinaryDocValues;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using SortedNumericDocValues =
    org::apache::lucene::index::SortedNumericDocValues;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using BytesRef = org::apache::lucene::util::BytesRef;
//    import static org.apache.lucene.index.SortedSetDocValues.NO_MORE_ORDS;
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

LegacyDocValuesIterables::LegacyDocValuesIterables()
{
  // no
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public static
// Iterable<org.apache.lucene.util.BytesRef> valuesIterable(final
// org.apache.lucene.index.SortedDocValues values)
deque<std::shared_ptr<BytesRef>>
LegacyDocValuesIterables::valuesIterable(shared_ptr<SortedDocValues> values)
{
  return make_shared<IterableAnonymousInnerClass>(values);
}

LegacyDocValuesIterables::IterableAnonymousInnerClass::
    IterableAnonymousInnerClass(shared_ptr<SortedDocValues> values)
{
  this->values = values;
}

shared_ptr<Iterator<std::shared_ptr<BytesRef>>>
LegacyDocValuesIterables::IterableAnonymousInnerClass::iterator()
{
  return make_shared<IteratorAnonymousInnerClass>(shared_from_this());
}

LegacyDocValuesIterables::IterableAnonymousInnerClass::
    IteratorAnonymousInnerClass::IteratorAnonymousInnerClass(
        shared_ptr<IterableAnonymousInnerClass> outerInstance)
{
  this->outerInstance = outerInstance;
}

bool LegacyDocValuesIterables::IterableAnonymousInnerClass::
    IteratorAnonymousInnerClass::hasNext()
{
  return nextOrd < outerInstance->values.getValueCount();
}

shared_ptr<BytesRef> LegacyDocValuesIterables::IterableAnonymousInnerClass::
    IteratorAnonymousInnerClass::next()
{
  try {
    return outerInstance->values.lookupOrd(nextOrd++);
  } catch (const IOException &e) {
    throw runtime_error(e);
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public static
// Iterable<org.apache.lucene.util.BytesRef> valuesIterable(final
// org.apache.lucene.index.SortedSetDocValues values)
deque<std::shared_ptr<BytesRef>>
LegacyDocValuesIterables::valuesIterable(shared_ptr<SortedSetDocValues> values)
{
  return make_shared<IterableAnonymousInnerClass2>(values);
}

LegacyDocValuesIterables::IterableAnonymousInnerClass2::
    IterableAnonymousInnerClass2(shared_ptr<SortedSetDocValues> values)
{
  this->values = values;
}

shared_ptr<Iterator<std::shared_ptr<BytesRef>>>
LegacyDocValuesIterables::IterableAnonymousInnerClass2::iterator()
{
  return make_shared<IteratorAnonymousInnerClass2>(shared_from_this());
}

LegacyDocValuesIterables::IterableAnonymousInnerClass2::
    IteratorAnonymousInnerClass2::IteratorAnonymousInnerClass2(
        shared_ptr<IterableAnonymousInnerClass2> outerInstance)
{
  this->outerInstance = outerInstance;
}

bool LegacyDocValuesIterables::IterableAnonymousInnerClass2::
    IteratorAnonymousInnerClass2::hasNext()
{
  return nextOrd < outerInstance->values.getValueCount();
}

shared_ptr<BytesRef> LegacyDocValuesIterables::IterableAnonymousInnerClass2::
    IteratorAnonymousInnerClass2::next()
{
  try {
    return outerInstance->values.lookupOrd(nextOrd++);
  } catch (const IOException &e) {
    throw runtime_error(e);
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public static Iterable<Number>
// sortedOrdIterable(final DocValuesProducer valuesProducer,
// org.apache.lucene.index.FieldInfo fieldInfo, int maxDoc)
deque<std::shared_ptr<Number>> LegacyDocValuesIterables::sortedOrdIterable(
    shared_ptr<DocValuesProducer> valuesProducer,
    shared_ptr<FieldInfo> fieldInfo, int maxDoc)
{
  return make_shared<IterableAnonymousInnerClass3>(valuesProducer, fieldInfo,
                                                   maxDoc);
}

LegacyDocValuesIterables::IterableAnonymousInnerClass3::
    IterableAnonymousInnerClass3(
        shared_ptr<org::apache::lucene::codecs::DocValuesProducer>
            valuesProducer,
        shared_ptr<FieldInfo> fieldInfo, int maxDoc)
{
  this->valuesProducer = valuesProducer;
  this->fieldInfo = fieldInfo;
  this->maxDoc = maxDoc;
}

shared_ptr<Iterator<std::shared_ptr<Number>>>
LegacyDocValuesIterables::IterableAnonymousInnerClass3::iterator()
{

  shared_ptr<SortedDocValues> *const values;
  try {
    values = valuesProducer->getSorted(fieldInfo);
  } catch (const IOException &ioe) {
    throw runtime_error(ioe);
  }

  return make_shared<IteratorAnonymousInnerClass3>(shared_from_this(), values);
}

LegacyDocValuesIterables::IterableAnonymousInnerClass3::
    IteratorAnonymousInnerClass3::IteratorAnonymousInnerClass3(
        shared_ptr<IterableAnonymousInnerClass3> outerInstance,
        shared_ptr<SortedDocValues> values)
{
  this->outerInstance = outerInstance;
  this->values = values;
}

bool LegacyDocValuesIterables::IterableAnonymousInnerClass3::
    IteratorAnonymousInnerClass3::hasNext()
{
  return nextDocID < outerInstance->maxDoc;
}

shared_ptr<Number> LegacyDocValuesIterables::IterableAnonymousInnerClass3::
    IteratorAnonymousInnerClass3::next()
{
  try {
    if (nextDocID > values->docID()) {
      values->nextDoc();
    }
    int result;
    if (nextDocID == values->docID()) {
      result = values->ordValue();
    } else {
      result = -1;
    }
    nextDocID++;
    return result;
  } catch (const IOException &ioe) {
    throw runtime_error(ioe);
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public static Iterable<Number>
// sortedSetOrdCountIterable(final DocValuesProducer valuesProducer, final
// org.apache.lucene.index.FieldInfo fieldInfo, final int maxDoc)
deque<std::shared_ptr<Number>>
LegacyDocValuesIterables::sortedSetOrdCountIterable(
    shared_ptr<DocValuesProducer> valuesProducer,
    shared_ptr<FieldInfo> fieldInfo, int const maxDoc)
{

  return make_shared<IterableAnonymousInnerClass4>(valuesProducer, fieldInfo,
                                                   maxDoc);
}

LegacyDocValuesIterables::IterableAnonymousInnerClass4::
    IterableAnonymousInnerClass4(
        shared_ptr<org::apache::lucene::codecs::DocValuesProducer>
            valuesProducer,
        shared_ptr<FieldInfo> fieldInfo, int maxDoc)
{
  this->valuesProducer = valuesProducer;
  this->fieldInfo = fieldInfo;
  this->maxDoc = maxDoc;
}

shared_ptr<Iterator<std::shared_ptr<Number>>>
LegacyDocValuesIterables::IterableAnonymousInnerClass4::iterator()
{

  shared_ptr<SortedSetDocValues> *const values;
  try {
    values = valuesProducer->getSortedSet(fieldInfo);
  } catch (const IOException &ioe) {
    throw runtime_error(ioe);
  }

  return make_shared<IteratorAnonymousInnerClass4>(shared_from_this(), values);
}

LegacyDocValuesIterables::IterableAnonymousInnerClass4::
    IteratorAnonymousInnerClass4::IteratorAnonymousInnerClass4(
        shared_ptr<IterableAnonymousInnerClass4> outerInstance,
        shared_ptr<SortedSetDocValues> values)
{
  this->outerInstance = outerInstance;
  this->values = values;
}

bool LegacyDocValuesIterables::IterableAnonymousInnerClass4::
    IteratorAnonymousInnerClass4::hasNext()
{
  return nextDocID < outerInstance->maxDoc;
}

shared_ptr<Number> LegacyDocValuesIterables::IterableAnonymousInnerClass4::
    IteratorAnonymousInnerClass4::next()
{
  try {
    if (nextDocID > values->docID()) {
      if (values->nextDoc() != NO_MORE_DOCS) {
        ordCount = 0;
        while (values->nextOrd() != SortedSetDocValues::NO_MORE_ORDS) {
          ordCount++;
        }
      }
    }
    int result;
    if (nextDocID == values->docID()) {
      result = ordCount;
    } else {
      result = 0;
    }
    nextDocID++;
    return result;
  } catch (const IOException &ioe) {
    throw runtime_error(ioe);
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public static Iterable<Number>
// sortedSetOrdsIterable(final DocValuesProducer valuesProducer, final
// org.apache.lucene.index.FieldInfo fieldInfo)
deque<std::shared_ptr<Number>> LegacyDocValuesIterables::sortedSetOrdsIterable(
    shared_ptr<DocValuesProducer> valuesProducer,
    shared_ptr<FieldInfo> fieldInfo)
{

  return make_shared<IterableAnonymousInnerClass5>(valuesProducer, fieldInfo);
}

LegacyDocValuesIterables::IterableAnonymousInnerClass5::
    IterableAnonymousInnerClass5(
        shared_ptr<org::apache::lucene::codecs::DocValuesProducer>
            valuesProducer,
        shared_ptr<FieldInfo> fieldInfo)
{
  this->valuesProducer = valuesProducer;
  this->fieldInfo = fieldInfo;
}

shared_ptr<Iterator<std::shared_ptr<Number>>>
LegacyDocValuesIterables::IterableAnonymousInnerClass5::iterator()
{

  shared_ptr<SortedSetDocValues> *const values;
  try {
    values = valuesProducer->getSortedSet(fieldInfo);
  } catch (const IOException &ioe) {
    throw runtime_error(ioe);
  }

  return make_shared<IteratorAnonymousInnerClass5>(shared_from_this(), values);
}

LegacyDocValuesIterables::IterableAnonymousInnerClass5::
    IteratorAnonymousInnerClass5::IteratorAnonymousInnerClass5(
        shared_ptr<IterableAnonymousInnerClass5> outerInstance,
        shared_ptr<SortedSetDocValues> values)
{
  this->outerInstance = outerInstance;
  this->values = values;
}

void LegacyDocValuesIterables::IterableAnonymousInnerClass5::
    IteratorAnonymousInnerClass5::setNext()
{
  try {
    if (nextIsSet == false) {
      if (values->docID() == -1) {
        values->nextDoc();
      }
      while (true) {
        if (values->docID() == NO_MORE_DOCS) {
          nextOrd = -1;
          break;
        }
        nextOrd = values->nextOrd();
        if (nextOrd != -1) {
          break;
        }
        values->nextDoc();
      }
      nextIsSet = true;
    }
  } catch (const IOException &ioe) {
    throw runtime_error(ioe);
  }
}

bool LegacyDocValuesIterables::IterableAnonymousInnerClass5::
    IteratorAnonymousInnerClass5::hasNext()
{
  setNext();
  return nextOrd != -1;
}

shared_ptr<Number> LegacyDocValuesIterables::IterableAnonymousInnerClass5::
    IteratorAnonymousInnerClass5::next()
{
  setNext();
  assert(nextOrd != -1);
  nextIsSet = false;
  return nextOrd;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public static Iterable<Number>
// sortedNumericToDocCount(final DocValuesProducer valuesProducer, final
// org.apache.lucene.index.FieldInfo fieldInfo, int maxDoc)
deque<std::shared_ptr<Number>>
LegacyDocValuesIterables::sortedNumericToDocCount(
    shared_ptr<DocValuesProducer> valuesProducer,
    shared_ptr<FieldInfo> fieldInfo, int maxDoc)
{
  return make_shared<IterableAnonymousInnerClass6>(valuesProducer, fieldInfo,
                                                   maxDoc);
}

LegacyDocValuesIterables::IterableAnonymousInnerClass6::
    IterableAnonymousInnerClass6(
        shared_ptr<org::apache::lucene::codecs::DocValuesProducer>
            valuesProducer,
        shared_ptr<FieldInfo> fieldInfo, int maxDoc)
{
  this->valuesProducer = valuesProducer;
  this->fieldInfo = fieldInfo;
  this->maxDoc = maxDoc;
}

shared_ptr<Iterator<std::shared_ptr<Number>>>
LegacyDocValuesIterables::IterableAnonymousInnerClass6::iterator()
{

  shared_ptr<SortedNumericDocValues> *const values;
  try {
    values = valuesProducer->getSortedNumeric(fieldInfo);
  } catch (const IOException &ioe) {
    throw runtime_error(ioe);
  }

  return make_shared<IteratorAnonymousInnerClass6>(shared_from_this(), values);
}

LegacyDocValuesIterables::IterableAnonymousInnerClass6::
    IteratorAnonymousInnerClass6::IteratorAnonymousInnerClass6(
        shared_ptr<IterableAnonymousInnerClass6> outerInstance,
        shared_ptr<SortedNumericDocValues> values)
{
  this->outerInstance = outerInstance;
  this->values = values;
}

bool LegacyDocValuesIterables::IterableAnonymousInnerClass6::
    IteratorAnonymousInnerClass6::hasNext()
{
  return nextDocID < outerInstance->maxDoc;
}

shared_ptr<Number> LegacyDocValuesIterables::IterableAnonymousInnerClass6::
    IteratorAnonymousInnerClass6::next()
{
  try {
    if (nextDocID > values->docID()) {
      values->nextDoc();
    }
    int result;
    if (nextDocID == values->docID()) {
      result = values->docValueCount();
    } else {
      result = 0;
    }
    nextDocID++;
    return result;
  } catch (const IOException &ioe) {
    throw runtime_error(ioe);
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public static Iterable<Number>
// sortedNumericToValues(final DocValuesProducer valuesProducer, final
// org.apache.lucene.index.FieldInfo fieldInfo)
deque<std::shared_ptr<Number>> LegacyDocValuesIterables::sortedNumericToValues(
    shared_ptr<DocValuesProducer> valuesProducer,
    shared_ptr<FieldInfo> fieldInfo)
{
  return make_shared<IterableAnonymousInnerClass7>(valuesProducer, fieldInfo);
}

LegacyDocValuesIterables::IterableAnonymousInnerClass7::
    IterableAnonymousInnerClass7(
        shared_ptr<org::apache::lucene::codecs::DocValuesProducer>
            valuesProducer,
        shared_ptr<FieldInfo> fieldInfo)
{
  this->valuesProducer = valuesProducer;
  this->fieldInfo = fieldInfo;
}

shared_ptr<Iterator<std::shared_ptr<Number>>>
LegacyDocValuesIterables::IterableAnonymousInnerClass7::iterator()
{

  shared_ptr<SortedNumericDocValues> *const values;
  try {
    values = valuesProducer->getSortedNumeric(fieldInfo);
  } catch (const IOException &ioe) {
    throw runtime_error(ioe);
  }

  return make_shared<IteratorAnonymousInnerClass7>(shared_from_this(), values);
}

LegacyDocValuesIterables::IterableAnonymousInnerClass7::
    IteratorAnonymousInnerClass7::IteratorAnonymousInnerClass7(
        shared_ptr<IterableAnonymousInnerClass7> outerInstance,
        shared_ptr<SortedNumericDocValues> values)
{
  this->outerInstance = outerInstance;
  this->values = values;
}

void LegacyDocValuesIterables::IterableAnonymousInnerClass7::
    IteratorAnonymousInnerClass7::setNext()
{
  try {
    if (nextIsSet == false) {
      if (upto == nextCount) {
        values->nextDoc();
        if (values->docID() == NO_MORE_DOCS) {
          nextCount = 0;
          nextIsSet = false;
          return;
        } else {
          nextCount = values->docValueCount();
        }
        upto = 0;
      }
      nextValue = values->nextValue();
      upto++;
      nextIsSet = true;
    }
  } catch (const IOException &ioe) {
    throw runtime_error(ioe);
  }
}

bool LegacyDocValuesIterables::IterableAnonymousInnerClass7::
    IteratorAnonymousInnerClass7::hasNext()
{
  setNext();
  return nextCount != 0;
}

shared_ptr<Number> LegacyDocValuesIterables::IterableAnonymousInnerClass7::
    IteratorAnonymousInnerClass7::next()
{
  setNext();
  assert(nextCount != 0);
  nextIsSet = false;
  return nextValue;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public static Iterable<Number> normsIterable(final
// org.apache.lucene.index.FieldInfo field, final NormsProducer normsProducer,
// final int maxDoc)
deque<std::shared_ptr<Number>>
LegacyDocValuesIterables::normsIterable(shared_ptr<FieldInfo> field,
                                        shared_ptr<NormsProducer> normsProducer,
                                        int const maxDoc)
{

  return make_shared<IterableAnonymousInnerClass8>(field, normsProducer,
                                                   maxDoc);
}

LegacyDocValuesIterables::IterableAnonymousInnerClass8::
    IterableAnonymousInnerClass8(
        shared_ptr<FieldInfo> field,
        shared_ptr<org::apache::lucene::codecs::NormsProducer> normsProducer,
        int maxDoc)
{
  this->field = field;
  this->normsProducer = normsProducer;
  this->maxDoc = maxDoc;
}

shared_ptr<Iterator<std::shared_ptr<Number>>>
LegacyDocValuesIterables::IterableAnonymousInnerClass8::iterator()
{

  shared_ptr<NumericDocValues> *const values;
  try {
    values = normsProducer->getNorms(field);
  } catch (const IOException &ioe) {
    throw runtime_error(ioe);
  }

  return make_shared<IteratorAnonymousInnerClass8>(shared_from_this(), values);
}

LegacyDocValuesIterables::IterableAnonymousInnerClass8::
    IteratorAnonymousInnerClass8::IteratorAnonymousInnerClass8(
        shared_ptr<IterableAnonymousInnerClass8> outerInstance,
        shared_ptr<NumericDocValues> values)
{
  this->outerInstance = outerInstance;
  this->values = values;
  docIDUpto = -1;
}

bool LegacyDocValuesIterables::IterableAnonymousInnerClass8::
    IteratorAnonymousInnerClass8::hasNext()
{
  return docIDUpto + 1 < outerInstance->maxDoc;
}

shared_ptr<Number> LegacyDocValuesIterables::IterableAnonymousInnerClass8::
    IteratorAnonymousInnerClass8::next()
{
  docIDUpto++;
  if (docIDUpto > values->docID()) {
    try {
      values->nextDoc();
    } catch (const IOException &ioe) {
      throw runtime_error(ioe);
    }
  }
  shared_ptr<Number> result;
  if (docIDUpto == values->docID()) {
    try {
      result = values->longValue();
    } catch (const IOException &ioe) {
      throw runtime_error(ioe);
    }
  } else {
    // Unlike NumericDocValues, norms used to return 0 for missing values:
    result = 0;
  }
  return result;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public static
// Iterable<org.apache.lucene.util.BytesRef> binaryIterable(final
// org.apache.lucene.index.FieldInfo field, final DocValuesProducer
// valuesProducer, final int maxDoc)
deque<std::shared_ptr<BytesRef>> LegacyDocValuesIterables::binaryIterable(
    shared_ptr<FieldInfo> field, shared_ptr<DocValuesProducer> valuesProducer,
    int const maxDoc)
{
  return make_shared<IterableAnonymousInnerClass9>(field, valuesProducer,
                                                   maxDoc);
}

LegacyDocValuesIterables::IterableAnonymousInnerClass9::
    IterableAnonymousInnerClass9(
        shared_ptr<FieldInfo> field,
        shared_ptr<org::apache::lucene::codecs::DocValuesProducer>
            valuesProducer,
        int maxDoc)
{
  this->field = field;
  this->valuesProducer = valuesProducer;
  this->maxDoc = maxDoc;
}

shared_ptr<Iterator<std::shared_ptr<BytesRef>>>
LegacyDocValuesIterables::IterableAnonymousInnerClass9::iterator()
{

  shared_ptr<BinaryDocValues> *const values;
  try {
    values = valuesProducer->getBinary(field);
  } catch (const IOException &ioe) {
    throw runtime_error(ioe);
  }

  return make_shared<IteratorAnonymousInnerClass9>(shared_from_this(), values);
}

LegacyDocValuesIterables::IterableAnonymousInnerClass9::
    IteratorAnonymousInnerClass9::IteratorAnonymousInnerClass9(
        shared_ptr<IterableAnonymousInnerClass9> outerInstance,
        shared_ptr<BinaryDocValues> values)
{
  this->outerInstance = outerInstance;
  this->values = values;
  docIDUpto = -1;
}

bool LegacyDocValuesIterables::IterableAnonymousInnerClass9::
    IteratorAnonymousInnerClass9::hasNext()
{
  return docIDUpto + 1 < outerInstance->maxDoc;
}

shared_ptr<BytesRef> LegacyDocValuesIterables::IterableAnonymousInnerClass9::
    IteratorAnonymousInnerClass9::next()
{
  docIDUpto++;
  if (docIDUpto > values->docID()) {
    try {
      values->nextDoc();
    } catch (const IOException &ioe) {
      throw runtime_error(ioe);
    }
  }
  shared_ptr<BytesRef> result;
  if (docIDUpto == values->docID()) {
    try {
      result = values->binaryValue();
    } catch (const IOException &e) {
      throw runtime_error(e);
    }
  } else {
    result.reset();
  }
  return result;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public static Iterable<Number>
// numericIterable(final org.apache.lucene.index.FieldInfo field, final
// DocValuesProducer valuesProducer, final int maxDoc)
deque<std::shared_ptr<Number>> LegacyDocValuesIterables::numericIterable(
    shared_ptr<FieldInfo> field, shared_ptr<DocValuesProducer> valuesProducer,
    int const maxDoc)
{
  return make_shared<IterableAnonymousInnerClass10>(field, valuesProducer,
                                                    maxDoc);
}

LegacyDocValuesIterables::IterableAnonymousInnerClass10::
    IterableAnonymousInnerClass10(
        shared_ptr<FieldInfo> field,
        shared_ptr<org::apache::lucene::codecs::DocValuesProducer>
            valuesProducer,
        int maxDoc)
{
  this->field = field;
  this->valuesProducer = valuesProducer;
  this->maxDoc = maxDoc;
}

shared_ptr<Iterator<std::shared_ptr<Number>>>
LegacyDocValuesIterables::IterableAnonymousInnerClass10::iterator()
{

  shared_ptr<NumericDocValues> *const values;
  try {
    values = valuesProducer->getNumeric(field);
  } catch (const IOException &ioe) {
    throw runtime_error(ioe);
  }

  return make_shared<IteratorAnonymousInnerClass10>(shared_from_this(), values);
}

LegacyDocValuesIterables::IterableAnonymousInnerClass10::
    IteratorAnonymousInnerClass10::IteratorAnonymousInnerClass10(
        shared_ptr<IterableAnonymousInnerClass10> outerInstance,
        shared_ptr<NumericDocValues> values)
{
  this->outerInstance = outerInstance;
  this->values = values;
  docIDUpto = -1;
}

bool LegacyDocValuesIterables::IterableAnonymousInnerClass10::
    IteratorAnonymousInnerClass10::hasNext()
{
  return docIDUpto + 1 < outerInstance->maxDoc;
}

shared_ptr<Number> LegacyDocValuesIterables::IterableAnonymousInnerClass10::
    IteratorAnonymousInnerClass10::next()
{
  docIDUpto++;
  if (docIDUpto > values->docID()) {
    try {
      values->nextDoc();
    } catch (const IOException &ioe) {
      throw runtime_error(ioe);
    }
  }
  shared_ptr<Number> result;
  if (docIDUpto == values->docID()) {
    try {
      result = values->longValue();
    } catch (const IOException &ioe) {
      throw runtime_error(ioe);
    }
  } else {
    result.reset();
  }
  return result;
}
} // namespace org::apache::lucene::codecs