using namespace std;

#include "FieldComparator.h"

namespace org::apache::lucene::search
{
using BinaryDocValues = org::apache::lucene::index::BinaryDocValues;
using DocValues = org::apache::lucene::index::DocValues;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;

FieldComparator<T>::DoubleComparator::DoubleComparator(
    int numHits, const wstring &field, optional<double> &missingValue)
    : NumericComparator<double>(field,
                                missingValue != nullptr ? missingValue : 0.0),
      values(std::deque<double>(numHits))
{
}

double
FieldComparator<T>::DoubleComparator::getValueForDoc(int doc) 
{
  if (currentReaderValues->advanceExact(doc)) {
    return Double::longBitsToDouble(currentReaderValues->longValue());
  } else {
    return missingValue;
  }
}

int FieldComparator<T>::DoubleComparator::compare(int slot1, int slot2)
{
  return Double::compare(values[slot1], values[slot2]);
}

int FieldComparator<T>::DoubleComparator::compareBottom(int doc) throw(
    IOException)
{
  return Double::compare(bottom, getValueForDoc(doc));
}

void FieldComparator<T>::DoubleComparator::copy(int slot,
                                                int doc) 
{
  values[slot] = getValueForDoc(doc);
}

void FieldComparator<T>::DoubleComparator::setBottom(int const bottom)
{
  this->bottom = values[bottom];
}

void FieldComparator<T>::DoubleComparator::setTopValue(optional<double> &value)
{
  topValue = value;
}

optional<double> FieldComparator<T>::DoubleComparator::value(int slot)
{
  return static_cast<Double>(values[slot]);
}

int FieldComparator<T>::DoubleComparator::compareTop(int doc) 
{
  return Double::compare(topValue, getValueForDoc(doc));
}

FieldComparator<T>::FloatComparator::FloatComparator(
    int numHits, const wstring &field, optional<float> &missingValue)
    : NumericComparator<float>(field,
                               missingValue != nullptr ? missingValue : 0.0f),
      values(std::deque<float>(numHits))
{
}

float FieldComparator<T>::FloatComparator::getValueForDoc(int doc) throw(
    IOException)
{
  if (currentReaderValues->advanceExact(doc)) {
    return Float::intBitsToFloat(
        static_cast<int>(currentReaderValues->longValue()));
  } else {
    return missingValue;
  }
}

int FieldComparator<T>::FloatComparator::compare(int slot1, int slot2)
{
  return Float::compare(values[slot1], values[slot2]);
}

int FieldComparator<T>::FloatComparator::compareBottom(int doc) throw(
    IOException)
{
  return Float::compare(bottom, getValueForDoc(doc));
}

void FieldComparator<T>::FloatComparator::copy(int slot,
                                               int doc) 
{
  values[slot] = getValueForDoc(doc);
}

void FieldComparator<T>::FloatComparator::setBottom(int const bottom)
{
  this->bottom = values[bottom];
}

void FieldComparator<T>::FloatComparator::setTopValue(optional<float> &value)
{
  topValue = value;
}

optional<float> FieldComparator<T>::FloatComparator::value(int slot)
{
  return static_cast<Float>(values[slot]);
}

int FieldComparator<T>::FloatComparator::compareTop(int doc) 
{
  return Float::compare(topValue, getValueForDoc(doc));
}

FieldComparator<T>::IntComparator::IntComparator(int numHits,
                                                 const wstring &field,
                                                 optional<int> &missingValue)
    : NumericComparator<int>(field, missingValue != nullptr ? missingValue : 0),
      values(std::deque<int>(numHits))
{
  // System.out.println("IntComparator.init");
  // new Throwable().printStackTrace(System.out);
}

int FieldComparator<T>::IntComparator::getValueForDoc(int doc) throw(
    IOException)
{
  if (currentReaderValues->advanceExact(doc)) {
    return static_cast<int>(currentReaderValues->longValue());
  } else {
    return missingValue;
  }
}

int FieldComparator<T>::IntComparator::compare(int slot1, int slot2)
{
  return Integer::compare(values[slot1], values[slot2]);
}

int FieldComparator<T>::IntComparator::compareBottom(int doc) 
{
  return Integer::compare(bottom, getValueForDoc(doc));
}

void FieldComparator<T>::IntComparator::copy(int slot,
                                             int doc) 
{
  values[slot] = getValueForDoc(doc);
}

void FieldComparator<T>::IntComparator::setBottom(int const bottom)
{
  this->bottom = values[bottom];
}

void FieldComparator<T>::IntComparator::setTopValue(optional<int> &value)
{
  topValue = value;
}

optional<int> FieldComparator<T>::IntComparator::value(int slot)
{
  return static_cast<Integer>(values[slot]);
}

int FieldComparator<T>::IntComparator::compareTop(int doc) 
{
  return Integer::compare(topValue, getValueForDoc(doc));
}

FieldComparator<T>::LongComparator::LongComparator(
    int numHits, const wstring &field, optional<int64_t> &missingValue)
    : NumericComparator<long>(field,
                              missingValue != nullptr ? missingValue : 0LL),
      values(std::deque<int64_t>(numHits))
{
}

int64_t
FieldComparator<T>::LongComparator::getValueForDoc(int doc) 
{
  if (currentReaderValues->advanceExact(doc)) {
    return currentReaderValues->longValue();
  } else {
    return missingValue;
  }
}

int FieldComparator<T>::LongComparator::compare(int slot1, int slot2)
{
  return Long::compare(values[slot1], values[slot2]);
}

int FieldComparator<T>::LongComparator::compareBottom(int doc) throw(
    IOException)
{
  return Long::compare(bottom, getValueForDoc(doc));
}

void FieldComparator<T>::LongComparator::copy(int slot,
                                              int doc) 
{
  values[slot] = getValueForDoc(doc);
}

void FieldComparator<T>::LongComparator::setBottom(int const bottom)
{
  this->bottom = values[bottom];
}

void FieldComparator<T>::LongComparator::setTopValue(optional<int64_t> &value)
{
  topValue = value;
}

optional<int64_t> FieldComparator<T>::LongComparator::value(int slot)
{
  return static_cast<int64_t>(values[slot]);
}

int FieldComparator<T>::LongComparator::compareTop(int doc) 
{
  return Long::compare(topValue, getValueForDoc(doc));
}

FieldComparator<T>::RelevanceComparator::RelevanceComparator(int numHits)
    : scores(std::deque<float>(numHits))
{
}

int FieldComparator<T>::RelevanceComparator::compare(int slot1, int slot2)
{
  return Float::compare(scores[slot2], scores[slot1]);
}

int FieldComparator<T>::RelevanceComparator::compareBottom(int doc) throw(
    IOException)
{
  float score = scorer->score();
  assert(!isnan(score));
  return Float::compare(score, bottom);
}

void FieldComparator<T>::RelevanceComparator::copy(int slot,
                                                   int doc) 
{
  scores[slot] = scorer->score();
  assert(!isnan(scores[slot]));
}

shared_ptr<LeafFieldComparator>
FieldComparator<T>::RelevanceComparator::getLeafComparator(
    shared_ptr<LeafReaderContext> context)
{
  return shared_from_this();
}

void FieldComparator<T>::RelevanceComparator::setBottom(int const bottom)
{
  this->bottom = scores[bottom];
}

void FieldComparator<T>::RelevanceComparator::setTopValue(
    optional<float> &value)
{
  topValue = value;
}

void FieldComparator<T>::RelevanceComparator::setScorer(
    shared_ptr<Scorer> scorer)
{
  // wrap with a ScoreCachingWrappingScorer so that successive calls to
  // score() will not incur score computation over and
  // over again.
  if (!(std::dynamic_pointer_cast<ScoreCachingWrappingScorer>(scorer) !=
        nullptr)) {
    this->scorer = make_shared<ScoreCachingWrappingScorer>(scorer);
  } else {
    this->scorer = scorer;
  }
}

optional<float> FieldComparator<T>::RelevanceComparator::value(int slot)
{
  return static_cast<Float>(scores[slot]);
}

int FieldComparator<T>::RelevanceComparator::compareValues(
    optional<float> &first, optional<float> &second)
{
  // Reversed intentionally because relevance by default
  // sorts descending:
  return second.compareTo(first);
}

int FieldComparator<T>::RelevanceComparator::compareTop(int doc) throw(
    IOException)
{
  float docValue = scorer->score();
  assert(!isnan(docValue));
  return Float::compare(docValue, topValue);
}

FieldComparator<T>::DocComparator::DocComparator(int numHits)
    : docIDs(std::deque<int>(numHits))
{
}

int FieldComparator<T>::DocComparator::compare(int slot1, int slot2)
{
  // No overflow risk because docIDs are non-negative
  return docIDs[slot1] - docIDs[slot2];
}

int FieldComparator<T>::DocComparator::compareBottom(int doc)
{
  // No overflow risk because docIDs are non-negative
  return bottom - (docBase + doc);
}

void FieldComparator<T>::DocComparator::copy(int slot, int doc)
{
  docIDs[slot] = docBase + doc;
}

shared_ptr<LeafFieldComparator>
FieldComparator<T>::DocComparator::getLeafComparator(
    shared_ptr<LeafReaderContext> context)
{
  // TODO: can we "map_obj" our docIDs to the current
  // reader? saves having to then subtract on every
  // compare call
  this->docBase = context->docBase;
  return shared_from_this();
}

void FieldComparator<T>::DocComparator::setBottom(int const bottom)
{
  this->bottom = docIDs[bottom];
}

void FieldComparator<T>::DocComparator::setTopValue(optional<int> &value)
{
  topValue = value;
}

optional<int> FieldComparator<T>::DocComparator::value(int slot)
{
  return static_cast<Integer>(docIDs[slot]);
}

int FieldComparator<T>::DocComparator::compareTop(int doc)
{
  int docValue = docBase + doc;
  return Integer::compare(topValue, docValue);
}

void FieldComparator<T>::DocComparator::setScorer(shared_ptr<Scorer> scorer) {}

FieldComparator<T>::TermOrdValComparator::TermOrdValComparator(
    int numHits, const wstring &field)
    : TermOrdValComparator(numHits, field, false)
{
}

FieldComparator<T>::TermOrdValComparator::TermOrdValComparator(
    int numHits, const wstring &field, bool sortMissingLast)
    : ords(std::deque<int>(numHits)),
      values(std::deque<std::shared_ptr<BytesRef>>(numHits)),
      tempBRs(std::deque<std::shared_ptr<BytesRefBuilder>>(numHits)),
      readerGen(std::deque<int>(numHits)), field(field)
{
  if (sortMissingLast) {
    missingSortCmp = 1;
    missingOrd = numeric_limits<int>::max();
  } else {
    missingSortCmp = -1;
    missingOrd = -1;
  }
}

int FieldComparator<T>::TermOrdValComparator::getOrdForDoc(int doc) throw(
    IOException)
{
  if (termsIndex->advanceExact(doc)) {
    return termsIndex->ordValue();
  } else {
    return -1;
  }
}

int FieldComparator<T>::TermOrdValComparator::compare(int slot1, int slot2)
{
  if (readerGen[slot1] == readerGen[slot2]) {
    return ords[slot1] - ords[slot2];
  }

  shared_ptr<BytesRef> *const val1 = values[slot1];
  shared_ptr<BytesRef> *const val2 = values[slot2];
  if (val1 == nullptr) {
    if (val2 == nullptr) {
      return 0;
    }
    return missingSortCmp;
  } else if (val2 == nullptr) {
    return -missingSortCmp;
  }
  return val1->compareTo(val2);
}

int FieldComparator<T>::TermOrdValComparator::compareBottom(int doc) throw(
    IOException)
{
  assert(bottomSlot != -1);
  int docOrd = getOrdForDoc(doc);
  if (docOrd == -1) {
    docOrd = missingOrd;
  }
  if (bottomSameReader) {
    // ord is precisely comparable, even in the equal case
    return bottomOrd - docOrd;
  } else if (bottomOrd >= docOrd) {
    // the equals case always means bottom is > doc
    // (because we set bottomOrd to the lower bound in
    // setBottom):
    return 1;
  } else {
    return -1;
  }
}

void FieldComparator<T>::TermOrdValComparator::copy(int slot,
                                                    int doc) 
{
  int ord = getOrdForDoc(doc);
  if (ord == -1) {
    ord = missingOrd;
    values[slot].reset();
  } else {
    assert(ord >= 0);
    if (tempBRs[slot] == nullptr) {
      tempBRs[slot] = make_shared<BytesRefBuilder>();
    }
    tempBRs[slot]->copyBytes(termsIndex->lookupOrd(ord));
    values[slot] = tempBRs[slot]->get();
  }
  ords[slot] = ord;
  readerGen[slot] = currentReaderGen;
}

shared_ptr<SortedDocValues>
FieldComparator<T>::TermOrdValComparator::getSortedDocValues(
    shared_ptr<LeafReaderContext> context,
    const wstring &field) 
{
  return DocValues::getSorted(context->reader(), field);
}

shared_ptr<LeafFieldComparator>
FieldComparator<T>::TermOrdValComparator::getLeafComparator(
    shared_ptr<LeafReaderContext> context) 
{
  termsIndex = getSortedDocValues(context, field);
  currentReaderGen++;

  if (topValue != nullptr) {
    // Recompute topOrd/SameReader
    int ord = termsIndex->lookupTerm(topValue);
    if (ord >= 0) {
      topSameReader = true;
      topOrd = ord;
    } else {
      topSameReader = false;
      topOrd = -ord - 2;
    }
  } else {
    topOrd = missingOrd;
    topSameReader = true;
  }
  // System.out.println("  getLeafComparator topOrd=" + topOrd + "
  // topSameReader=" + topSameReader);

  if (bottomSlot != -1) {
    // Recompute bottomOrd/SameReader
    setBottom(bottomSlot);
  }

  return shared_from_this();
}

void FieldComparator<T>::TermOrdValComparator::setBottom(
    int const bottom) 
{
  bottomSlot = bottom;

  bottomValue = values[bottomSlot];
  if (currentReaderGen == readerGen[bottomSlot]) {
    bottomOrd = ords[bottomSlot];
    bottomSameReader = true;
  } else {
    if (bottomValue == nullptr) {
      // missingOrd is null for all segments
      assert(ords[bottomSlot] == missingOrd);
      bottomOrd = missingOrd;
      bottomSameReader = true;
      readerGen[bottomSlot] = currentReaderGen;
    } else {
      constexpr int ord = termsIndex->lookupTerm(bottomValue);
      if (ord < 0) {
        bottomOrd = -ord - 2;
        bottomSameReader = false;
      } else {
        bottomOrd = ord;
        // exact value match
        bottomSameReader = true;
        readerGen[bottomSlot] = currentReaderGen;
        ords[bottomSlot] = bottomOrd;
      }
    }
  }
}

void FieldComparator<T>::TermOrdValComparator::setTopValue(
    shared_ptr<BytesRef> value)
{
  // null is fine: it means the last doc of the prior
  // search was missing this value
  topValue = value;
  // System.out.println("setTopValue " + topValue);
}

shared_ptr<BytesRef> FieldComparator<T>::TermOrdValComparator::value(int slot)
{
  return values[slot];
}

int FieldComparator<T>::TermOrdValComparator::compareTop(int doc) throw(
    IOException)
{

  int ord = getOrdForDoc(doc);
  if (ord == -1) {
    ord = missingOrd;
  }

  if (topSameReader) {
    // ord is precisely comparable, even in the equal
    // case
    // System.out.println("compareTop doc=" + doc + " ord=" + ord + " ret=" +
    // (topOrd-ord));
    return topOrd - ord;
  } else if (ord <= topOrd) {
    // the equals case always means doc is < value
    // (because we set lastOrd to the lower bound)
    return 1;
  } else {
    return -1;
  }
}

int FieldComparator<T>::TermOrdValComparator::compareValues(
    shared_ptr<BytesRef> val1, shared_ptr<BytesRef> val2)
{
  if (val1 == nullptr) {
    if (val2 == nullptr) {
      return 0;
    }
    return missingSortCmp;
  } else if (val2 == nullptr) {
    return -missingSortCmp;
  }
  return val1->compareTo(val2);
}

void FieldComparator<T>::TermOrdValComparator::setScorer(
    shared_ptr<Scorer> scorer)
{
}

FieldComparator<T>::TermValComparator::TermValComparator(int numHits,
                                                         const wstring &field,
                                                         bool sortMissingLast)
    : values(std::deque<std::shared_ptr<BytesRef>>(numHits)),
      tempBRs(std::deque<std::shared_ptr<BytesRefBuilder>>(numHits)),
      field(field), missingSortCmp(sortMissingLast ? 1 : -1)
{
}

shared_ptr<BytesRef> FieldComparator<T>::TermValComparator::getValueForDoc(
    int doc) 
{
  if (docTerms->advanceExact(doc)) {
    return docTerms->binaryValue();
  } else {
    return nullptr;
  }
}

int FieldComparator<T>::TermValComparator::compare(int slot1, int slot2)
{
  shared_ptr<BytesRef> *const val1 = values[slot1];
  shared_ptr<BytesRef> *const val2 = values[slot2];
  return compareValues(val1, val2);
}

int FieldComparator<T>::TermValComparator::compareBottom(int doc) throw(
    IOException)
{
  shared_ptr<BytesRef> *const comparableBytes = getValueForDoc(doc);
  return compareValues(bottom, comparableBytes);
}

void FieldComparator<T>::TermValComparator::copy(int slot,
                                                 int doc) 
{
  shared_ptr<BytesRef> *const comparableBytes = getValueForDoc(doc);
  if (comparableBytes == nullptr) {
    values[slot].reset();
  } else {
    if (tempBRs[slot] == nullptr) {
      tempBRs[slot] = make_shared<BytesRefBuilder>();
    }
    tempBRs[slot]->copyBytes(comparableBytes);
    values[slot] = tempBRs[slot]->get();
  }
}

shared_ptr<BinaryDocValues>
FieldComparator<T>::TermValComparator::getBinaryDocValues(
    shared_ptr<LeafReaderContext> context,
    const wstring &field) 
{
  return DocValues::getBinary(context->reader(), field);
}

shared_ptr<LeafFieldComparator>
FieldComparator<T>::TermValComparator::getLeafComparator(
    shared_ptr<LeafReaderContext> context) 
{
  docTerms = getBinaryDocValues(context, field);
  return shared_from_this();
}

void FieldComparator<T>::TermValComparator::setBottom(int const bottom)
{
  this->bottom = values[bottom];
}

void FieldComparator<T>::TermValComparator::setTopValue(
    shared_ptr<BytesRef> value)
{
  // null is fine: it means the last doc of the prior
  // search was missing this value
  topValue = value;
}

shared_ptr<BytesRef> FieldComparator<T>::TermValComparator::value(int slot)
{
  return values[slot];
}

int FieldComparator<T>::TermValComparator::compareValues(
    shared_ptr<BytesRef> val1, shared_ptr<BytesRef> val2)
{
  // missing always sorts first:
  if (val1 == nullptr) {
    if (val2 == nullptr) {
      return 0;
    }
    return missingSortCmp;
  } else if (val2 == nullptr) {
    return -missingSortCmp;
  }
  return val1->compareTo(val2);
}

int FieldComparator<T>::TermValComparator::compareTop(int doc) throw(
    IOException)
{
  return compareValues(topValue, getValueForDoc(doc));
}

void FieldComparator<T>::TermValComparator::setScorer(shared_ptr<Scorer> scorer)
{
}
} // namespace org::apache::lucene::search