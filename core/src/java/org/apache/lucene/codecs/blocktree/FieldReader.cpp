using namespace std;

#include "FieldReader.h"
#include "../../index/FieldInfo.h"
#include "../../index/IndexOptions.h"
#include "../../index/TermsEnum.h"
#include "../../store/IndexInput.h"
#include "../../util/Accountables.h"
#include "../../util/automaton/CompiledAutomaton.h"
#include "../../util/fst/ByteSequenceOutputs.h"
#include "BlockTreeTermsReader.h"
#include "IntersectTermsEnum.h"
#include "SegmentTermsEnum.h"
#include "Stats.h"

namespace org::apache::lucene::codecs::blocktree
{
using FieldInfo = org::apache::lucene::index::FieldInfo;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using IndexInput = org::apache::lucene::store::IndexInput;
using Accountable = org::apache::lucene::util::Accountable;
using Accountables = org::apache::lucene::util::Accountables;
using BytesRef = org::apache::lucene::util::BytesRef;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
using CompiledAutomaton =
    org::apache::lucene::util::automaton::CompiledAutomaton;
using ByteSequenceOutputs = org::apache::lucene::util::fst::ByteSequenceOutputs;
using FST = org::apache::lucene::util::fst::FST;

FieldReader::FieldReader(shared_ptr<BlockTreeTermsReader> parent,
                         shared_ptr<FieldInfo> fieldInfo, int64_t numTerms,
                         shared_ptr<BytesRef> rootCode,
                         int64_t sumTotalTermFreq, int64_t sumDocFreq,
                         int docCount, int64_t indexStartFP, int longsSize,
                         shared_ptr<IndexInput> indexIn,
                         shared_ptr<BytesRef> minTerm,
                         shared_ptr<BytesRef> maxTerm) 
    : numTerms(numTerms), fieldInfo(fieldInfo),
      sumTotalTermFreq(sumTotalTermFreq), sumDocFreq(sumDocFreq),
      docCount(docCount), indexStartFP(indexStartFP),
      rootBlockFP(static_cast<int64_t>(
          static_cast<uint64_t>(
              (make_shared<ByteArrayDataInput>(
                   rootCode->bytes, rootCode->offset, rootCode->length))
                  ->readVLong()) >>
          BlockTreeTermsReader::OUTPUT_FLAGS_NUM_BITS)),
      rootCode(rootCode), minTerm(minTerm), maxTerm(maxTerm),
      longsSize(longsSize), parent(parent)
{
  assert(numTerms > 0);
  // DEBUG = BlockTreeTermsReader.DEBUG && fieldInfo.name.equals("id");
  // if (DEBUG) {
  //   System.out.println("BTTR: seg=" + segment + " field=" + fieldInfo.name +
  //   " rootBlockCode=" + rootCode + " divisor=" + indexDivisor);
  // }

  if (indexIn != nullptr) {
    shared_ptr<IndexInput> *const clone = indexIn->clone();
    // System.out.println("start=" + indexStartFP + " field=" + fieldInfo.name);
    clone->seek(indexStartFP);
    index =
        make_shared<FST<BytesRef>>(clone, ByteSequenceOutputs::getSingleton());

    /*
      if (false) {
      final std::wstring dotFileName = segment + "_" + fieldInfo.name + ".dot";
      Writer w = new OutputStreamWriter(new FileOutputStream(dotFileName));
      Util.toDot(index, w, false, false);
      System.out.println("FST INDEX: SAVED to " + dotFileName);
      w.close();
      }
    */
  } else {
    index.reset();
  }
}

shared_ptr<BytesRef> FieldReader::getMin() 
{
  if (minTerm == nullptr) {
    // Older index that didn't store min/maxTerm
    return Terms::getMin();
  } else {
    return minTerm;
  }
}

shared_ptr<BytesRef> FieldReader::getMax() 
{
  if (maxTerm == nullptr) {
    // Older index that didn't store min/maxTerm
    return Terms::getMax();
  } else {
    return maxTerm;
  }
}

shared_ptr<Stats> FieldReader::getStats() 
{
  // TODO: add auto-prefix terms into stats
  return (make_shared<SegmentTermsEnum>(shared_from_this()))
      ->computeBlockStats();
}

bool FieldReader::hasFreqs()
{
  return fieldInfo->getIndexOptions().compareTo(IndexOptions::DOCS_AND_FREQS) >=
         0;
}

bool FieldReader::hasOffsets()
{
  return fieldInfo->getIndexOptions().compareTo(
             IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS) >= 0;
}

bool FieldReader::hasPositions()
{
  return fieldInfo->getIndexOptions().compareTo(
             IndexOptions::DOCS_AND_FREQS_AND_POSITIONS) >= 0;
}

bool FieldReader::hasPayloads() { return fieldInfo->hasPayloads(); }

shared_ptr<TermsEnum> FieldReader::iterator() 
{
  return make_shared<SegmentTermsEnum>(shared_from_this());
}

int64_t FieldReader::size() { return numTerms; }

int64_t FieldReader::getSumTotalTermFreq() { return sumTotalTermFreq; }

int64_t FieldReader::getSumDocFreq() { return sumDocFreq; }

int FieldReader::getDocCount() { return docCount; }

shared_ptr<TermsEnum>
FieldReader::intersect(shared_ptr<CompiledAutomaton> compiled,
                       shared_ptr<BytesRef> startTerm) 
{
  // if (DEBUG) System.out.println("  FieldReader.intersect startTerm=" +
  // BlockTreeTermsWriter.brToString(startTerm));
  // System.out.println("intersect: " + compiled.type + " a=" +
  // compiled.automaton);
  // TODO: we could push "it's a range" or "it's a prefix" down into
  // IntersectTermsEnum? can we optimize knowing that...?
  if (compiled->type != CompiledAutomaton::AUTOMATON_TYPE::NORMAL) {
    throw invalid_argument(
        L"please use CompiledAutomaton.getTermsEnum instead");
  }
  return make_shared<IntersectTermsEnum>(
      shared_from_this(), compiled->automaton, compiled->runAutomaton,
      compiled->commonSuffixRef, startTerm, compiled->sinkState);
}

int64_t FieldReader::ramBytesUsed()
{
  return BASE_RAM_BYTES_USED + ((index != nullptr) ? index->ramBytesUsed() : 0);
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
FieldReader::getChildResources()
{
  if (index == nullptr) {
    return Collections::emptyList();
  } else {
    return Collections::singleton(
        Accountables::namedAccountable(L"term index", index));
  }
}

wstring FieldReader::toString()
{
  return L"BlockTreeTerms(seg=" + parent->segment + L" terms=" +
         to_wstring(numTerms) + L",postings=" + to_wstring(sumDocFreq) +
         L",positions=" + to_wstring(sumTotalTermFreq) + L",docs=" +
         to_wstring(docCount) + L")";
}
} // namespace org::apache::lucene::codecs::blocktree