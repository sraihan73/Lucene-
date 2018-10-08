using namespace std;

#include "Sorter.h"

namespace org::apache::lucene::index
{
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using FieldComparator = org::apache::lucene::search::FieldComparator;
using Scorer = org::apache::lucene::search::Scorer;
using Sort = org::apache::lucene::search::Sort;
using SortField = org::apache::lucene::search::SortField;
using SortedNumericSelector =
    org::apache::lucene::search::SortedNumericSelector;
using SortedNumericSortField =
    org::apache::lucene::search::SortedNumericSortField;
using SortedSetSelector = org::apache::lucene::search::SortedSetSelector;
using SortedSetSortField = org::apache::lucene::search::SortedSetSortField;
using TimSorter = org::apache::lucene::util::TimSorter;
using PackedInts = org::apache::lucene::util::packed::PackedInts;
using PackedLongValues = org::apache::lucene::util::packed::PackedLongValues;
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

Sorter::Sorter(shared_ptr<Sort> sort) : sort(sort)
{
  if (sort->needsScores()) {
    throw invalid_argument(
        L"Cannot sort an index with a Sort that refers to the relevance score");
  }
}

bool Sorter::isConsistent(shared_ptr<DocMap> docMap)
{
  constexpr int maxDoc = docMap->size();
  for (int i = 0; i < maxDoc; ++i) {
    constexpr int newID = docMap->oldToNew(i);
    constexpr int oldID = docMap->newToOld(newID);
    assert((newID >= 0 && newID < maxDoc, L"doc IDs must be in [0-" +
                                              to_wstring(maxDoc) + L"[, got " +
                                              to_wstring(newID)));
    assert((i == oldID, L"mapping is inconsistent: " + to_wstring(i) +
                            L" --oldToNew--> " + to_wstring(newID) +
                            L" --newToOld--> " + to_wstring(oldID)));
    if (i != oldID || newID < 0 || newID >= maxDoc) {
      return false;
    }
  }
  return true;
}

Sorter::DocValueSorter::DocValueSorter(
    std::deque<int> &docs, shared_ptr<Sorter::DocComparator> comparator)
    : org::apache::lucene::util::TimSorter(docs.length / 64), docs(docs),
      comparator(comparator), tmp(std::deque<int>(docs.size() / 64))
{
}

int Sorter::DocValueSorter::compare(int i, int j)
{
  return comparator->compare(docs[i], docs[j]);
}

void Sorter::DocValueSorter::swap(int i, int j)
{
  int tmpDoc = docs[i];
  docs[i] = docs[j];
  docs[j] = tmpDoc;
}

void Sorter::DocValueSorter::copy(int src, int dest) { docs[dest] = docs[src]; }

void Sorter::DocValueSorter::save(int i, int len)
{
  System::arraycopy(docs, i, tmp, 0, len);
}

void Sorter::DocValueSorter::restore(int i, int j) { docs[j] = tmp[i]; }

int Sorter::DocValueSorter::compareSaved(int i, int j)
{
  return comparator->compare(tmp[i], docs[j]);
}

shared_ptr<Sorter::DocMap>
Sorter::sort(int const maxDoc,
             shared_ptr<FieldComparator::DocComparator> comparator)
{
  // check if the index is sorted
  bool sorted = true;
  for (int i = 1; i < maxDoc; ++i) {
    if (comparator->compare(i - 1, i) > 0) {
      sorted = false;
      break;
    }
  }
  if (sorted) {
    return nullptr;
  }

  // sort doc IDs
  const std::deque<int> docs = std::deque<int>(maxDoc);
  for (int i = 0; i < maxDoc; i++) {
    docs[i] = i;
  }

  shared_ptr<DocValueSorter> sorter =
      make_shared<DocValueSorter>(docs, comparator);
  // It can be common to sort a reader, add docs, sort it again, ... and in
  // that case timSort can save a lot of time
  sorter->sort(0, docs.size()); // docs is now the newToOld mapping

  // The reason why we use MonotonicAppendingLongBuffer here is that it
  // wastes very little memory if the index is in random order but can save
  // a lot of memory if the index is already "almost" sorted
  shared_ptr<PackedLongValues::Builder> *const newToOldBuilder =
      PackedLongValues::monotonicBuilder(PackedInts::COMPACT);
  for (int i = 0; i < maxDoc; ++i) {
    newToOldBuilder->add(docs[i]);
  }
  shared_ptr<PackedLongValues> *const newToOld = newToOldBuilder->build();

  // invert the docs mapping:
  for (int i = 0; i < maxDoc; ++i) {
    docs[static_cast<int>(newToOld->get(i))] = i;
  } // docs is now the oldToNew mapping

  shared_ptr<PackedLongValues::Builder> *const oldToNewBuilder =
      PackedLongValues::monotonicBuilder(PackedInts::COMPACT);
  for (int i = 0; i < maxDoc; ++i) {
    oldToNewBuilder->add(docs[i]);
  }
  shared_ptr<PackedLongValues> *const oldToNew = oldToNewBuilder->build();

  return make_shared<DocMapAnonymousInnerClass>(maxDoc, newToOld, oldToNew);
}

Sorter::DocMapAnonymousInnerClass::DocMapAnonymousInnerClass(
    int maxDoc, shared_ptr<PackedLongValues> newToOld,
    shared_ptr<PackedLongValues> oldToNew)
{
  this->maxDoc = maxDoc;
  this->newToOld = newToOld;
  this->oldToNew = oldToNew;
}

int Sorter::DocMapAnonymousInnerClass::oldToNew(int docID)
{
  return static_cast<int>(oldToNew->get(docID));
}

int Sorter::DocMapAnonymousInnerClass::newToOld(int docID)
{
  return static_cast<int>(newToOld->get(docID));
}

int Sorter::DocMapAnonymousInnerClass::size() { return maxDoc; }

SortField::Type Sorter::getSortFieldType(shared_ptr<SortField> sortField)
{
  if (std::dynamic_pointer_cast<SortedSetSortField>(sortField) != nullptr) {
    return SortField::Type::STRING;
  } else if (std::dynamic_pointer_cast<SortedNumericSortField>(sortField) !=
             nullptr) {
    return (std::static_pointer_cast<SortedNumericSortField>(sortField))
        ->getNumericType();
  } else {
    return sortField->getType();
  }
}

shared_ptr<NumericDocValues>
Sorter::getOrWrapNumeric(shared_ptr<LeafReader> reader,
                         shared_ptr<SortField> sortField) 
{
  if (std::dynamic_pointer_cast<SortedNumericSortField>(sortField) != nullptr) {
    shared_ptr<SortedNumericSortField> sf =
        std::static_pointer_cast<SortedNumericSortField>(sortField);
    return SortedNumericSelector::wrap(
        DocValues::getSortedNumeric(reader, sf->getField()), sf->getSelector(),
        sf->getNumericType());
  } else {
    return DocValues::getNumeric(reader, sortField->getField());
  }
}

shared_ptr<SortedDocValues>
Sorter::getOrWrapSorted(shared_ptr<LeafReader> reader,
                        shared_ptr<SortField> sortField) 
{
  if (std::dynamic_pointer_cast<SortedSetSortField>(sortField) != nullptr) {
    shared_ptr<SortedSetSortField> sf =
        std::static_pointer_cast<SortedSetSortField>(sortField);
    return SortedSetSelector::wrap(
        DocValues::getSortedSet(reader, sf->getField()), sf->getSelector());
  } else {
    return DocValues::getSorted(reader, sortField->getField());
  }
}

shared_ptr<FieldComparator::DocComparator>
Sorter::getDocComparator(shared_ptr<LeafReader> reader,
                         shared_ptr<SortField> sortField) 
{
  return getDocComparator(reader->maxDoc(), sortField,
                          [&]() { getOrWrapSorted(reader, sortField); },
                          [&]() { getOrWrapNumeric(reader, sortField); });
}

shared_ptr<FieldComparator::DocComparator> Sorter::getDocComparator(
    int maxDoc, shared_ptr<SortField> sortField,
    shared_ptr<SortedDocValuesSupplier> sortedProvider,
    shared_ptr<NumericDocValuesSupplier> numericProvider) 
{

  constexpr int reverseMul = sortField->getReverse() ? -1 : 1;
  constexpr SortField::Type sortType = getSortFieldType(sortField);

  switch (sortType) {

  case SortField::Type::STRING: {
    shared_ptr<SortedDocValues> *const sorted = sortedProvider->get();
    constexpr int missingOrd;
    if (sortField->getMissingValue() == SortField::STRING_LAST) {
      missingOrd = numeric_limits<int>::max();
    } else {
      missingOrd = numeric_limits<int>::min();
    }

    const std::deque<int> ords = std::deque<int>(maxDoc);
    Arrays::fill(ords, missingOrd);
    int docID;
    while ((docID = sorted->nextDoc()) != DocIdSetIterator::NO_MORE_DOCS) {
      ords[docID] = sorted->ordValue();
    }

    return make_shared<DocComparatorAnonymousInnerClass>(reverseMul, ords);
  }

  case SortField::Type::LONG: {
    shared_ptr<NumericDocValues> *const dvs = numericProvider->get();
    std::deque<int64_t> values(maxDoc);
    if (sortField->getMissingValue() != nullptr) {
      Arrays::fill(values,
                   any_cast<optional<int64_t>>(sortField->getMissingValue()));
    }
    while (true) {
      int docID = dvs->nextDoc();
      if (docID == DocIdSetIterator::NO_MORE_DOCS) {
        break;
      }
      values[docID] = dvs->longValue();
    }

    return make_shared<DocComparatorAnonymousInnerClass2>(reverseMul, values);
  }

  case SortField::Type::INT: {
    shared_ptr<NumericDocValues> *const dvs = numericProvider->get();
    std::deque<int> values(maxDoc);
    if (sortField->getMissingValue() != nullptr) {
      Arrays::fill(values,
                   any_cast<optional<int>>(sortField->getMissingValue()));
    }

    while (true) {
      int docID = dvs->nextDoc();
      if (docID == DocIdSetIterator::NO_MORE_DOCS) {
        break;
      }
      values[docID] = static_cast<int>(dvs->longValue());
    }

    return make_shared<DocComparatorAnonymousInnerClass3>(reverseMul, values);
  }

  case SortField::Type::DOUBLE: {
    shared_ptr<NumericDocValues> *const dvs = numericProvider->get();
    std::deque<double> values(maxDoc);
    if (sortField->getMissingValue() != nullptr) {
      Arrays::fill(values,
                   any_cast<optional<double>>(sortField->getMissingValue()));
    }
    while (true) {
      int docID = dvs->nextDoc();
      if (docID == DocIdSetIterator::NO_MORE_DOCS) {
        break;
      }
      values[docID] = Double::longBitsToDouble(dvs->longValue());
    }

    return make_shared<DocComparatorAnonymousInnerClass4>(reverseMul, values);
  }

  case SortField::Type::FLOAT: {
    shared_ptr<NumericDocValues> *const dvs = numericProvider->get();
    std::deque<float> values(maxDoc);
    if (sortField->getMissingValue() != nullptr) {
      Arrays::fill(values,
                   any_cast<optional<float>>(sortField->getMissingValue()));
    }
    while (true) {
      int docID = dvs->nextDoc();
      if (docID == DocIdSetIterator::NO_MORE_DOCS) {
        break;
      }
      values[docID] = Float::intBitsToFloat(static_cast<int>(dvs->longValue()));
    }

    return make_shared<DocComparatorAnonymousInnerClass5>(reverseMul, values);
  }

  default:
    throw invalid_argument(L"unhandled SortField.getType()=" +
                           sortField->getType());
  }
}

Sorter::DocComparatorAnonymousInnerClass::DocComparatorAnonymousInnerClass(
    int reverseMul, deque<int> &ords)
{
  this->reverseMul = reverseMul;
  this->ords = ords;
}

int Sorter::DocComparatorAnonymousInnerClass::compare(int docID1, int docID2)
{
  return reverseMul * Integer::compare(ords[docID1], ords[docID2]);
}

Sorter::DocComparatorAnonymousInnerClass2::DocComparatorAnonymousInnerClass2(
    int reverseMul, deque<int64_t> &values)
{
  this->reverseMul = reverseMul;
  this->values = values;
}

int Sorter::DocComparatorAnonymousInnerClass2::compare(int docID1, int docID2)
{
  return reverseMul * Long::compare(values[docID1], values[docID2]);
}

Sorter::DocComparatorAnonymousInnerClass3::DocComparatorAnonymousInnerClass3(
    int reverseMul, deque<int> &values)
{
  this->reverseMul = reverseMul;
  this->values = values;
}

int Sorter::DocComparatorAnonymousInnerClass3::compare(int docID1, int docID2)
{
  return reverseMul * Integer::compare(values[docID1], values[docID2]);
}

Sorter::DocComparatorAnonymousInnerClass4::DocComparatorAnonymousInnerClass4(
    int reverseMul, deque<double> &values)
{
  this->reverseMul = reverseMul;
  this->values = values;
}

int Sorter::DocComparatorAnonymousInnerClass4::compare(int docID1, int docID2)
{
  return reverseMul * Double::compare(values[docID1], values[docID2]);
}

Sorter::DocComparatorAnonymousInnerClass5::DocComparatorAnonymousInnerClass5(
    int reverseMul, deque<float> &values)
{
  this->reverseMul = reverseMul;
  this->values = values;
}

int Sorter::DocComparatorAnonymousInnerClass5::compare(int docID1, int docID2)
{
  return reverseMul * Float::compare(values[docID1], values[docID2]);
}

shared_ptr<DocMap>
Sorter::sort(shared_ptr<LeafReader> reader) 
{
  std::deque<std::shared_ptr<SortField>> fields = sort_->getSort();
  std::deque<std::shared_ptr<FieldComparator::DocComparator>> comparators(
      fields.size());

  for (int i = 0; i < fields.size(); i++) {
    comparators[i] = getDocComparator(reader, fields[i]);
  }
  return sort(reader->maxDoc(), comparators);
}

shared_ptr<DocMap>
Sorter::sort(int maxDoc,
             std::deque<std::shared_ptr<FieldComparator::DocComparator>>
                 &comparators) 
{
  shared_ptr<FieldComparator::DocComparator> *const comparator =
      make_shared<DocComparatorAnonymousInnerClass6>(shared_from_this(),
                                                     comparators);

  return sort(maxDoc, comparator);
}

Sorter::DocComparatorAnonymousInnerClass6::DocComparatorAnonymousInnerClass6(
    shared_ptr<Sorter> outerInstance,
    deque<std::shared_ptr<FieldComparator::DocComparator>> &comparators)
{
  this->outerInstance = outerInstance;
  this->comparators = comparators;
}

int Sorter::DocComparatorAnonymousInnerClass6::compare(int docID1, int docID2)
{
  for (int i = 0; i < comparators.size(); i++) {
    int comp = comparators[i]->compare(docID1, docID2);
    if (comp != 0) {
      return comp;
    }
  }
  return Integer::compare(docID1, docID2); // docid order tiebreak
}

wstring Sorter::getID()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return sort_->toString();
}

wstring Sorter::toString() { return getID(); }

const shared_ptr<org::apache::lucene::search::Scorer> Sorter::FAKESCORER =
    make_shared<ScorerAnonymousInnerClass>();

Sorter::ScorerAnonymousInnerClass::ScorerAnonymousInnerClass()
    : org::apache::lucene::search::Scorer(nullptr)
{
}

int Sorter::ScorerAnonymousInnerClass::docID() { return doc; }

shared_ptr<DocIdSetIterator> Sorter::ScorerAnonymousInnerClass::iterator()
{
  throw make_shared<UnsupportedOperationException>();
}

float Sorter::ScorerAnonymousInnerClass::score() 
{
  return score;
}
} // namespace org::apache::lucene::index