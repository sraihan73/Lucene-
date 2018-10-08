using namespace std;

#include "FieldPhraseList.h"

namespace org::apache::lucene::search::vectorhighlight
{
using QueryPhraseMap =
    org::apache::lucene::search::vectorhighlight::FieldQuery::QueryPhraseMap;
using TermInfo =
    org::apache::lucene::search::vectorhighlight::FieldTermStack::TermInfo;
using MergedIterator = org::apache::lucene::util::MergedIterator;

FieldPhraseList::FieldPhraseList(shared_ptr<FieldTermStack> fieldTermStack,
                                 shared_ptr<FieldQuery> fieldQuery)
    : FieldPhraseList(fieldTermStack, fieldQuery, numeric_limits<int>::max())
{
}

deque<std::shared_ptr<WeightedPhraseInfo>> FieldPhraseList::getPhraseList()
{
  return phraseList;
}

FieldPhraseList::FieldPhraseList(shared_ptr<FieldTermStack> fieldTermStack,
                                 shared_ptr<FieldQuery> fieldQuery,
                                 int phraseLimit)
{
  const wstring field = fieldTermStack->getFieldName();

  deque<std::shared_ptr<TermInfo>> phraseCandidate =
      deque<std::shared_ptr<TermInfo>>();
  shared_ptr<QueryPhraseMap> currMap = nullptr;
  shared_ptr<QueryPhraseMap> nextMap = nullptr;
  while (!fieldTermStack->isEmpty() && (phraseList.size() < phraseLimit)) {
    phraseCandidate.clear();

    shared_ptr<TermInfo> ti = nullptr;
    shared_ptr<TermInfo> first = nullptr;

    first = ti = fieldTermStack->pop();
    currMap = fieldQuery->getFieldTermMap(field, ti->getText());
    while (currMap == nullptr && ti->getNext() != first) {
      ti = ti->getNext();
      currMap = fieldQuery->getFieldTermMap(field, ti->getText());
    }

    // if not found, discard top TermInfo from stack, then try next element
    if (currMap == nullptr) {
      continue;
    }

    // if found, search the longest phrase
    phraseCandidate.push_back(ti);
    while (true) {
      first = ti = fieldTermStack->pop();
      nextMap.reset();
      if (ti != nullptr) {
        nextMap = currMap->getTermMap(ti->getText());
        while (nextMap == nullptr && ti->getNext() != first) {
          ti = ti->getNext();
          nextMap = currMap->getTermMap(ti->getText());
        }
      }
      if (ti == nullptr || nextMap == nullptr) {
        if (ti != nullptr) {
          fieldTermStack->push(ti);
        }
        if (currMap->isValidTermOrPhrase(phraseCandidate)) {
          addIfNoOverlap(make_shared<WeightedPhraseInfo>(
              phraseCandidate, currMap->getBoost(),
              currMap->getTermOrPhraseNumber()));
        } else {
          while (phraseCandidate.size() > 1) {
            fieldTermStack->push(phraseCandidate.pop_back());
            currMap = fieldQuery->searchPhrase(field, phraseCandidate);
            if (currMap != nullptr) {
              addIfNoOverlap(make_shared<WeightedPhraseInfo>(
                  phraseCandidate, currMap->getBoost(),
                  currMap->getTermOrPhraseNumber()));
              break;
            }
          }
        }
        break;
      } else {
        phraseCandidate.push_back(ti);
        currMap = nextMap;
      }
    }
  }
}

FieldPhraseList::FieldPhraseList(
    std::deque<std::shared_ptr<FieldPhraseList>> &toMerge)
{
  // Merge all overlapping WeightedPhraseInfos
  // Step 1.  Sort by startOffset, endOffset, and boost, in that order.
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings({ "rawtypes", "unchecked" })
  // java.util.Iterator< WeightedPhraseInfo >[] allInfos = new
  // java.util.Iterator[toMerge.length];
  std::deque<Iterator<std::shared_ptr<WeightedPhraseInfo>>> allInfos =
      std::deque<std::shared_ptr<Iterator>>(toMerge.size());
  int index = 0;
  for (auto fplToMerge : toMerge) {
    allInfos[index++] = fplToMerge->phraseList.begin();
  }
  shared_ptr<MergedIterator<std::shared_ptr<WeightedPhraseInfo>>> itr =
      make_shared<MergedIterator<std::shared_ptr<WeightedPhraseInfo>>>(
          false, allInfos);
  // Step 2.  Walk the sorted deque merging infos that overlap
  phraseList = deque<std::shared_ptr<WeightedPhraseInfo>>();
  if (!itr->hasNext()) {
    return;
  }
  deque<std::shared_ptr<WeightedPhraseInfo>> work =
      deque<std::shared_ptr<WeightedPhraseInfo>>();
  shared_ptr<WeightedPhraseInfo> first = itr->next();
  work.push_back(first);
  int workEndOffset = first->getEndOffset();
  while (itr->hasNext()) {
    shared_ptr<WeightedPhraseInfo> current = itr->next();
    if (current->getStartOffset() <= workEndOffset) {
      workEndOffset = max(workEndOffset, current->getEndOffset());
      work.push_back(current);
    } else {
      if (work.size() == 1) {
        phraseList.push_back(work[0]);
        work[0] = current;
      } else {
        phraseList.push_back(make_shared<WeightedPhraseInfo>(work));
        work.clear();
        work.push_back(current);
      }
      workEndOffset = current->getEndOffset();
    }
    itr++;
  }
  if (work.size() == 1) {
    phraseList.push_back(work[0]);
  } else {
    phraseList.push_back(make_shared<WeightedPhraseInfo>(work));
    work.clear();
  }
}

void FieldPhraseList::addIfNoOverlap(shared_ptr<WeightedPhraseInfo> wpi)
{
  for (auto existWpi : getPhraseList()) {
    if (existWpi->isOffsetOverlap(wpi)) {
      // WeightedPhraseInfo.addIfNoOverlap() dumps the second part of, for
      // example, hyphenated words (social-economics). The result is that all
      // informations in TermInfo are lost and not available for further
      // operations.
      existWpi->getTermsInfos().addAll(wpi->getTermsInfos());
      return;
    }
  }
  getPhraseList().push_back(wpi);
}

wstring FieldPhraseList::WeightedPhraseInfo::getText()
{
  shared_ptr<StringBuilder> text = make_shared<StringBuilder>();
  for (auto ti : termsInfos) {
    text->append(ti->getText());
  }
  return text->toString();
}

deque<std::shared_ptr<Toffs>>
FieldPhraseList::WeightedPhraseInfo::getTermsOffsets()
{
  return termsOffsets;
}

float FieldPhraseList::WeightedPhraseInfo::getBoost() { return boost; }

deque<std::shared_ptr<TermInfo>>
FieldPhraseList::WeightedPhraseInfo::getTermsInfos()
{
  return termsInfos;
}

FieldPhraseList::WeightedPhraseInfo::WeightedPhraseInfo(
    deque<std::shared_ptr<TermInfo>> &terms, float boost)
    : WeightedPhraseInfo(terms, boost, 0)
{
}

FieldPhraseList::WeightedPhraseInfo::WeightedPhraseInfo(
    deque<std::shared_ptr<TermInfo>> &terms, float boost, int seqnum)
{
  this->boost = boost;
  this->seqnum = seqnum;

  // We keep TermInfos for further operations
  termsInfos = deque<std::shared_ptr<TermInfo>>(terms);

  termsOffsets = deque<>(terms.size());
  shared_ptr<TermInfo> ti = terms.get(0);
  termsOffsets.push_back(
      make_shared<Toffs>(ti->getStartOffset(), ti->getEndOffset()));
  if (terms.size() == 1) {
    return;
  }
  int pos = ti->getPosition();
  for (int i = 1; i < terms.size(); i++) {
    ti = terms.get(i);
    if (ti->getPosition() - pos == 1) {
      shared_ptr<Toffs> to = termsOffsets[termsOffsets.size() - 1];
      to->setEndOffset(ti->getEndOffset());
    } else {
      termsOffsets.push_back(
          make_shared<Toffs>(ti->getStartOffset(), ti->getEndOffset()));
    }
    pos = ti->getPosition();
  }
}

FieldPhraseList::WeightedPhraseInfo::WeightedPhraseInfo(
    shared_ptr<deque<std::shared_ptr<WeightedPhraseInfo>>> toMerge)
{
  // Pretty much the same idea as merging FieldPhraseLists:
  // Step 1.  Sort by startOffset, endOffset
  //          While we are here merge the boosts and termInfos
  deque<std::shared_ptr<WeightedPhraseInfo>>::const_iterator toMergeItr =
      toMerge->begin();
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  if (!toMergeItr->hasNext()) {
    throw invalid_argument(
        L"toMerge must contain at least one WeightedPhraseInfo.");
  }
  // C++ TODO: Java iterators are only converted within the context of 'while'
  // and 'for' loops:
  shared_ptr<WeightedPhraseInfo> first = toMergeItr->next();
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings({ "rawtypes", "unchecked" })
  // java.util.Iterator< Toffs >[] allToffs = new
  // java.util.Iterator[toMerge.size()];
  std::deque<Iterator<std::shared_ptr<Toffs>>> allToffs =
      std::deque<std::shared_ptr<Iterator>>(toMerge->size());
  termsInfos = deque<std::shared_ptr<TermInfo>>();
  seqnum = first->seqnum;
  boost = first->boost;
  allToffs[0] = first->termsOffsets.begin();
  int index = 1;
  while (toMergeItr != toMerge->end()) {
    shared_ptr<WeightedPhraseInfo> info = *toMergeItr;
    boost += info->boost;
    termsInfos.insert(termsInfos.end(), info->termsInfos.begin(),
                      info->termsInfos.end());
    allToffs[index++] = info->termsOffsets.begin();
    toMergeItr++;
  }
  // Step 2.  Walk the sorted deque merging overlaps
  shared_ptr<MergedIterator<std::shared_ptr<Toffs>>> itr =
      make_shared<MergedIterator<std::shared_ptr<Toffs>>>(false, allToffs);
  termsOffsets = deque<>();
  if (!itr->hasNext()) {
    return;
  }
  shared_ptr<Toffs> work = itr->next();
  while (itr->hasNext()) {
    shared_ptr<Toffs> current = itr->next();
    if (current->startOffset <= work->endOffset) {
      work->endOffset = max(work->endOffset, current->endOffset);
    } else {
      termsOffsets.push_back(work);
      work = current;
    }
    itr++;
  }
  termsOffsets.push_back(work);
}

int FieldPhraseList::WeightedPhraseInfo::getStartOffset()
{
  return termsOffsets[0]->startOffset;
}

int FieldPhraseList::WeightedPhraseInfo::getEndOffset()
{
  return termsOffsets[termsOffsets.size() - 1]->endOffset;
}

bool FieldPhraseList::WeightedPhraseInfo::isOffsetOverlap(
    shared_ptr<WeightedPhraseInfo> other)
{
  int so = getStartOffset();
  int eo = getEndOffset();
  int oso = other->getStartOffset();
  int oeo = other->getEndOffset();
  if (so <= oso && oso < eo) {
    return true;
  }
  if (so < oeo && oeo <= eo) {
    return true;
  }
  if (oso <= so && so < oeo) {
    return true;
  }
  if (oso < eo && eo <= oeo) {
    return true;
  }
  return false;
}

wstring FieldPhraseList::WeightedPhraseInfo::toString()
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  sb->append(getText())->append(L'(')->append(boost)->append(L")(");
  for (auto to : termsOffsets) {
    sb->append(to);
  }
  sb->append(L')');
  return sb->toString();
}

