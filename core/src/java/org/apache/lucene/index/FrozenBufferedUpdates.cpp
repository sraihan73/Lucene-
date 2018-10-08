using namespace std;

#include "FrozenBufferedUpdates.h"

namespace org::apache::lucene::index
{
using BinaryDocValuesUpdate =
    org::apache::lucene::index::DocValuesUpdate::BinaryDocValuesUpdate;
using NumericDocValuesUpdate =
    org::apache::lucene::index::DocValuesUpdate::NumericDocValuesUpdate;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using Weight = org::apache::lucene::search::Weight;
using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using RAMOutputStream = org::apache::lucene::store::RAMOutputStream;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using Counter = org::apache::lucene::util::Counter;
using IOUtils = org::apache::lucene::util::IOUtils;
using InfoStream = org::apache::lucene::util::InfoStream;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

FrozenBufferedUpdates::FrozenBufferedUpdates(
    shared_ptr<InfoStream> infoStream, shared_ptr<BufferedUpdates> updates,
    shared_ptr<SegmentCommitInfo> privateSegment) 
    : deleteTerms(builder::finish()),
      deleteQueries(
          std::deque<std::shared_ptr<Query>>(updates->deleteQueries.size())),
      deleteQueryLimits(std::deque<int>(updates->deleteQueries.size())),
      numericDVUpdates(
          freezeDVUpdates(updates->numericUpdates, counter::addAndGet)),
      binaryDVUpdates(
          freezeDVUpdates(updates->binaryUpdates, counter::addAndGet)),
      numericDVUpdateCount(static_cast<int>(counter->get())),
      binaryDVUpdateCount(static_cast<int>(counter->get())),
      bytesUsed(static_cast<int>(deleteTerms->ramBytesUsed() +
                                 deleteQueries.size() * BYTES_PER_DEL_QUERY +
                                 numericDVUpdates.size() +
                                 binaryDVUpdates.size())),
      numTermDeletes(updates->numTermDeletes->get()),
      privateSegment(privateSegment), infoStream(infoStream)
{
  assert(updates->deleteDocIDs.empty());
  assert((privateSegment == nullptr || updates->deleteTerms.empty(),
          L"segment private packet should only have del queries"));
  std::deque<std::shared_ptr<Term>> termsArray =
      updates->deleteTerms.keySet().toArray(
          std::deque<std::shared_ptr<Term>>(updates->deleteTerms.size()));
  ArrayUtil::timSort(termsArray);
  shared_ptr<PrefixCodedTerms::Builder> builder =
      make_shared<PrefixCodedTerms::Builder>();
  for (auto term : termsArray) {
    builder->add(term);
  }

  int upto = 0;
  for (auto ent : updates->deleteQueries) {
    deleteQueries[upto] = ent.first;
    deleteQueryLimits[upto] = ent.second;
    upto++;
  }
  shared_ptr<Counter> counter = Counter::newCounter();
  // TODO if a Term affects multiple fields, we could keep the updates key'd by
  // Term so that it maps to all fields it affects, sorted by their docUpto, and
  // traverse that Term only once, applying the update to all fields that still
  // need to be updated.
  counter->addAndGet(-counter->get());
  assert(counter->get() == 0);
  // TODO if a Term affects multiple fields, we could keep the updates key'd by
  // Term so that it maps to all fields it affects, sorted by their docUpto, and
  // traverse that Term only once, applying the update to all fields that still
  // need to be updated.

  if (infoStream != nullptr && infoStream->isEnabled(L"BD")) {
    infoStream->message(
        L"BD", wstring::format(Locale::ROOT,
                               L"compressed %d to %d bytes (%.2f%%) for "
                               L"deletes/updates; private segment %s",
                               updates->bytesUsed->get(), bytesUsed,
                               100.0 * bytesUsed / updates->bytesUsed->get(),
                               privateSegment));
  }
}

template <typename T>
std::deque<char> FrozenBufferedUpdates::freezeDVUpdates(
    unordered_map<wstring, LinkedHashMap<std::shared_ptr<Term>, T>> &dvUpdates,
    function<void(int)> &updateSizeConsumer) 
{
  static_assert(is_base_of<DocValuesUpdate, T>::value,
                L"T must inherit from DocValuesUpdate");

  // TODO: we could do better here, e.g. collate the updates by field
  // so if you are updating 2 fields interleaved we don't keep writing the field
  // strings
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.RAMOutputStream out
  // = new org.apache.lucene.store.RAMOutputStream())
  {
    org::apache::lucene::store::RAMOutputStream out =
        org::apache::lucene::store::RAMOutputStream();
    wstring lastTermField = L"";
    wstring lastUpdateField = L"";
    for (auto updates : dvUpdates) {
      updateSizeConsumer(updates->second->size());
      for (T update : updates->second.values()) {
        int code = update->term->bytes()->length << 3;

        wstring termField = update->term->field();
        if (termField == lastTermField == false) {
          code |= 1;
        }
        wstring updateField = update->field;
        if (updateField == lastUpdateField == false) {
          code |= 2;
        }
        if (update->hasValue()) {
          code |= 4;
        }
        out->writeVInt(code);
        out->writeVInt(update->docIDUpto);
        if (termField == lastTermField == false) {
          out->writeString(termField);
          lastTermField = termField;
        }
        if (updateField == lastUpdateField == false) {
          out->writeString(updateField);
          lastUpdateField = updateField;
        }
        out->writeBytes(update->term->bytes()->bytes,
                        update->term->bytes()->offset,
                        update->term->bytes()->length);
        if (update->hasValue()) {
          update->writeTo(out);
        }
      }
    }
    std::deque<char> bytes(static_cast<int>(out->getFilePointer()));
    out->writeTo(bytes, 0);
    return bytes;
  }
}

deque<std::shared_ptr<SegmentCommitInfo>>
FrozenBufferedUpdates::getInfosToApply(shared_ptr<IndexWriter> writer)
{
  assert(Thread::holdsLock(writer));
  deque<std::shared_ptr<SegmentCommitInfo>> infos;
  if (privateSegment != nullptr) {
    if (writer->segmentInfos->find(privateSegment) == -1) {
      if (infoStream->isEnabled(L"BD")) {
        infoStream->message(
            L"BD", L"private segment already gone; skip processing updates");
      }
      return nullptr;
    } else {
      infos = Collections::singletonList(privateSegment);
    }
  } else {
    infos = writer->segmentInfos->asList();
  }
  return infos;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("try") public synchronized void
// apply(IndexWriter writer) throws java.io.IOException C++ WARNING: The
// following method was originally marked 'synchronized':
void FrozenBufferedUpdates::apply(shared_ptr<IndexWriter> writer) throw(
    IOException)
{
  if (applied->getCount() == 0) {
    // already done
    return;
  }

  int64_t startNS = System::nanoTime();

  assert(any());

  shared_ptr<Set<std::shared_ptr<SegmentCommitInfo>>> seenSegments =
      unordered_set<std::shared_ptr<SegmentCommitInfo>>();

  int iter = 0;
  int totalSegmentCount = 0;
  int64_t totalDelCount = 0;

  bool finished = false;

  // Optimistic concurrency: assume we are free to resolve the deletes against
  // all current segments in the index, despite that concurrent merges are
  // running.  Once we are done, we check to see if a merge completed while we
  // were running.  If so, we must retry resolving against the newly merged
  // segment(s).  Eventually no merge finishes while we were running and we are
  // done.
  while (true) {
    wstring messagePrefix;
    if (iter == 0) {
      messagePrefix = L"";
    } else {
      messagePrefix = L"iter " + to_wstring(iter);
    }

    int64_t iterStartNS = System::nanoTime();

    int64_t mergeGenStart = writer->mergeFinishedGen->get();

    shared_ptr<Set<wstring>> delFiles = unordered_set<wstring>();
    std::deque<std::shared_ptr<BufferedUpdatesStream::SegmentState>> segStates;

    {
      lock_guard<mutex> lock(writer);
      deque<std::shared_ptr<SegmentCommitInfo>> infos =
          getInfosToApply(writer);
      if (infos.empty()) {
        break;
      }

      for (auto info : infos) {
        delFiles->addAll(info->files());
      }

      // Must open while holding IW lock so that e.g. segments are not merged
      // away, dropped from 100% deletions, etc., before we can open the readers
      segStates = openSegmentStates(writer, infos, seenSegments, delGen());

      if (segStates.empty()) {

        if (infoStream->isEnabled(L"BD")) {
          infoStream->message(L"BD", L"packet matches no segments");
        }
        break;
      }

      if (infoStream->isEnabled(L"BD")) {
        infoStream->message(
            L"BD",
            wstring::format(
                Locale::ROOT,
                messagePrefix +
                    L"now apply del packet (%s) to %d segments, mergeGen %d",
                shared_from_this(), segStates.size(), mergeGenStart));
      }

      totalSegmentCount += segStates.size();

      // Important, else IFD may try to delete our files while we are still
      // using them, if e.g. a merge finishes on some of the segments we are
      // resolving on:
      writer->deleter->incRef(delFiles);
    }

    shared_ptr<AtomicBoolean> success = make_shared<AtomicBoolean>();
    int64_t delCount;
    {

      java::io::Closeable finalizer = [&]() {
        finishApply(writer, segStates, success->get(), delFiles);
      };
      // don't hold IW monitor lock here so threads are free concurrently
      // resolve deletes/updates:
      delCount = apply(segStates);
      success::set(true);
    }

    // Since we just resolved some more deletes/updates, now is a good time to
    // write them:
    writer->writeSomeDocValuesUpdates();

    // It's OK to add this here, even if the while loop retries, because
    // delCount only includes newly deleted documents, on the segments we didn't
    // already do in previous iterations:
    totalDelCount += delCount;

    if (infoStream->isEnabled(L"BD")) {
      infoStream->message(
          L"BD", wstring::format(
                     Locale::ROOT,
                     messagePrefix +
                         L"done inner apply del packet (%s) to %d segments; %d "
                         L"new deletes/updates; took %.3f sec",
                     shared_from_this(), segStates->length, delCount,
                     (System::nanoTime() - iterStartNS) / 1000000000.0));
    }
    if (privateSegment != nullptr) {
      // No need to retry for a segment-private packet: the merge that folds in
      // our private segment already waits for all deletes to be applied before
      // it kicks off, so this private segment must already not be in the set of
      // merging segments

      break;
    }

    // Must sync on writer here so that IW.mergeCommit is not running
    // concurrently, so that if we exit, we know mergeCommit will succeed in
    // pulling all our delGens into a merge:
    {
      lock_guard<mutex> lock(writer);
      int64_t mergeGenCur = writer->mergeFinishedGen->get();

      if (mergeGenCur == mergeGenStart) {

        // Must do this while still holding IW lock else a merge could finish
        // and skip carrying over our updates:

        // Record that this packet is finished:
        writer->finished(shared_from_this());

        finished = true;

        // No merge finished while we were applying, so we are done!
        break;
      }
    }

    if (infoStream->isEnabled(L"BD")) {
      infoStream->message(L"BD",
                          messagePrefix +
                              L"concurrent merges finished; move to next iter");
    }

    // A merge completed while we were running.  In this case, that merge may
    // have picked up some of the updates we did, but not necessarily all of
    // them, so we cycle again, re-applying all our updates to the newly merged
    // segment.

    iter++;
  }

  if (finished == false) {
    // Record that this packet is finished:
    writer->finished(shared_from_this());
  }

  if (infoStream->isEnabled(L"BD")) {
    wstring message =
        wstring::format(Locale::ROOT,
                        L"done apply del packet (%s) to %d segments; %d new "
                        L"deletes/updates; took %.3f sec",
                        shared_from_this(), totalSegmentCount, totalDelCount,
                        (System::nanoTime() - startNS) / 1000000000.0);
    if (iter > 0) {
      message += L"; " + (iter + 1) + L" iters due to concurrent merges";
    }
    message += L"; " + to_wstring(writer->getPendingUpdatesCount()) +
               L" packets remain";
    infoStream->message(L"BD", message);
  }
}

std::deque<std::shared_ptr<BufferedUpdatesStream::SegmentState>>
FrozenBufferedUpdates::openSegmentStates(
    shared_ptr<IndexWriter> writer,
    deque<std::shared_ptr<SegmentCommitInfo>> &infos,
    shared_ptr<Set<std::shared_ptr<SegmentCommitInfo>>> alreadySeenSegments,
    int64_t delGen) 
{
  deque<std::shared_ptr<BufferedUpdatesStream::SegmentState>> segStates =
      deque<std::shared_ptr<BufferedUpdatesStream::SegmentState>>();
  try {
    for (auto info : infos) {
      if (info->getBufferedDeletesGen() <= delGen &&
          alreadySeenSegments->contains(info) == false) {
        segStates.push_back(make_shared<BufferedUpdatesStream::SegmentState>(
            writer->getPooledInstance(info, true), writer::release, info));
        alreadySeenSegments->add(info);
      }
    }
  } catch (const runtime_error &t) {
    try {
      IOUtils::close(segStates);
    } catch (const runtime_error &t1) {
      t.addSuppressed(t1);
    }
    throw t;
  }

  return segStates.toArray(
      std::deque<std::shared_ptr<BufferedUpdatesStream::SegmentState>>(0));
}

shared_ptr<BufferedUpdatesStream::ApplyDeletesResult>
FrozenBufferedUpdates::closeSegmentStates(
    shared_ptr<IndexWriter> writer,
    std::deque<std::shared_ptr<BufferedUpdatesStream::SegmentState>>
        &segStates,
    bool success) 
{
  deque<std::shared_ptr<SegmentCommitInfo>> allDeleted;
  int64_t totDelCount = 0;
  const deque<std::shared_ptr<BufferedUpdatesStream::SegmentState>>
      segmentStates = Arrays::asList(segStates);
  for (auto segState : segmentStates) {
    if (success) {
      totDelCount += segState->rld->getDelCount() - segState->startDelCount;
      int fullDelCount = segState->rld->getDelCount();
      assert((fullDelCount <= segState->rld->info->info->maxDoc(),
              to_wstring(fullDelCount) + L" > " +
                  to_wstring(segState->rld->info->info->maxDoc())));
      if (segState->rld->isFullyDeleted() &&
          writer->getConfig()->getMergePolicy()->keepFullyDeletedSegment(
              [&]() { segState->reader; }) == false) {
        if (allDeleted.empty()) {
          allDeleted = deque<>();
        }
        allDeleted.push_back(segState->reader->getOriginalSegmentInfo());
      }
    }
  }
  IOUtils::close(segmentStates);
  if (writer->infoStream->isEnabled(L"BD")) {
    writer->infoStream->message(
        L"BD", L"closeSegmentStates: " + to_wstring(totDelCount) +
                   L" new deleted documents; pool " +
                   to_wstring(writer->getPendingUpdatesCount()) +
                   L" packets; bytesUsed=" +
                   to_wstring(writer->getReaderPoolRamBytesUsed()));
  }

  return make_shared<BufferedUpdatesStream::ApplyDeletesResult>(totDelCount > 0,
                                                                allDeleted);
}

void FrozenBufferedUpdates::finishApply(
    shared_ptr<IndexWriter> writer,
    std::deque<std::shared_ptr<BufferedUpdatesStream::SegmentState>>
        &segStates,
    bool success, shared_ptr<Set<wstring>> delFiles) 
{
  {
    lock_guard<mutex> lock(writer);

    shared_ptr<BufferedUpdatesStream::ApplyDeletesResult> result;
    try {
      result = closeSegmentStates(writer, segStates, success);
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      // Matches the incRef we did above, but we must do the decRef after
      // closing segment states else IFD can't delete still-open files
      writer->deleter->decRef(delFiles);
    }

    if (result->anyDeletes) {
      writer->maybeMerge_.set(true);
      writer->checkpoint();
    }

    if (result->allDeleted.size() > 0) {
      if (infoStream->isEnabled(L"IW")) {
        infoStream->message(L"IW", L"drop 100% deleted segments: " +
                                       writer->segString(result->allDeleted));
      }
      for (auto info : result->allDeleted) {
        writer->dropDeletedSegment(info);
      }
      writer->checkpoint();
    }
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
int64_t FrozenBufferedUpdates::apply(
    std::deque<std::shared_ptr<BufferedUpdatesStream::SegmentState>>
        &segStates) 
{

  if (delGen_ == -1) {
    // we were not yet pushed
    throw invalid_argument(
        L"gen is not yet set; call BufferedUpdatesStream.push first");
  }

  assert(applied->getCount() != 0);

  if (privateSegment != nullptr) {
    assert(segStates.size() == 1);
    assert(privateSegment == segStates[0]->reader.getOriginalSegmentInfo());
  }

  totalDelCount += applyTermDeletes(segStates);
  totalDelCount += applyQueryDeletes(segStates);
  totalDelCount += applyDocValuesUpdates(segStates);

  return totalDelCount;
}

int64_t FrozenBufferedUpdates::applyDocValuesUpdates(
    std::deque<std::shared_ptr<BufferedUpdatesStream::SegmentState>>
        &segStates) 
{

  if (numericDVUpdates.empty() && binaryDVUpdates.empty()) {
    return 0;
  }

  int64_t startNS = System::nanoTime();

  int64_t updateCount = 0;

  for (auto segState : segStates) {

    if (delGen_ < segState->delGen) {
      // segment is newer than this deletes packet
      continue;
    }

    if (segState->rld->refCount() == 1) {
      // This means we are the only remaining reference to this segment, meaning
      // it was merged away while we were running, so we can safely skip running
      // because we will run on the newly merged segment next:
      continue;
    }
    constexpr bool isSegmentPrivateDeletes = privateSegment != nullptr;
    if (numericDVUpdates.size() > 0) {
      updateCount += applyDocValuesUpdates(segState, numericDVUpdates, true,
                                           delGen_, isSegmentPrivateDeletes);
    }

    if (binaryDVUpdates.size() > 0) {
      updateCount += applyDocValuesUpdates(segState, binaryDVUpdates, false,
                                           delGen_, isSegmentPrivateDeletes);
    }
  }

  if (infoStream->isEnabled(L"BD")) {
    infoStream->message(
        L"BD", wstring::format(
                   Locale::ROOT,
                   L"applyDocValuesUpdates %.1f msec for %d segments, %d "
                   L"numeric updates and %d binary updates; %d new updates",
                   (System::nanoTime() - startNS) / 1000000.0, segStates.size(),
                   numericDVUpdateCount, binaryDVUpdateCount, updateCount));
  }

  return updateCount;
}

int64_t FrozenBufferedUpdates::applyDocValuesUpdates(
    shared_ptr<BufferedUpdatesStream::SegmentState> segState,
    std::deque<char> &updates, bool isNumeric, int64_t delGen,
    bool segmentPrivateDeletes) 
{

  shared_ptr<TermsEnum> termsEnum = nullptr;
  shared_ptr<PostingsEnum> postingsEnum = nullptr;

  // TODO: we can process the updates per DV field, from last to first so that
  // if multiple terms affect same document for the same field, we add an update
  // only once (that of the last term). To do that, we can keep a bitset which
  // marks which documents have already been updated. So e.g. if term T1
  // updates doc 7, and then we process term T2 and it updates doc 7 as well,
  // we don't apply the update since we know T1 came last and therefore wins
  // the update.
  // We can also use that bitset as 'liveDocs' to pass to TermEnum.docs(), so
  // that these documents aren't even returned.

  int64_t updateCount = 0;

  // We first write all our updates private, and only in the end publish to the
  // ReadersAndUpdates */
  unordered_map<wstring, std::shared_ptr<DocValuesFieldUpdates>> holder =
      unordered_map<wstring, std::shared_ptr<DocValuesFieldUpdates>>();

  shared_ptr<ByteArrayDataInput> in_ = make_shared<ByteArrayDataInput>(updates);

  wstring termField = L"";
  wstring updateField = L"";
  shared_ptr<BytesRef> term = make_shared<BytesRef>();
  term->bytes = std::deque<char>(16);

  shared_ptr<BytesRef> scratch = make_shared<BytesRef>();
  scratch->bytes = std::deque<char>(16);

  while (in_->getPosition() != updates.size()) {
    int code = in_->readVInt();
    int docIDUpto = in_->readVInt();
    term->length = code >> 3;

    if ((code & 1) != 0) {
      termField = in_->readString();
    }
    if ((code & 2) != 0) {
      updateField = in_->readString();
    }
    bool hasValue = (code & 4) != 0;

    if (term->bytes.size() < term->length) {
      term->bytes = ArrayUtil::grow(term->bytes, term->length);
    }
    in_->readBytes(term->bytes, 0, term->length);

    constexpr int limit;
    if (delGen == segState->delGen) {
      assert(segmentPrivateDeletes);
      limit = docIDUpto;
    } else {
      limit = numeric_limits<int>::max();
    }

    // TODO: we traverse the terms in update order (not term order) so that we
    // apply the updates in the correct order, i.e. if two terms udpate the
    // same document, the last one that came in wins, irrespective of the
    // terms lexical order.
    // we can apply the updates in terms order if we keep an updatesGen (and
    // increment it with every update) and attach it to each NumericUpdate. Note
    // that we cannot rely only on docIDUpto because an app may send two updates
    // which will get same docIDUpto, yet will still need to respect the order
    // those updates arrived.

    // TODO: we could at least *collate* by field?

    // This is the field used to resolve to docIDs, e.g. an "id" field, not the
    // doc values field we are updating!
    if ((code & 1) != 0) {
      shared_ptr<Terms> terms = segState->reader->terms(termField);
      if (terms != nullptr) {
        termsEnum = terms->begin();
      } else {
        termsEnum.reset();
      }
    }

    shared_ptr<BytesRef> *const binaryValue;
    constexpr int64_t longValue;
    if (hasValue == false) {
      longValue = -1;
      binaryValue.reset();
    } else if (isNumeric) {
      longValue = NumericDocValuesUpdate::readFrom(in_);
      binaryValue.reset();
    } else {
      longValue = -1;
      binaryValue = BinaryDocValuesUpdate::readFrom(in_, scratch);
    }

    if (termsEnum == nullptr) {
      // no terms in this segment for this field
      continue;
    }

    if (termsEnum->seekExact(term)) {
      // we don't need term frequencies for this
      postingsEnum = termsEnum->postings(postingsEnum, PostingsEnum::NONE);
      shared_ptr<DocValuesFieldUpdates> dvUpdates = holder[updateField];
      if (dvUpdates == nullptr) {
        if (isNumeric) {
          dvUpdates = make_shared<NumericDocValuesFieldUpdates>(
              delGen, updateField, segState->reader->maxDoc());
        } else {
          dvUpdates = make_shared<BinaryDocValuesFieldUpdates>(
              delGen, updateField, segState->reader->maxDoc());
        }
        holder.emplace(updateField, dvUpdates);
      }
      const function<void(int)> docIdConsumer;
      shared_ptr<DocValuesFieldUpdates> *const update = dvUpdates;
      if (hasValue == false) {
        docIdConsumer = [&](int doc) { update->reset(doc); };
      } else if (isNumeric) {
        docIdConsumer = [&](int doc) { update->add(doc, longValue); };
      } else {
        docIdConsumer = [&](int doc) { update->add(doc, binaryValue); };
      }
      shared_ptr<Bits> *const acceptDocs = segState->rld->getLiveDocs();
      if (segState->rld->sortMap != nullptr && segmentPrivateDeletes) {
        // This segment was sorted on flush; we must apply seg-private deletes
        // carefully in this case:
        int doc;
        while ((doc = postingsEnum->nextDoc()) !=
               DocIdSetIterator::NO_MORE_DOCS) {
          if (acceptDocs == nullptr || acceptDocs->get(doc)) {
            // The limit is in the pre-sorted doc space:
            if (segState->rld->sortMap->newToOld(doc) < limit) {
              docIdConsumer(doc);
              updateCount++;
            }
          }
        }
      } else {
        int doc;
        while ((doc = postingsEnum->nextDoc()) !=
               DocIdSetIterator::NO_MORE_DOCS) {
          if (doc >= limit) {
            break; // no more docs that can be updated for this term
          }
          if (acceptDocs == nullptr || acceptDocs->get(doc)) {
            docIdConsumer(doc);
            updateCount++;
          }
        }
      }
    }
  }

  // now freeze & publish:
  for (auto update : holder) {
    if (update->second.any()) {
      update->second.finish();
      segState->rld->addDVUpdate(update->second);
    }
  }

  return updateCount;
}

int64_t FrozenBufferedUpdates::applyQueryDeletes(
    std::deque<std::shared_ptr<BufferedUpdatesStream::SegmentState>>
        &segStates) 
{

  if (deleteQueries.empty()) {
    return 0;
  }

  int64_t startNS = System::nanoTime();

  int64_t delCount = 0;
  for (auto segState : segStates) {

    if (delGen_ < segState->delGen) {
      // segment is newer than this deletes packet
      continue;
    }

    if (segState->rld->refCount() == 1) {
      // This means we are the only remaining reference to this segment, meaning
      // it was merged away while we were running, so we can safely skip running
      // because we will run on the newly merged segment next:
      continue;
    }

    shared_ptr<LeafReaderContext> *const readerContext =
        segState->reader->getContext();
    for (int i = 0; i < deleteQueries.size(); i++) {
      shared_ptr<Query> query = deleteQueries[i];
      int limit;
      if (delGen_ == segState->delGen) {
        assert(privateSegment != nullptr);
        limit = deleteQueryLimits[i];
      } else {
        limit = numeric_limits<int>::max();
      }
      shared_ptr<IndexSearcher> *const searcher =
          make_shared<IndexSearcher>(readerContext->reader());
      searcher->setQueryCache(nullptr);
      query = searcher->rewrite(query);
      shared_ptr<Weight> *const weight =
          searcher->createWeight(query, false, 1);
      shared_ptr<Scorer> *const scorer = weight->scorer(readerContext);
      if (scorer != nullptr) {
        shared_ptr<DocIdSetIterator> *const it = scorer->begin();

        int docID;
        while ((docID = it->nextDoc()) < limit) {
          if (segState->rld->delete (docID)) {
            delCount++;
          }
        }
      }
    }
  }

  if (infoStream->isEnabled(L"BD")) {
    infoStream->message(
        L"BD",
        wstring::format(Locale::ROOT,
                        L"applyQueryDeletes took %.2f msec for %d segments and "
                        L"%d queries; %d new deletions",
                        (System::nanoTime() - startNS) / 1000000.0,
                        segStates.size(), deleteQueries.size(), delCount));
  }

  return delCount;
}

int64_t FrozenBufferedUpdates::applyTermDeletes(
    std::deque<std::shared_ptr<BufferedUpdatesStream::SegmentState>>
        &segStates) 
{

  if (deleteTerms->size() == 0) {
    return 0;
  }

  // We apply segment-private deletes on flush:
  assert(privateSegment == nullptr);

  int64_t startNS = System::nanoTime();

  int64_t delCount = 0;

  for (auto segState : segStates) {
    assert((segState->delGen != delGen_,
            L"segState.delGen=" + to_wstring(segState->delGen) +
                L" vs this.gen=" + to_wstring(delGen_)));
    if (segState->delGen > delGen_) {
      // our deletes don't apply to this segment
      continue;
    }
    if (segState->rld->refCount() == 1) {
      // This means we are the only remaining reference to this segment, meaning
      // it was merged away while we were running, so we can safely skip running
      // because we will run on the newly merged segment next:
      continue;
    }

    shared_ptr<FieldTermIterator> iter = deleteTerms->begin();

    shared_ptr<BytesRef> delTerm;
    wstring field = L"";
    shared_ptr<TermsEnum> termsEnum = nullptr;
    shared_ptr<BytesRef> readerTerm = nullptr;
    shared_ptr<PostingsEnum> postingsEnum = nullptr;
    while ((delTerm = iter->next()) != nullptr) {

      if (iter->field() != field) {
        // field changed
        field = iter->field();
        shared_ptr<Terms> terms = segState->reader->terms(field);
        if (terms != nullptr) {
          termsEnum = terms->begin();
          readerTerm = termsEnum->next();
        } else {
          termsEnum.reset();
        }
      }

      if (termsEnum != nullptr) {
        int cmp = delTerm->compareTo(readerTerm);
        if (cmp < 0) {
          // TODO: can we advance across del terms here?
          // move to next del term
          continue;
        } else if (cmp == 0) {
          // fall through
        } else if (cmp > 0) {
          TermsEnum::SeekStatus status = termsEnum->seekCeil(delTerm);
          if (status == TermsEnum::SeekStatus::FOUND) {
            // fall through
          } else if (status == TermsEnum::SeekStatus::NOT_FOUND) {
            readerTerm = termsEnum->term();
            continue;
          } else {
            // TODO: can we advance to next field in deleted terms?
            // no more terms in this segment
            termsEnum.reset();
            continue;
          }
        }

        // we don't need term frequencies for this
        postingsEnum = termsEnum->postings(postingsEnum, PostingsEnum::NONE);

        assert(postingsEnum != nullptr);

        int docID;
        while ((docID = postingsEnum->nextDoc()) !=
               DocIdSetIterator::NO_MORE_DOCS) {

          // NOTE: there is no limit check on the docID
          // when deleting by Term (unlike by Query)
          // because on flush we apply all Term deletes to
          // each segment.  So all Term deleting here is
          // against prior segments:
          if (segState->rld->delete (docID)) {
            delCount++;
          }
        }
      }
    }
  }

  if (infoStream->isEnabled(L"BD")) {
    infoStream->message(
        L"BD",
        wstring::format(Locale::ROOT,
                        L"applyTermDeletes took %.2f msec for %d segments and "
                        L"%d del terms; %d new deletions",
                        (System::nanoTime() - startNS) / 1000000.0,
                        segStates.size(), deleteTerms->size(), delCount));
  }

  return delCount;
}

void FrozenBufferedUpdates::setDelGen(int64_t delGen)
{
  assert((this->delGen_ == -1, L"delGen was already previously set to " +
                                   to_wstring(this->delGen_)));
  this->delGen_ = delGen;
  deleteTerms->setDelGen(delGen);
}

int64_t FrozenBufferedUpdates::delGen()
{
  assert(delGen_ != -1);
  return delGen_;
}

wstring FrozenBufferedUpdates::toString()
{
  wstring s = L"delGen=" + to_wstring(delGen_);
  if (numTermDeletes != 0) {
    s += L" numDeleteTerms=" + to_wstring(numTermDeletes);
    if (numTermDeletes != deleteTerms->size()) {
      s += L" (" + to_wstring(deleteTerms->size()) + L" unique)";
    }
  }
  if (deleteQueries.size() != 0) {
    s += L" numDeleteQueries=" + deleteQueries.size();
  }
  if (numericDVUpdates.size() > 0) {
    s += L" numNumericDVUpdates=" + to_wstring(numericDVUpdateCount);
  }
  if (binaryDVUpdates.size() > 0) {
    s += L" numBinaryDVUpdates=" + to_wstring(binaryDVUpdateCount);
  }
  if (bytesUsed != 0) {
    s += L" bytesUsed=" + to_wstring(bytesUsed);
  }
  if (privateSegment != nullptr) {
    s += L" privateSegment=" + privateSegment;
  }

  return s;
}

bool FrozenBufferedUpdates::any()
{
  return deleteTerms->size() > 0 || deleteQueries.size() > 0 ||
         numericDVUpdates.size() > 0 || binaryDVUpdates.size() > 0;
}
} // namespace org::apache::lucene::index