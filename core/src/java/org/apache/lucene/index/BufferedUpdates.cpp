using namespace std;

#include "BufferedUpdates.h"

namespace org::apache::lucene::index
{
using BinaryDocValuesUpdate =
    org::apache::lucene::index::DocValuesUpdate::BinaryDocValuesUpdate;
using NumericDocValuesUpdate =
    org::apache::lucene::index::DocValuesUpdate::NumericDocValuesUpdate;
using Query = org::apache::lucene::search::Query;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
const optional<int> BufferedUpdates::MAX_INT =
    static_cast<Integer>(numeric_limits<int>::max());

BufferedUpdates::BufferedUpdates(const wstring &segmentName)
    : bytesUsed(make_shared<AtomicLong>()), segmentName(segmentName)
{
}

wstring BufferedUpdates::toString()
{
  if (VERBOSE_DELETES) {
    return L"gen=" + to_wstring(gen) + L" numTerms=" + numTermDeletes +
           L", deleteTerms=" + deleteTerms + L", deleteQueries=" +
           deleteQueries + L", deleteDocIDs=" + deleteDocIDs +
           L", numericUpdates=" + numericUpdates + L", binaryUpdates=" +
           binaryUpdates + L", bytesUsed=" + bytesUsed;
  } else {
    wstring s = L"gen=" + to_wstring(gen);
    if (numTermDeletes->get() != 0) {
      s += L" " + numTermDeletes->get() + L" deleted terms (unique count=" +
           deleteTerms.size() + L")";
    }
    if (deleteQueries.size() != 0) {
      s += L" " + deleteQueries.size() + L" deleted queries";
    }
    if (deleteDocIDs.size() != 0) {
      s += L" " + deleteDocIDs.size() + L" deleted docIDs";
    }
    if (numNumericUpdates->get() != 0) {
      s += L" " + numNumericUpdates->get() +
           L" numeric updates (unique count=" + numericUpdates.size() + L")";
    }
    if (numBinaryUpdates->get() != 0) {
      s += L" " + numBinaryUpdates->get() + L" binary updates (unique count=" +
           binaryUpdates.size() + L")";
    }
    if (bytesUsed->get() != 0) {
      s += L" bytesUsed=" + bytesUsed->get();
    }

    return s;
  }
}

void BufferedUpdates::addQuery(shared_ptr<Query> query, int docIDUpto)
{
  optional<int> current = deleteQueries.emplace(query, docIDUpto);
  // increment bytes used only if the query wasn't added so far.
  if (!current) {
    bytesUsed->addAndGet(BYTES_PER_DEL_QUERY);
  }
}

void BufferedUpdates::addDocID(int docID)
{
  deleteDocIDs.push_back(static_cast<Integer>(docID));
  bytesUsed->addAndGet(BYTES_PER_DEL_DOCID);
}

void BufferedUpdates::addTerm(shared_ptr<Term> term, int docIDUpto)
{
  optional<int> current = deleteTerms[term];
  if (current && docIDUpto < current) {
    // Only record the new number if it's greater than the
    // current one.  This is important because if multiple
    // threads are replacing the same doc at nearly the
    // same time, it's possible that one thread that got a
    // higher docID is scheduled before the other
    // threads.  If we blindly replace than we can
    // incorrectly get both docs indexed.
    return;
  }

  deleteTerms.emplace(term, static_cast<Integer>(docIDUpto));
  // note that if current != null then it means there's already a buffered
  // delete on that term, therefore we seem to over-count. this over-counting
  // is done to respect IndexWriterConfig.setMaxBufferedDeleteTerms.
  numTermDeletes->incrementAndGet();
  if (!current) {
    bytesUsed->addAndGet(BYTES_PER_DEL_TERM + term->bytes_->length +
                         (Character::BYTES * term->field().length()));
  }
}

void BufferedUpdates::addNumericUpdate(
    shared_ptr<NumericDocValuesUpdate> update, int docIDUpto)
{
  if (addDocValuesUpdate(
          numericUpdates, update, docIDUpto, update::prepareForApply,
          BYTES_PER_NUMERIC_UPDATE_ENTRY, BYTES_PER_NUMERIC_FIELD_ENTRY)) {
    numNumericUpdates->incrementAndGet();
  }
}

void BufferedUpdates::addBinaryUpdate(shared_ptr<BinaryDocValuesUpdate> update,
                                      int docIDUpto)
{
  if (addDocValuesUpdate(binaryUpdates, update, docIDUpto,
                         update::prepareForApply, BYTES_PER_BINARY_UPDATE_ENTRY,
                         BYTES_PER_BINARY_FIELD_ENTRY)) {
    numBinaryUpdates->incrementAndGet();
  }
}

template <typename T>
bool BufferedUpdates::addDocValuesUpdate(
    unordered_map<wstring, LinkedHashMap<std::shared_ptr<Term>, T>> &updates,
    T update, int docIDUpto, function<T *(int)> &prepareForApply,
    int64_t bytesPerUpdateEntry, int64_t bytesPerFieldEntry)
{
  static_assert(is_base_of<DocValuesUpdate, T>::value,
                L"T must inherit from DocValuesUpdate");

  shared_ptr<LinkedHashMap<std::shared_ptr<Term>, T>> fieldUpdates =
      updates[update->field];
  if (fieldUpdates == nullptr) {
    fieldUpdates = make_shared<LinkedHashMap<Term, T>>();
    updates.emplace(update->field, fieldUpdates);
    bytesUsed->addAndGet(bytesPerFieldEntry);
  }
  constexpr T current = fieldUpdates->get(update->term);
  if (current != nullptr && docIDUpto < current->docIDUpto) {
    // Only record the new number if it's greater than or equal to the current
    // one. This is important because if multiple threads are replacing the
    // same doc at nearly the same time, it's possible that one thread that
    // got a higher docID is scheduled before the other threads.
    return false;
  }

  // since it's a LinkedHashMap, we must first remove the Term entry so that
  // it's added last (we're interested in insertion-order).
  if (current != nullptr) {
    fieldUpdates->remove(update->term);
  }

  fieldUpdates->put(
      update->term,
      prepareForApply(docIDUpto)); // only make a copy if necessary
  if (current == nullptr) {
    bytesUsed->addAndGet(bytesPerUpdateEntry + update->sizeInBytes());
  }
  return true;
}

void BufferedUpdates::clearDeleteTerms()
{
  deleteTerms.clear();
  numTermDeletes->set(0);
}

void BufferedUpdates::clear()
{
  deleteTerms.clear();
  deleteQueries.clear();
  deleteDocIDs.clear();
  numericUpdates.clear();
  binaryUpdates.clear();
  numTermDeletes->set(0);
  numNumericUpdates->set(0);
  numBinaryUpdates->set(0);
  bytesUsed->set(0);
}

bool BufferedUpdates::any()
{
  return deleteTerms.size() > 0 || deleteDocIDs.size() > 0 ||
         deleteQueries.size() > 0 || numericUpdates.size() > 0 ||
         binaryUpdates.size() > 0;
}
} // namespace org::apache::lucene::index