int FieldPhraseList::WeightedPhraseInfo::getSeqnum() { return seqnum; }

int FieldPhraseList::WeightedPhraseInfo::compareTo(
    shared_ptr<WeightedPhraseInfo> other)
{
  int diff = getStartOffset() - other->getStartOffset();
  if (diff != 0) {
    return diff;
  }
  diff = getEndOffset() - other->getEndOffset();
  if (diff != 0) {
    return diff;
  }
  return static_cast<int>(Math::signum(getBoost() - other->getBoost()));
}

int FieldPhraseList::WeightedPhraseInfo::hashCode()
{
  constexpr int prime = 31;
  int result = 1;
  result = prime * result + getStartOffset();
  result = prime * result + getEndOffset();
  int64_t b = Double::doubleToLongBits(getBoost());
  result = prime * result +
           static_cast<int>(b ^ (static_cast<int64_t>(
                                    static_cast<uint64_t>(b) >> 32)));
  return result;
}

bool FieldPhraseList::WeightedPhraseInfo::equals(any obj)
{
  if (shared_from_this() == obj) {
    return true;
  }
  if (obj == nullptr) {
    return false;
  }
  if (getClass() != obj.type()) {
    return false;
  }
  shared_ptr<WeightedPhraseInfo> other =
      any_cast<std::shared_ptr<WeightedPhraseInfo>>(obj);
  if (getStartOffset() != other->getStartOffset()) {
    return false;
  }
  if (getEndOffset() != other->getEndOffset()) {
    return false;
  }
  if (getBoost() != other->getBoost()) {
    return false;
  }
  return true;
}

