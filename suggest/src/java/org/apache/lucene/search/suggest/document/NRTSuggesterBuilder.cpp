using namespace std;

#include "NRTSuggesterBuilder.h"

namespace org::apache::lucene::search::suggest::document
{
using ConcatenateGraphFilter =
    org::apache::lucene::analysis::miscellaneous::ConcatenateGraphFilter;
using DataOutput = org::apache::lucene::store::DataOutput;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using Builder = org::apache::lucene::util::fst::Builder;
using ByteSequenceOutputs = org::apache::lucene::util::fst::ByteSequenceOutputs;
using FST = org::apache::lucene::util::fst::FST;
using PairOutputs = org::apache::lucene::util::fst::PairOutputs;
using PositiveIntOutputs = org::apache::lucene::util::fst::PositiveIntOutputs;
using Util = org::apache::lucene::util::fst::Util;
//    import static
//    org.apache.lucene.search.suggest.document.NRTSuggester.encode;

NRTSuggesterBuilder::NRTSuggesterBuilder()
    : outputs(make_shared<PairOutputs<>>(PositiveIntOutputs::getSingleton(),
                                         ByteSequenceOutputs::getSingleton())),
      builder(make_shared<Builder<>>(FST::INPUT_TYPE::BYTE1, outputs)),
      entries(make_shared<PriorityQueue<>>()), payloadSep(PAYLOAD_SEP),
      endByte(END_BYTE)
{
}

void NRTSuggesterBuilder::startTerm(shared_ptr<BytesRef> analyzed)
{
  this->analyzed->copyBytes(analyzed);
  this->analyzed->append(static_cast<char>(endByte));
}

void NRTSuggesterBuilder::addEntry(int docID, shared_ptr<BytesRef> surfaceForm,
                                   int64_t weight) 
{
  shared_ptr<BytesRef> payloadRef =
      NRTSuggester::PayLoadProcessor::make(surfaceForm, docID, payloadSep);
  entries->add(make_shared<Entry>(payloadRef, encode(weight)));
}

void NRTSuggesterBuilder::finishTerm() 
{
  int numArcs = 0;
  int numDedupBytes = 1;
  analyzed->grow(analyzed->length() + 1);
  analyzed->setLength(analyzed->length() + 1);
  for (auto entry : entries) {
    if (numArcs == maxNumArcsForDedupByte(numDedupBytes)) {
      analyzed->setByteAt(analyzed->length() - 1, static_cast<char>(numArcs));
      analyzed->grow(analyzed->length() + 1);
      analyzed->setLength(analyzed->length() + 1);
      numArcs = 0;
      numDedupBytes++;
    }
    analyzed->setByteAt(analyzed->length() - 1, static_cast<char>(numArcs)++);
    Util::toIntsRef(analyzed->get(), scratchInts);
    builder->add(scratchInts->get(),
                 outputs->newPair(entry->weight, entry->payload));
  }
  maxAnalyzedPathsPerOutput = max(maxAnalyzedPathsPerOutput, entries->size());
  entries->clear();
}

bool NRTSuggesterBuilder::store(shared_ptr<DataOutput> output) throw(
    IOException)
{
  shared_ptr<FST<PairOutputs::Pair<int64_t, std::shared_ptr<BytesRef>>>>
      *const build = builder->finish();
  if (build == nullptr) {
    return false;
  }
  build->save(output);

  /* write some more meta-info */
  assert(maxAnalyzedPathsPerOutput > 0);
  output->writeVInt(maxAnalyzedPathsPerOutput);
  output->writeVInt(END_BYTE);
  output->writeVInt(PAYLOAD_SEP);
  return true;
}

int NRTSuggesterBuilder::maxNumArcsForDedupByte(int currentNumDedupBytes)
{
  int maxArcs = 1 + (2 * currentNumDedupBytes);
  if (currentNumDedupBytes > 5) {
    maxArcs *= currentNumDedupBytes;
  }
  return min(maxArcs, 255);
}

NRTSuggesterBuilder::Entry::Entry(shared_ptr<BytesRef> payload,
                                  int64_t weight)
    : payload(payload), weight(weight)
{
}

int NRTSuggesterBuilder::Entry::compareTo(shared_ptr<Entry> o)
{
  return Long::compare(weight, o->weight);
}
} // namespace org::apache::lucene::search::suggest::document