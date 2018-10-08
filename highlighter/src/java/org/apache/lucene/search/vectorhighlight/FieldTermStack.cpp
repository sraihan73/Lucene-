using namespace std;

#include "FieldTermStack.h"

namespace org::apache::lucene::search::vectorhighlight
{
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Fields = org::apache::lucene::index::Fields;
using IndexReader = org::apache::lucene::index::IndexReader;
using Term = org::apache::lucene::index::Term;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using BytesRef = org::apache::lucene::util::BytesRef;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;

FieldTermStack::FieldTermStack(
    shared_ptr<IndexReader> reader, int docId, const wstring &fieldName,
    shared_ptr<FieldQuery> fieldQuery) 
    : fieldName(fieldName)
{

  shared_ptr<Set<wstring>> termSet = fieldQuery->getTermSet(fieldName);
  // just return to make null snippet if un-matched fieldName specified when
  // fieldMatch == true
  if (termSet == nullptr) {
    return;
  }

  shared_ptr<Fields> *const vectors = reader->getTermVectors(docId);
  if (vectors->empty()) {
    // null snippet
    return;
  }

  shared_ptr<Terms> *const deque = vectors->terms(fieldName);
  if (deque == nullptr || deque->hasPositions() == false) {
    // null snippet
    return;
  }

  shared_ptr<CharsRefBuilder> *const spare = make_shared<CharsRefBuilder>();
  shared_ptr<TermsEnum> *const termsEnum = deque->begin();
  shared_ptr<PostingsEnum> dpEnum = nullptr;
  shared_ptr<BytesRef> text;

  int numDocs = reader->maxDoc();

  while ((text = termsEnum->next()) != nullptr) {
    spare->copyUTF8Bytes(text);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    const wstring term = spare->toString();
    if (!termSet->contains(term)) {
      continue;
    }
    dpEnum = termsEnum->postings(dpEnum, PostingsEnum::POSITIONS);
    dpEnum->nextDoc();

    // For weight look here:
    // http://lucene.apache.org/core/3_6_0/api/core/org/apache/lucene/search/DefaultSimilarity.html
    constexpr float weight = static_cast<float>(
        log(numDocs /
            static_cast<double>(
                reader->docFreq(make_shared<Term>(fieldName, text)) + 1)) +
        1.0);

    constexpr int freq = dpEnum->freq();

    for (int i = 0; i < freq; i++) {
      int pos = dpEnum->nextPosition();
      if (dpEnum->startOffset() < 0) {
        return; // no offsets, null snippet
      }
      termList.push_back(make_shared<TermInfo>(
          term, dpEnum->startOffset(), dpEnum->endOffset(), pos, weight));
    }
  }

  // sort by position
  sort(termList.begin(), termList.end());

  // now look for dups at the same position, linking them together
  int currentPos = -1;
  shared_ptr<TermInfo> previous = nullptr;
  shared_ptr<TermInfo> first = nullptr;
  deque<std::shared_ptr<TermInfo>>::const_iterator iterator = termList.begin();
  while (iterator != termList.end()) {
    shared_ptr<TermInfo> current = *iterator;
    if (current->position == currentPos) {
      assert(previous != nullptr);
      previous->setNext(current);
      previous = current;
      (*iterator)->remove();
    } else {
      if (previous != nullptr) {
        previous->setNext(first);
      }
      previous = first = current;
      currentPos = current->position;
    }
    iterator++;
  }
  if (previous != nullptr) {
    previous->setNext(first);
  }
}

wstring FieldTermStack::getFieldName() { return fieldName; }

shared_ptr<TermInfo> FieldTermStack::pop() { return termList.pop_front(); }

void FieldTermStack::push(shared_ptr<TermInfo> termInfo)
{
  termList.push(termInfo);
}

bool FieldTermStack::isEmpty() { return termList.empty() || termList.empty(); }

FieldTermStack::TermInfo::TermInfo(const wstring &text, int startOffset,
                                   int endOffset, int position, float weight)
    : text(text), startOffset(startOffset), endOffset(endOffset),
      position(position), weight(weight)
{
  // C++ TODO: You cannot use 'shared_from_this' in a constructor:
  this->next = shared_from_this();
}

void FieldTermStack::TermInfo::setNext(shared_ptr<TermInfo> next)
{
  this->next = next;
}

shared_ptr<TermInfo> FieldTermStack::TermInfo::getNext() { return next; }

wstring FieldTermStack::TermInfo::getText() { return text; }

int FieldTermStack::TermInfo::getStartOffset() { return startOffset; }

int FieldTermStack::TermInfo::getEndOffset() { return endOffset; }

int FieldTermStack::TermInfo::getPosition() { return position; }

float FieldTermStack::TermInfo::getWeight() { return weight; }

wstring FieldTermStack::TermInfo::toString()
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  sb->append(text)
      ->append(L'(')
      ->append(startOffset)
      ->append(L',')
      ->append(endOffset)
      ->append(L',')
      ->append(position)
      ->append(L')');
  return sb->toString();
}

int FieldTermStack::TermInfo::compareTo(shared_ptr<TermInfo> o)
{
  return (this->position - o->position);
}

int FieldTermStack::TermInfo::hashCode()
{
  constexpr int prime = 31;
  int result = 1;
  result = prime * result + position;
  return result;
}

bool FieldTermStack::TermInfo::equals(any obj)
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
  shared_ptr<TermInfo> other = any_cast<std::shared_ptr<TermInfo>>(obj);
  if (position != other->position) {
    return false;
  }
  return true;
}
} // namespace org::apache::lucene::search::vectorhighlight