FieldPhraseList::WeightedPhraseInfo::Toffs::Toffs(int startOffset,
                                                  int endOffset)
{
  this->startOffset = startOffset;
  this->endOffset = endOffset;
}

void FieldPhraseList::WeightedPhraseInfo::Toffs::setEndOffset(int endOffset)
{
  this->endOffset = endOffset;
}

int FieldPhraseList::WeightedPhraseInfo::Toffs::getStartOffset()
{
  return startOffset;
}

int FieldPhraseList::WeightedPhraseInfo::Toffs::getEndOffset()
{
  return endOffset;
}

int FieldPhraseList::WeightedPhraseInfo::Toffs::compareTo(
    shared_ptr<Toffs> other)
{
  int diff = getStartOffset() - other->getStartOffset();
  if (diff != 0) {
    return diff;
  }
  return getEndOffset() - other->getEndOffset();
}

int FieldPhraseList::WeightedPhraseInfo::Toffs::hashCode()
{
  constexpr int prime = 31;
  int result = 1;
  result = prime * result + getStartOffset();
  result = prime * result + getEndOffset();
  return result;
}

bool FieldPhraseList::WeightedPhraseInfo::Toffs::equals(any obj)
{
  if (shared_from_this() == obj) {
    return true;
  }
  if (obj == nullptr) {
    return false;
  }
  if (getClass() != obj.type()) {
    return false;
  }
  shared_ptr<Toffs> other = any_cast<std::shared_ptr<Toffs>>(obj);
  if (getStartOffset() != other->getStartOffset()) {
    return false;
  }
  if (getEndOffset() != other->getEndOffset()) {
    return false;
  }
  return true;
}

wstring FieldPhraseList::WeightedPhraseInfo::Toffs::toString()
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  sb->append(L'(')
      ->append(startOffset)
      ->append(L',')
      ->append(endOffset)
      ->append(L')');
  return sb->toString();
}
} // namespace org::apache::lucene::search::vectorhighlight