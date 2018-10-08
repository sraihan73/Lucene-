using namespace std;

#include "NRTSuggester.h"

namespace org::apache::lucene::search::suggest::document
{
using FSTUtil = org::apache::lucene::search::suggest::analyzing::FSTUtil;
using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using ByteArrayDataOutput = org::apache::lucene::store::ByteArrayDataOutput;
using IndexInput = org::apache::lucene::store::IndexInput;
using Accountable = org::apache::lucene::util::Accountable;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;
using ByteSequenceOutputs = org::apache::lucene::util::fst::ByteSequenceOutputs;
using FST = org::apache::lucene::util::fst::FST;
using org::apache::lucene::util::fst::PairOutputs::Pair;
using PairOutputs = org::apache::lucene::util::fst::PairOutputs;
using PositiveIntOutputs = org::apache::lucene::util::fst::PositiveIntOutputs;
using Util = org::apache::lucene::util::fst::Util;
//    import static
//    org.apache.lucene.search.suggest.document.NRTSuggester.PayLoadProcessor.parseSurfaceForm;

NRTSuggester::NRTSuggester(
    shared_ptr<FST<Pair<int64_t, std::shared_ptr<BytesRef>>>> fst,
    int maxAnalyzedPathsPerOutput, int payloadSep)
    : fst(fst), maxAnalyzedPathsPerOutput(maxAnalyzedPathsPerOutput),
      payloadSep(payloadSep)
{
}

int64_t NRTSuggester::ramBytesUsed()
{
  return fst == nullptr ? 0 : fst->ramBytesUsed();
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
NRTSuggester::getChildResources()
{
  return Collections::emptyList();
}

void NRTSuggester::lookup(
    shared_ptr<CompletionScorer> scorer, shared_ptr<Bits> acceptDocs,
    shared_ptr<TopSuggestDocsCollector> collector) 
{
  constexpr double liveDocsRatio = calculateLiveDocRatio(
      scorer->reader->numDocs(), scorer->reader->maxDoc());
  if (liveDocsRatio == -1) {
    return;
  }
  const deque<FSTUtil::Path<Pair<int64_t, std::shared_ptr<BytesRef>>>>
      prefixPaths = FSTUtil::intersectPrefixPaths(scorer->automaton, fst);
  // The topN is increased by a factor of # of intersected path
  // to ensure search admissibility. For example, one suggestion can
  // have multiple contexts, resulting in num_context paths for the
  // suggestion instead of 1 in the FST. When queried for the suggestion,
  // the topN value ensures that all paths to the suggestion are evaluated
  // (in case of a match all context query).
  // Note that collectors will early terminate as soon as enough suggestions
  // have been collected, regardless of the set topN value. This value is the
  // maximum number of suggestions that can be collected.
  constexpr int topN = collector->getCountToCollect() * prefixPaths.size();
  constexpr int queueSize = getMaxTopNSearcherQueueSize(
      topN, scorer->reader->numDocs(), liveDocsRatio, scorer->filtered);

  shared_ptr<CharsRefBuilder> *const spare = make_shared<CharsRefBuilder>();

  shared_ptr<Comparator<Pair<int64_t, std::shared_ptr<BytesRef>>>>
      comparator = getComparator();
  shared_ptr<Util::TopNSearcher<Pair<int64_t, std::shared_ptr<BytesRef>>>>
      searcher = make_shared<TopNSearcherAnonymousInnerClass>(
          shared_from_this(), fst, comparator,
          make_shared<ScoringPathComparator>(scorer), scorer, acceptDocs,
          collector, spare);

  for (auto path : prefixPaths) {
    scorer->weight->setNextMatch(path->input->get());
    shared_ptr<BytesRef> output = path->output->output2;
    int payload = -1;
    if (collector->doSkipDuplicates()) {
      for (int j = 0; j < output->length; j++) {
        if (output->bytes[output->offset + j] == payloadSep) {
          // Important to cache this, else we have a possibly O(N^2) cost where
          // N is the length of suggestions
          payload = j;
          break;
        }
      }
    }

    searcher->addStartPaths(path->fstNode, path->output, false, path->input,
                            scorer->weight->boost(), scorer->weight->context(),
                            payload);
  }
  // hits are also returned by search()
  // we do not use it, instead collect at acceptResult
  searcher->search();
  // search admissibility is not guaranteed
  // see comment on getMaxTopNSearcherQueueSize
  // assert  search.isComplete;
}

NRTSuggester::TopNSearcherAnonymousInnerClass::TopNSearcherAnonymousInnerClass(
    shared_ptr<NRTSuggester> outerInstance,
    shared_ptr<FST<Pair<int64_t, std::shared_ptr<BytesRef>>>> fst,
    shared_ptr<Comparator<Pair<int64_t, std::shared_ptr<BytesRef>>>>
        comparator,
    shared_ptr<ScoringPathComparator> new,
    shared_ptr<org::apache::lucene::search::suggest::document::CompletionScorer>
        scorer,
    shared_ptr<Bits> acceptDocs,
    shared_ptr<
        org::apache::lucene::search::suggest::document::TopSuggestDocsCollector>
        collector,
    shared_ptr<CharsRefBuilder> spare) new
    : org::apache::lucene::util::fst::Util::TopNSearcher<
          org::apache::lucene::util::fst::PairOutputs::Pair<
              long, org::apache::lucene::util::BytesRef>>(
          fst, topN, queueSize, comparator, new ScoringPathComparator)
{
  this->outerInstance = outerInstance;
  this->scorer = scorer;
  this->acceptDocs = acceptDocs;
  this->collector = collector;
  this->spare = spare;
  scratchInput = make_shared<ByteArrayDataInput>();
}

bool NRTSuggester::TopNSearcherAnonymousInnerClass::acceptPartialPath(
    shared_ptr<Util::FSTPath<Pair<int64_t, std::shared_ptr<BytesRef>>>> path)
{
  if (collector->doSkipDuplicates()) {
    // We are removing dups
    if (path->payload == -1) {
      // This path didn't yet see the complete surface form; let's see if it
      // just did with the arc output we just added:
      shared_ptr<BytesRef> arcOutput = path->arc->output->output2;
      shared_ptr<BytesRef> output = path->output->output2;
      for (int i = 0; i < arcOutput->length; i++) {
        if (arcOutput->bytes[arcOutput->offset + i] ==
            outerInstance->payloadSep) {
          // OK this arc that the path was just extended by contains the
          // payloadSep, so we now have a full surface form in this path
          path->payload = output->length - arcOutput->length + i;
          assert(output->bytes[output->offset + path->payload] ==
                 outerInstance->payloadSep);
          break;
        }
      }
    }

    if (path->payload != -1) {
      shared_ptr<BytesRef> output = path->output->output2;
      spare->copyUTF8Bytes(output->bytes, output->offset, path->payload);
      if (collector->seenSurfaceForms->contains(spare->chars(), 0,
                                                spare->length())) {
        return false;
      }
    }
  }
  return true;
}

bool NRTSuggester::TopNSearcherAnonymousInnerClass::acceptResult(
    shared_ptr<Util::FSTPath<Pair<int64_t, std::shared_ptr<BytesRef>>>> path)
{
  shared_ptr<BytesRef> output = path->output->output2;
  int payloadSepIndex;
  if (path->payload != -1) {
    payloadSepIndex = path->payload;
    spare->copyUTF8Bytes(output->bytes, output->offset, payloadSepIndex);
  } else {
    assert(collector->doSkipDuplicates() == false);
    payloadSepIndex =
        parseSurfaceForm(output, outerInstance->payloadSep, spare);
  }

  scratchInput::reset(output->bytes, output->offset + payloadSepIndex + 1,
                      output->length - payloadSepIndex - 1);
  int docID = scratchInput::readVInt();

  if (!scorer->accept(docID, acceptDocs)) {
    return false;
  }
  if (collector->doSkipDuplicates()) {
    // now record that we've seen this surface form:
    std::deque<wchar_t> key(spare->length());
    System::arraycopy(spare->chars(), 0, key, 0, spare->length());
    if (collector->seenSurfaceForms->contains(key)) {
      // we already collected a higher scoring document with this key, in this
      // segment:
      return false;
    }
    collector->seenSurfaceForms->add(key);
  }
  try {
    float score = scorer->score(decode(path->output->output1), path->boost);
    collector->collect(docID, spare->toCharsRef(), path->context, score);
    return true;
  } catch (const IOException &e) {
    throw runtime_error(e);
  }
}

NRTSuggester::ScoringPathComparator::ScoringPathComparator(
    shared_ptr<CompletionScorer> scorer)
    : scorer(scorer)
{
}

int NRTSuggester::ScoringPathComparator::compare(
    shared_ptr<Util::FSTPath<Pair<int64_t, std::shared_ptr<BytesRef>>>> first,
    shared_ptr<Util::FSTPath<Pair<int64_t, std::shared_ptr<BytesRef>>>>
        second)
{
  int cmp = Float::compare(
      scorer->score(decode(second->output->output1), second->boost),
      scorer->score(decode(first->output->output1), first->boost));
  return (cmp != 0) ? cmp
                    : first->input->get()->compareTo(second->input->get());
}

shared_ptr<Comparator<Pair<int64_t, std::shared_ptr<BytesRef>>>>
NRTSuggester::getComparator()
{
  return make_shared<ComparatorAnonymousInnerClass>();
}

NRTSuggester::ComparatorAnonymousInnerClass::ComparatorAnonymousInnerClass() {}

int NRTSuggester::ComparatorAnonymousInnerClass::compare(
    shared_ptr<Pair<int64_t, std::shared_ptr<BytesRef>>> o1,
    shared_ptr<Pair<int64_t, std::shared_ptr<BytesRef>>> o2)
{
  return Long::compare(o1->output1, o2->output1);
}

int NRTSuggester::getMaxTopNSearcherQueueSize(int topN, int numDocs,
                                              double liveDocsRatio,
                                              bool filterEnabled)
{
  int64_t maxQueueSize = topN * maxAnalyzedPathsPerOutput;
  // liveDocRatio can be at most 1.0 (if no docs were deleted)
  assert(liveDocsRatio <= 1.0);
  maxQueueSize = static_cast<int64_t>(maxQueueSize / liveDocsRatio);
  if (filterEnabled) {
    maxQueueSize = maxQueueSize + (numDocs / 2);
  }
  return static_cast<int>(min(MAX_TOP_N_QUEUE_SIZE, maxQueueSize));
}

double NRTSuggester::calculateLiveDocRatio(int numDocs, int maxDocs)
{
  return (numDocs > 0) ? (static_cast<double>(numDocs) / maxDocs) : -1;
}

shared_ptr<NRTSuggester>
NRTSuggester::load(shared_ptr<IndexInput> input) 
{
  shared_ptr<FST<Pair<int64_t, std::shared_ptr<BytesRef>>>> *const fst =
      make_shared<FST<Pair<int64_t, std::shared_ptr<BytesRef>>>>(
          input,
          make_shared<PairOutputs<>>(PositiveIntOutputs::getSingleton(),
                                     ByteSequenceOutputs::getSingleton()));

  /* read some meta info */
  int maxAnalyzedPathsPerOutput = input->readVInt();
  /*
   * Label used to denote the end of an input in the FST and
   * the beginning of dedup bytes
   */
  int endByte = input->readVInt();
  int payloadSep = input->readVInt();

  return make_shared<NRTSuggester>(fst, maxAnalyzedPathsPerOutput, payloadSep);
}

int64_t NRTSuggester::encode(int64_t input)
{
  if (input < 0 || input > numeric_limits<int>::max()) {
    throw make_shared<UnsupportedOperationException>(L"cannot encode value: " +
                                                     to_wstring(input));
  }
  return numeric_limits<int>::max() - input;
}

int64_t NRTSuggester::decode(int64_t output)
{
  assert((output >= 0 && output <= std,
          : numeric_limits<int>::max()
          : L"decoded output: " + to_wstring(output) +
                L" is not within 0 and Integer.MAX_VALUE"));
  return numeric_limits<int>::max() - output;
}

int NRTSuggester::PayLoadProcessor::parseSurfaceForm(
    shared_ptr<BytesRef> output, int payloadSep,
    shared_ptr<CharsRefBuilder> spare)
{
  int surfaceFormLen = -1;
  for (int i = 0; i < output->length; i++) {
    if (output->bytes[output->offset + i] == payloadSep) {
      surfaceFormLen = i;
      break;
    }
  }
  assert((surfaceFormLen != -1,
          L"no payloadSep found, unable to determine surface form"));
  spare->copyUTF8Bytes(output->bytes, output->offset, surfaceFormLen);
  return surfaceFormLen;
}

shared_ptr<BytesRef>
NRTSuggester::PayLoadProcessor::make(shared_ptr<BytesRef> surface, int docID,
                                     int payloadSep) 
{
  int len = surface->length + MAX_DOC_ID_LEN_WITH_SEP;
  std::deque<char> buffer(len);
  shared_ptr<ByteArrayDataOutput> output =
      make_shared<ByteArrayDataOutput>(buffer);
  output->writeBytes(surface->bytes, surface->length - surface->offset);
  output->writeByte(static_cast<char>(payloadSep));
  output->writeVInt(docID);
  return make_shared<BytesRef>(buffer, 0, output->getPosition());
}
} // namespace org::apache::lucene::search::suggest::document