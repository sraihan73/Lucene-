using namespace std;

#include "TermGroupSelector.h"

namespace org::apache::lucene::search::grouping
{
using DocValues = org::apache::lucene::index::DocValues;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefHash = org::apache::lucene::util::BytesRefHash;

TermGroupSelector::TermGroupSelector(const wstring &field) : field(field) {}

void TermGroupSelector::setNextReader(
    shared_ptr<LeafReaderContext> readerContext) 
{
  this->docValues = DocValues::getSorted(readerContext->reader(), field);
  this->ordsToGroupIds.clear();
  shared_ptr<BytesRef> scratch = make_shared<BytesRef>();
  for (int i = 0; i < values->size(); i++) {
    values->get(i, scratch);
    int ord = this->docValues->lookupTerm(scratch);
    if (ord >= 0) {
      ordsToGroupIds.emplace(ord, i);
    }
  }
}

State TermGroupSelector::advanceTo(int doc) 
{
  if (this->docValues->advanceExact(doc) == false) {
    groupId = -1;
    return includeEmpty ? State::ACCEPT : State::SKIP;
  }
  int ord = docValues->ordValue();
  if (ordsToGroupIds.find(ord) != ordsToGroupIds.end()) {
    groupId = ordsToGroupIds[ord];
    return State::ACCEPT;
  }
  if (secondPass) {
    return State::SKIP;
  }
  groupId = values->add(docValues->binaryValue());
  ordsToGroupIds.emplace(ord, groupId);
  return State::ACCEPT;
}

shared_ptr<BytesRef> TermGroupSelector::currentValue()
{
  if (groupId == -1) {
    return nullptr;
  }
  values->get(groupId, scratch);
  return scratch;
}

shared_ptr<BytesRef> TermGroupSelector::copyValue()
{
  if (groupId == -1) {
    return nullptr;
  }
  return BytesRef::deepCopyOf(currentValue());
}

void TermGroupSelector::setGroups(
    shared_ptr<deque<SearchGroup<std::shared_ptr<BytesRef>>>> searchGroups)
{
  this->values->clear();
  this->values->reinit();
  for (auto sg : searchGroups) {
    if (sg->groupValue == nullptr) {
      includeEmpty = true;
    } else {
      this->values->add(sg->groupValue);
    }
  }
  this->secondPass = true;
}
} // namespace org::apache::lucene::search::grouping