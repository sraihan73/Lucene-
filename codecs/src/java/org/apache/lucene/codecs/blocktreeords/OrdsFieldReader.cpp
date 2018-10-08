using namespace std;

#include "OrdsFieldReader.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/FieldInfo.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexOptions.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/TermsEnum.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IndexInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/Accountables.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/automaton/CompiledAutomaton.h"
#include "FSTOrdsOutputs.h"
#include "OrdsBlockTreeTermsReader.h"
#include "OrdsBlockTreeTermsWriter.h"
#include "OrdsIntersectTermsEnum.h"
#include "OrdsSegmentTermsEnum.h"

namespace org::apache::lucene::codecs::blocktreeords
{
using Output =
    org::apache::lucene::codecs::blocktreeords::FSTOrdsOutputs::Output;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using IndexInput = org::apache::lucene::store::IndexInput;
using Accountable = org::apache::lucene::util::Accountable;
using Accountables = org::apache::lucene::util::Accountables;
using BytesRef = org::apache::lucene::util::BytesRef;
using CompiledAutomaton =
    org::apache::lucene::util::automaton::CompiledAutomaton;
using FST = org::apache::lucene::util::fst::FST;

OrdsFieldReader::OrdsFieldReader(
    shared_ptr<OrdsBlockTreeTermsReader> parent,
    shared_ptr<FieldInfo> fieldInfo, int64_t numTerms,
    shared_ptr<Output> rootCode, int64_t sumTotalTermFreq,
    int64_t sumDocFreq, int docCount, int64_t indexStartFP, int longsSize,
    shared_ptr<IndexInput> indexIn, shared_ptr<BytesRef> minTerm,
    shared_ptr<BytesRef> maxTerm) 
    : numTerms(numTerms), fieldInfo(fieldInfo),
      sumTotalTermFreq(sumTotalTermFreq), sumDocFreq(sumDocFreq),
      docCount(docCount), indexStartFP(indexStartFP),
      rootBlockFP(static_cast<int64_t>(
          static_cast<uint64_t>(
              (make_shared<ByteArrayDataInput>(rootCode->bytes->bytes,
                                               rootCode->bytes->offset,
                                               rootCode->bytes->length))
                  ->readVLong()) >>
          OrdsBlockTreeTermsWriter::OUTPUT_FLAGS_NUM_BITS)),
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
        make_shared<FST<Output>>(clone, OrdsBlockTreeTermsWriter::FST_OUTPUTS);

    /*
    if (true) {
      final std::wstring dotFileName = "/tmp/" + parent.segment + "_" + fieldInfo.name
    + ".dot"; Writer w = new OutputStreamWriter(new
    FileOutputStream(dotFileName)); Util.toDot(index, w, false, false);
      System.out.println("FST INDEX: SAVED to " + dotFileName);
      w.close();
    }
    */
  } else {
    index.reset();
  }
}

shared_ptr<BytesRef> OrdsFieldReader::getMin() 
{
  if (minTerm == nullptr) {
    // Older index that didn't store min/maxTerm
    return Terms::getMin();
  } else {
    return minTerm;
  }
}

shared_ptr<BytesRef> OrdsFieldReader::getMax() 
{
  if (maxTerm == nullptr) {
    // Older index that didn't store min/maxTerm
    return Terms::getMax();
  } else {
    return maxTerm;
  }
}

bool OrdsFieldReader::hasFreqs()
{
  return fieldInfo->getIndexOptions().compareTo(IndexOptions::DOCS_AND_FREQS) >=
         0;
}

bool OrdsFieldReader::hasOffsets()
{
  return fieldInfo->getIndexOptions().compareTo(
             IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS) >= 0;
}

bool OrdsFieldReader::hasPositions()
{
  return fieldInfo->getIndexOptions().compareTo(
             IndexOptions::DOCS_AND_FREQS_AND_POSITIONS) >= 0;
}

bool OrdsFieldReader::hasPayloads() { return fieldInfo->hasPayloads(); }

shared_ptr<TermsEnum> OrdsFieldReader::iterator() 
{
  return make_shared<OrdsSegmentTermsEnum>(shared_from_this());
}

int64_t OrdsFieldReader::size() { return numTerms; }

int64_t OrdsFieldReader::getSumTotalTermFreq() { return sumTotalTermFreq; }

int64_t OrdsFieldReader::getSumDocFreq() { return sumDocFreq; }

int OrdsFieldReader::getDocCount() { return docCount; }

shared_ptr<TermsEnum>
OrdsFieldReader::intersect(shared_ptr<CompiledAutomaton> compiled,
                           shared_ptr<BytesRef> startTerm) 
{
  if (compiled->type != CompiledAutomaton::AUTOMATON_TYPE::NORMAL) {
    throw invalid_argument(
        L"please use CompiledAutomaton.getTermsEnum instead");
  }
  return make_shared<OrdsIntersectTermsEnum>(shared_from_this(), compiled,
                                             startTerm);
}

int64_t OrdsFieldReader::ramBytesUsed()
{
  return ((index != nullptr) ? index->ramBytesUsed() : 0);
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
OrdsFieldReader::getChildResources()
{
  if (index == nullptr) {
    return Collections::emptyList();
  } else {
    return Collections::singleton(
        Accountables::namedAccountable(L"term index", index));
  }
}

wstring OrdsFieldReader::toString()
{
  return L"OrdsBlockTreeTerms(terms=" + to_wstring(numTerms) + L",postings=" +
         to_wstring(sumDocFreq) + L",positions=" +
         to_wstring(sumTotalTermFreq) + L",docs=" + to_wstring(docCount) + L")";
}
} // namespace org::apache::lucene::codecs::blocktreeords