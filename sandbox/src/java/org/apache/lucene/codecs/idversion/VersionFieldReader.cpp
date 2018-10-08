using namespace std;

#include "VersionFieldReader.h"

namespace org::apache::lucene::codecs::idversion
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
using FST = org::apache::lucene::util::fst::FST;
using org::apache::lucene::util::fst::PairOutputs::Pair;

VersionFieldReader::VersionFieldReader(
    shared_ptr<VersionBlockTreeTermsReader> parent,
    shared_ptr<FieldInfo> fieldInfo, int64_t numTerms,
    shared_ptr<Pair<std::shared_ptr<BytesRef>, int64_t>> rootCode,
    int64_t sumTotalTermFreq, int64_t sumDocFreq, int docCount,
    int64_t indexStartFP, int longsSize, shared_ptr<IndexInput> indexIn,
    shared_ptr<BytesRef> minTerm,
    shared_ptr<BytesRef> maxTerm) 
    : numTerms(numTerms), fieldInfo(fieldInfo),
      sumTotalTermFreq(sumTotalTermFreq), sumDocFreq(sumDocFreq),
      docCount(docCount), indexStartFP(indexStartFP),
      rootBlockFP(static_cast<int64_t>(
          static_cast<uint64_t>(
              (make_shared<ByteArrayDataInput>(rootCode->output1->bytes,
                                               rootCode->output1->offset,
                                               rootCode->output1->length))
                  ->readVLong()) >>
          VersionBlockTreeTermsWriter::OUTPUT_FLAGS_NUM_BITS)),
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
    index = make_shared<FST<Pair<BytesRef, int64_t>>>(
        clone, VersionBlockTreeTermsWriter::FST_OUTPUTS);

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

shared_ptr<BytesRef> VersionFieldReader::getMin() 
{
  if (minTerm == nullptr) {
    // Older index that didn't store min/maxTerm
    return Terms::getMin();
  } else {
    return minTerm;
  }
}

shared_ptr<BytesRef> VersionFieldReader::getMax() 
{
  if (maxTerm == nullptr) {
    // Older index that didn't store min/maxTerm
    return Terms::getMax();
  } else {
    return maxTerm;
  }
}

bool VersionFieldReader::hasFreqs()
{
  return fieldInfo->getIndexOptions().compareTo(IndexOptions::DOCS_AND_FREQS) >=
         0;
}

bool VersionFieldReader::hasOffsets()
{
  return fieldInfo->getIndexOptions().compareTo(
             IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS) >= 0;
}

bool VersionFieldReader::hasPositions()
{
  return fieldInfo->getIndexOptions().compareTo(
             IndexOptions::DOCS_AND_FREQS_AND_POSITIONS) >= 0;
}

bool VersionFieldReader::hasPayloads() { return fieldInfo->hasPayloads(); }

shared_ptr<TermsEnum> VersionFieldReader::iterator() 
{
  return make_shared<IDVersionSegmentTermsEnum>(shared_from_this());
}

int64_t VersionFieldReader::size() { return numTerms; }

int64_t VersionFieldReader::getSumTotalTermFreq() { return sumTotalTermFreq; }

int64_t VersionFieldReader::getSumDocFreq() { return sumDocFreq; }

int VersionFieldReader::getDocCount() { return docCount; }

int64_t VersionFieldReader::ramBytesUsed()
{
  return ((index != nullptr) ? index->ramBytesUsed() : 0);
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
VersionFieldReader::getChildResources()
{
  if (index == nullptr) {
    return Collections::emptyList();
  } else {
    return Collections::singletonList(
        Accountables::namedAccountable(L"term index", index));
  }
}

wstring VersionFieldReader::toString()
{
  return L"IDVersionTerms(terms=" + to_wstring(numTerms) + L",postings=" +
         to_wstring(sumDocFreq) + L",positions=" +
         to_wstring(sumTotalTermFreq) + L",docs=" + to_wstring(docCount) + L")";
}
} // namespace org::apache::lucene::codecs::idversion