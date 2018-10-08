using namespace std;

#include "SortedSetSelector.h"

namespace org::apache::lucene::search
{
using DocValues = org::apache::lucene::index::DocValues;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
//    import static org.apache.lucene.index.SortedSetDocValues.NO_MORE_ORDS;

shared_ptr<SortedDocValues>
SortedSetSelector::wrap(shared_ptr<SortedSetDocValues> sortedSet, Type selector)
{
  if (sortedSet->getValueCount() >= numeric_limits<int>::max()) {
    throw make_shared<UnsupportedOperationException>(
        L"fields containing more than " + (numeric_limits<int>::max() - 1) +
        L" unique terms are unsupported");
  }

  shared_ptr<SortedDocValues> singleton = DocValues::unwrapSingleton(sortedSet);
  if (singleton != nullptr) {
    // it's actually single-valued in practice, but indexed as multi-valued,
    // so just sort on the underlying single-valued dv directly.
    // regardless of selector type, this optimization is safe!
    return singleton;
  } else {
    switch (selector) {
    case org::apache::lucene::search::SortedSetSelector::Type::MIN:
      return make_shared<MinValue>(sortedSet);
    case org::apache::lucene::search::SortedSetSelector::Type::MAX:
      return make_shared<MaxValue>(sortedSet);
    case org::apache::lucene::search::SortedSetSelector::Type::MIDDLE_MIN:
      return make_shared<MiddleMinValue>(sortedSet);
    case org::apache::lucene::search::SortedSetSelector::Type::MIDDLE_MAX:
      return make_shared<MiddleMaxValue>(sortedSet);
    default:
      throw make_shared<AssertionError>();
    }
  }
}

SortedSetSelector::MinValue::MinValue(shared_ptr<SortedSetDocValues> in_)
    : in_(in_)
{
}

int SortedSetSelector::MinValue::docID() { return in_->docID(); }

int SortedSetSelector::MinValue::nextDoc() 
{
  in_->nextDoc();
  setOrd();
  return docID();
}

int SortedSetSelector::MinValue::advance(int target) 
{
  in_->advance(target);
  setOrd();
  return docID();
}

bool SortedSetSelector::MinValue::advanceExact(int target) 
{
  if (in_->advanceExact(target)) {
    setOrd();
    return true;
  }
  return false;
}

int64_t SortedSetSelector::MinValue::cost() { return in_->cost(); }

int SortedSetSelector::MinValue::ordValue() { return ord; }

shared_ptr<BytesRef>
SortedSetSelector::MinValue::lookupOrd(int ord) 
{
  return in_->lookupOrd(ord);
}

int SortedSetSelector::MinValue::getValueCount()
{
  return static_cast<int>(in_->getValueCount());
}

int SortedSetSelector::MinValue::lookupTerm(shared_ptr<BytesRef> key) throw(
    IOException)
{
  return static_cast<int>(in_->lookupTerm(key));
}

void SortedSetSelector::MinValue::setOrd() 
{
  if (docID() != NO_MORE_DOCS) {
    ord = static_cast<int>(in_->nextOrd());
  } else {
    ord = static_cast<int>(SortedSetDocValues::NO_MORE_ORDS);
  }
}

SortedSetSelector::MaxValue::MaxValue(shared_ptr<SortedSetDocValues> in_)
    : in_(in_)
{
}

int SortedSetSelector::MaxValue::docID() { return in_->docID(); }

int SortedSetSelector::MaxValue::nextDoc() 
{
  in_->nextDoc();
  setOrd();
  return docID();
}

int SortedSetSelector::MaxValue::advance(int target) 
{
  in_->advance(target);
  setOrd();
  return docID();
}

bool SortedSetSelector::MaxValue::advanceExact(int target) 
{
  if (in_->advanceExact(target)) {
    setOrd();
    return true;
  }
  return false;
}

int64_t SortedSetSelector::MaxValue::cost() { return in_->cost(); }

int SortedSetSelector::MaxValue::ordValue() { return ord; }

shared_ptr<BytesRef>
SortedSetSelector::MaxValue::lookupOrd(int ord) 
{
  return in_->lookupOrd(ord);
}

int SortedSetSelector::MaxValue::getValueCount()
{
  return static_cast<int>(in_->getValueCount());
}

int SortedSetSelector::MaxValue::lookupTerm(shared_ptr<BytesRef> key) throw(
    IOException)
{
  return static_cast<int>(in_->lookupTerm(key));
}

void SortedSetSelector::MaxValue::setOrd() 
{
  if (docID() != NO_MORE_DOCS) {
    while (true) {
      int64_t nextOrd = in_->nextOrd();
      if (nextOrd == SortedSetDocValues::NO_MORE_ORDS) {
        break;
      }
      ord = static_cast<int>(nextOrd);
    }
  } else {
    ord = static_cast<int>(SortedSetDocValues::NO_MORE_ORDS);
  }
}

SortedSetSelector::MiddleMinValue::MiddleMinValue(
    shared_ptr<SortedSetDocValues> in_)
    : in_(in_)
{
}

int SortedSetSelector::MiddleMinValue::docID() { return in_->docID(); }

int SortedSetSelector::MiddleMinValue::nextDoc() 
{
  in_->nextDoc();
  setOrd();
  return docID();
}

int SortedSetSelector::MiddleMinValue::advance(int target) 
{
  in_->advance(target);
  setOrd();
  return docID();
}

bool SortedSetSelector::MiddleMinValue::advanceExact(int target) throw(
    IOException)
{
  if (in_->advanceExact(target)) {
    setOrd();
    return true;
  }
  return false;
}

int64_t SortedSetSelector::MiddleMinValue::cost() { return in_->cost(); }

int SortedSetSelector::MiddleMinValue::ordValue() { return ord; }

shared_ptr<BytesRef>
SortedSetSelector::MiddleMinValue::lookupOrd(int ord) 
{
  return in_->lookupOrd(ord);
}

int SortedSetSelector::MiddleMinValue::getValueCount()
{
  return static_cast<int>(in_->getValueCount());
}

int SortedSetSelector::MiddleMinValue::lookupTerm(
    shared_ptr<BytesRef> key) 
{
  return static_cast<int>(in_->lookupTerm(key));
}

void SortedSetSelector::MiddleMinValue::setOrd() 
{
  if (docID() != NO_MORE_DOCS) {
    int upto = 0;
    while (true) {
      int64_t nextOrd = in_->nextOrd();
      if (nextOrd == SortedSetDocValues::NO_MORE_ORDS) {
        break;
      }
      if (upto == ords.size()) {
        ords = ArrayUtil::grow(ords);
      }
      ords[upto++] = static_cast<int>(nextOrd);
    }

    if (upto == 0) {
      // iterator should not have returned this docID if it has no ords:
      assert(false);
      ord = static_cast<int>(SortedSetDocValues::NO_MORE_ORDS);
    } else {
      ord = ords[static_cast<int>(static_cast<unsigned int>((upto - 1)) >> 1)];
    }
  } else {
    ord = static_cast<int>(SortedSetDocValues::NO_MORE_ORDS);
  }
}

SortedSetSelector::MiddleMaxValue::MiddleMaxValue(
    shared_ptr<SortedSetDocValues> in_)
    : in_(in_)
{
}

int SortedSetSelector::MiddleMaxValue::docID() { return in_->docID(); }

int SortedSetSelector::MiddleMaxValue::nextDoc() 
{
  in_->nextDoc();
  setOrd();
  return docID();
}

int SortedSetSelector::MiddleMaxValue::advance(int target) 
{
  in_->advance(target);
  setOrd();
  return docID();
}

bool SortedSetSelector::MiddleMaxValue::advanceExact(int target) throw(
    IOException)
{
  if (in_->advanceExact(target)) {
    setOrd();
    return true;
  }
  return false;
}

int64_t SortedSetSelector::MiddleMaxValue::cost() { return in_->cost(); }

int SortedSetSelector::MiddleMaxValue::ordValue() { return ord; }

shared_ptr<BytesRef>
SortedSetSelector::MiddleMaxValue::lookupOrd(int ord) 
{
  return in_->lookupOrd(ord);
}

int SortedSetSelector::MiddleMaxValue::getValueCount()
{
  return static_cast<int>(in_->getValueCount());
}

int SortedSetSelector::MiddleMaxValue::lookupTerm(
    shared_ptr<BytesRef> key) 
{
  return static_cast<int>(in_->lookupTerm(key));
}

void SortedSetSelector::MiddleMaxValue::setOrd() 
{
  if (docID() != NO_MORE_DOCS) {
    int upto = 0;
    while (true) {
      int64_t nextOrd = in_->nextOrd();
      if (nextOrd == SortedSetDocValues::NO_MORE_ORDS) {
        break;
      }
      if (upto == ords.size()) {
        ords = ArrayUtil::grow(ords);
      }
      ords[upto++] = static_cast<int>(nextOrd);
    }

    if (upto == 0) {
      // iterator should not have returned this docID if it has no ords:
      assert(false);
      ord = static_cast<int>(SortedSetDocValues::NO_MORE_ORDS);
    } else {
      ord = ords[static_cast<int>(static_cast<unsigned int>(upto) >> 1)];
    }
  } else {
    ord = static_cast<int>(SortedSetDocValues::NO_MORE_ORDS);
  }
}
} // namespace org::apache::lucene::search