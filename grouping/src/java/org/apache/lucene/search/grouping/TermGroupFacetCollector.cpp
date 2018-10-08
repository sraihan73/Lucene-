using namespace std;

#include "TermGroupFacetCollector.h"

namespace org::apache::lucene::search::grouping
{
using DocValues = org::apache::lucene::index::DocValues;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using SentinelIntSet = org::apache::lucene::util::SentinelIntSet;
using UnicodeUtil = org::apache::lucene::util::UnicodeUtil;

shared_ptr<TermGroupFacetCollector>
TermGroupFacetCollector::createTermGroupFacetCollector(
    const wstring &groupField, const wstring &facetField,
    bool facetFieldMultivalued, shared_ptr<BytesRef> facetPrefix,
    int initialSize)
{
  if (facetFieldMultivalued) {
    return make_shared<MV>(groupField, facetField, facetPrefix, initialSize);
  } else {
    return make_shared<SV>(groupField, facetField, facetPrefix, initialSize);
  }
}

TermGroupFacetCollector::TermGroupFacetCollector(
    const wstring &groupField, const wstring &facetField,
    shared_ptr<BytesRef> facetPrefix, int initialSize)
    : GroupFacetCollector(groupField, facetField, facetPrefix),
      groupedFacetHits(deque<>(initialSize)),
      segmentGroupedFacetHits(
          make_shared<SentinelIntSet>(initialSize, numeric_limits<int>::min()))
{
}

TermGroupFacetCollector::SV::SV(const wstring &groupField,
                                const wstring &facetField,
                                shared_ptr<BytesRef> facetPrefix,
                                int initialSize)
    : TermGroupFacetCollector(groupField, facetField, facetPrefix, initialSize)
{
}

void TermGroupFacetCollector::SV::collect(int doc) 
{
  if (doc > facetFieldTermsIndex->docID()) {
    facetFieldTermsIndex->advance(doc);
  }

  int facetOrd;
  if (doc == facetFieldTermsIndex->docID()) {
    facetOrd = facetFieldTermsIndex->ordValue();
  } else {
    facetOrd = -1;
  }

  if (facetOrd < startFacetOrd || facetOrd >= endFacetOrd) {
    return;
  }

  if (doc > groupFieldTermsIndex->docID()) {
    groupFieldTermsIndex->advance(doc);
  }

  int groupOrd;
  if (doc == groupFieldTermsIndex->docID()) {
    groupOrd = groupFieldTermsIndex->ordValue();
  } else {
    groupOrd = -1;
  }
  int segmentGroupedFacetsIndex =
      groupOrd * (facetFieldTermsIndex->getValueCount() + 1) + facetOrd;
  if (segmentGroupedFacetHits->exists(segmentGroupedFacetsIndex)) {
    return;
  }

  segmentTotalCount++;
  segmentFacetCounts[facetOrd + 1]++;

  segmentGroupedFacetHits->put(segmentGroupedFacetsIndex);

  shared_ptr<BytesRef> groupKey;
  if (groupOrd == -1) {
    groupKey.reset();
  } else {
    groupKey = BytesRef::deepCopyOf(groupFieldTermsIndex->lookupOrd(groupOrd));
  }

  shared_ptr<BytesRef> facetKey;
  if (facetOrd == -1) {
    facetKey.reset();
  } else {
    facetKey = BytesRef::deepCopyOf(facetFieldTermsIndex->lookupOrd(facetOrd));
  }

  groupedFacetHits.push_back(make_shared<GroupedFacetHit>(groupKey, facetKey));
}

void TermGroupFacetCollector::SV::doSetNextReader(
    shared_ptr<LeafReaderContext> context) 
{
  if (segmentFacetCounts.size() > 0) {
    segmentResults.push_back(createSegmentResult());
  }

  groupFieldTermsIndex = DocValues::getSorted(context->reader(), groupField);
  facetFieldTermsIndex = DocValues::getSorted(context->reader(), facetField);

  // 1+ to allow for the -1 "not set":
  segmentFacetCounts =
      std::deque<int>(facetFieldTermsIndex->getValueCount() + 1);
  segmentTotalCount = 0;

  segmentGroupedFacetHits->clear();
  for (auto groupedFacetHit : groupedFacetHits) {
    int facetOrd =
        groupedFacetHit->facetValue == nullptr
            ? -1
            : facetFieldTermsIndex->lookupTerm(groupedFacetHit->facetValue);
    if (groupedFacetHit->facetValue != nullptr && facetOrd < 0) {
      continue;
    }

    int groupOrd =
        groupedFacetHit->groupValue == nullptr
            ? -1
            : groupFieldTermsIndex->lookupTerm(groupedFacetHit->groupValue);
    if (groupedFacetHit->groupValue != nullptr && groupOrd < 0) {
      continue;
    }

    int segmentGroupedFacetsIndex =
        groupOrd * (facetFieldTermsIndex->getValueCount() + 1) + facetOrd;
    segmentGroupedFacetHits->put(segmentGroupedFacetsIndex);
  }

  if (facetPrefix != nullptr) {
    startFacetOrd = facetFieldTermsIndex->lookupTerm(facetPrefix);
    if (startFacetOrd < 0) {
      // Points to the ord one higher than facetPrefix
      startFacetOrd = -startFacetOrd - 1;
    }
    shared_ptr<BytesRefBuilder> facetEndPrefix = make_shared<BytesRefBuilder>();
    facetEndPrefix->append(facetPrefix);
    facetEndPrefix->append(UnicodeUtil::BIG_TERM);
    endFacetOrd = facetFieldTermsIndex->lookupTerm(facetEndPrefix->get());
    assert(endFacetOrd < 0);
    endFacetOrd =
        -endFacetOrd - 1; // Points to the ord one higher than facetEndPrefix
  } else {
    startFacetOrd = -1;
    endFacetOrd = facetFieldTermsIndex->getValueCount();
  }
}

shared_ptr<SegmentResult>
TermGroupFacetCollector::SV::createSegmentResult() 
{
  return make_shared<SegmentResult>(segmentFacetCounts, segmentTotalCount,
                                    facetFieldTermsIndex->termsEnum(),
                                    startFacetOrd, endFacetOrd);
}

TermGroupFacetCollector::SV::SegmentResult::SegmentResult(
    std::deque<int> &counts, int total, shared_ptr<TermsEnum> tenum,
    int startFacetOrd, int endFacetOrd) 
    : GroupFacetCollector::SegmentResult(counts, total - counts[0], counts[0],
                                         endFacetOrd + 1),
      tenum(tenum)
{
  this->mergePos = startFacetOrd == -1 ? 1 : startFacetOrd + 1;
  if (mergePos < maxTermPos) {
    assert(tenum != nullptr);
    tenum->seekExact(startFacetOrd == -1 ? 0 : startFacetOrd);
    mergeTerm = tenum->term();
  }
}

void TermGroupFacetCollector::SV::SegmentResult::nextTerm() 
{
  mergeTerm = tenum->next();
}

TermGroupFacetCollector::MV::MV(const wstring &groupField,
                                const wstring &facetField,
                                shared_ptr<BytesRef> facetPrefix,
                                int initialSize)
    : TermGroupFacetCollector(groupField, facetField, facetPrefix, initialSize)
{
}

void TermGroupFacetCollector::MV::collect(int doc) 
{
  if (doc > groupFieldTermsIndex->docID()) {
    groupFieldTermsIndex->advance(doc);
  }

  int groupOrd;
  if (doc == groupFieldTermsIndex->docID()) {
    groupOrd = groupFieldTermsIndex->ordValue();
  } else {
    groupOrd = -1;
  }

  if (facetFieldNumTerms == 0) {
    int segmentGroupedFacetsIndex = groupOrd * (facetFieldNumTerms + 1);
    if (facetPrefix != nullptr ||
        segmentGroupedFacetHits->exists(segmentGroupedFacetsIndex)) {
      return;
    }

    segmentTotalCount++;
    segmentFacetCounts[facetFieldNumTerms]++;

    segmentGroupedFacetHits->put(segmentGroupedFacetsIndex);
    shared_ptr<BytesRef> groupKey;
    if (groupOrd == -1) {
      groupKey.reset();
    } else {
      groupKey =
          BytesRef::deepCopyOf(groupFieldTermsIndex->lookupOrd(groupOrd));
    }
    groupedFacetHits.push_back(make_shared<GroupedFacetHit>(groupKey, nullptr));
    return;
  }

  if (doc > facetFieldDocTermOrds->docID()) {
    facetFieldDocTermOrds->advance(doc);
  }
  bool empty = true;
  if (doc == facetFieldDocTermOrds->docID()) {
    int64_t ord;
    while ((ord = facetFieldDocTermOrds->nextOrd()) !=
           SortedSetDocValues::NO_MORE_ORDS) {
      process(groupOrd, static_cast<int>(ord));
      empty = false;
    }
  }

  if (empty) {
    process(groupOrd, facetFieldNumTerms); // this facet ord is reserved for
                                           // docs not containing facet field.
  }
}

void TermGroupFacetCollector::MV::process(int groupOrd,
                                          int facetOrd) 
{
  if (facetOrd < startFacetOrd || facetOrd >= endFacetOrd) {
    return;
  }

  int segmentGroupedFacetsIndex =
      groupOrd * (facetFieldNumTerms + 1) + facetOrd;
  if (segmentGroupedFacetHits->exists(segmentGroupedFacetsIndex)) {
    return;
  }

  segmentTotalCount++;
  segmentFacetCounts[facetOrd]++;

  segmentGroupedFacetHits->put(segmentGroupedFacetsIndex);

  shared_ptr<BytesRef> groupKey;
  if (groupOrd == -1) {
    groupKey.reset();
  } else {
    groupKey = BytesRef::deepCopyOf(groupFieldTermsIndex->lookupOrd(groupOrd));
  }

  shared_ptr<BytesRef> *const facetValue;
  if (facetOrd == facetFieldNumTerms) {
    facetValue.reset();
  } else {
    facetValue =
        BytesRef::deepCopyOf(facetFieldDocTermOrds->lookupOrd(facetOrd));
  }
  groupedFacetHits.push_back(
      make_shared<GroupedFacetHit>(groupKey, facetValue));
}

void TermGroupFacetCollector::MV::doSetNextReader(
    shared_ptr<LeafReaderContext> context) 
{
  if (segmentFacetCounts.size() > 0) {
    segmentResults.push_back(createSegmentResult());
  }

  groupFieldTermsIndex = DocValues::getSorted(context->reader(), groupField);
  facetFieldDocTermOrds =
      DocValues::getSortedSet(context->reader(), facetField);
  facetFieldNumTerms = static_cast<int>(facetFieldDocTermOrds->getValueCount());
  if (facetFieldNumTerms == 0) {
    facetOrdTermsEnum.reset();
  } else {
    facetOrdTermsEnum = facetFieldDocTermOrds->termsEnum();
  }
  // [facetFieldNumTerms() + 1] for all possible facet values and docs not
  // containing facet field
  segmentFacetCounts = std::deque<int>(facetFieldNumTerms + 1);
  segmentTotalCount = 0;

  segmentGroupedFacetHits->clear();
  for (auto groupedFacetHit : groupedFacetHits) {
    int groupOrd =
        groupedFacetHit->groupValue == nullptr
            ? -1
            : groupFieldTermsIndex->lookupTerm(groupedFacetHit->groupValue);
    if (groupedFacetHit->groupValue != nullptr && groupOrd < 0) {
      continue;
    }

    int facetOrd;
    if (groupedFacetHit->facetValue != nullptr) {
      if (facetOrdTermsEnum == nullptr ||
          !facetOrdTermsEnum->seekExact(groupedFacetHit->facetValue)) {
        continue;
      }
      facetOrd = static_cast<int>(facetOrdTermsEnum->ord());
    } else {
      facetOrd = facetFieldNumTerms;
    }

    // (facetFieldDocTermOrds.numTerms() + 1) for all possible facet values and
    // docs not containing facet field
    int segmentGroupedFacetsIndex =
        groupOrd * (facetFieldNumTerms + 1) + facetOrd;
    segmentGroupedFacetHits->put(segmentGroupedFacetsIndex);
  }

  if (facetPrefix != nullptr) {
    TermsEnum::SeekStatus seekStatus;
    if (facetOrdTermsEnum != nullptr) {
      seekStatus = facetOrdTermsEnum->seekCeil(facetPrefix);
    } else {
      seekStatus = TermsEnum::SeekStatus::END;
    }

    if (seekStatus != TermsEnum::SeekStatus::END) {
      startFacetOrd = static_cast<int>(facetOrdTermsEnum->ord());
    } else {
      startFacetOrd = 0;
      endFacetOrd = 0;
      return;
    }

    shared_ptr<BytesRefBuilder> facetEndPrefix = make_shared<BytesRefBuilder>();
    facetEndPrefix->append(facetPrefix);
    facetEndPrefix->append(UnicodeUtil::BIG_TERM);
    seekStatus = facetOrdTermsEnum->seekCeil(facetEndPrefix->get());
    if (seekStatus != TermsEnum::SeekStatus::END) {
      endFacetOrd = static_cast<int>(facetOrdTermsEnum->ord());
    } else {
      endFacetOrd = facetFieldNumTerms; // Don't include null...
    }
  } else {
    startFacetOrd = 0;
    endFacetOrd = facetFieldNumTerms + 1;
  }
}

shared_ptr<SegmentResult>
TermGroupFacetCollector::MV::createSegmentResult() 
{
  return make_shared<SegmentResult>(segmentFacetCounts, segmentTotalCount,
                                    facetFieldNumTerms, facetOrdTermsEnum,
                                    startFacetOrd, endFacetOrd);
}

TermGroupFacetCollector::MV::SegmentResult::SegmentResult(
    std::deque<int> &counts, int total, int missingCountIndex,
    shared_ptr<TermsEnum> tenum, int startFacetOrd,
    int endFacetOrd) 
    : GroupFacetCollector::SegmentResult(
          counts, total - counts[missingCountIndex], counts[missingCountIndex],
          endFacetOrd == missingCountIndex + 1 ? missingCountIndex
                                               : endFacetOrd),
      tenum(tenum)
{
  this->mergePos = startFacetOrd;
  if (tenum != nullptr) {
    tenum->seekExact(mergePos);
    mergeTerm = tenum->term();
  }
}

void TermGroupFacetCollector::MV::SegmentResult::nextTerm() 
{
  mergeTerm = tenum->next();
}

TermGroupFacetCollector::GroupedFacetHit::GroupedFacetHit(
    shared_ptr<BytesRef> groupValue, shared_ptr<BytesRef> facetValue)
    : groupValue(groupValue), facetValue(facetValue)
{
}
} // namespace org::apache::lucene::search::grouping