using namespace std;

#include "FieldHighlighter.h"

namespace org::apache::lucene::search::uhighlight
{
using IndexReader = org::apache::lucene::index::IndexReader;
using BytesRef = org::apache::lucene::util::BytesRef;

FieldHighlighter::FieldHighlighter(
    const wstring &field, shared_ptr<FieldOffsetStrategy> fieldOffsetStrategy,
    shared_ptr<BreakIterator> breakIterator,
    shared_ptr<PassageScorer> passageScorer, int maxPassages,
    int maxNoHighlightPassages, shared_ptr<PassageFormatter> passageFormatter)
    : field(field), fieldOffsetStrategy(fieldOffsetStrategy),
      breakIterator(breakIterator), passageScorer(passageScorer),
      maxPassages(maxPassages), maxNoHighlightPassages(maxNoHighlightPassages),
      passageFormatter(passageFormatter)
{
}

wstring FieldHighlighter::getField() { return field; }

UnifiedHighlighter::OffsetSource FieldHighlighter::getOffsetSource()
{
  return fieldOffsetStrategy->getOffsetSource();
}

any FieldHighlighter::highlightFieldForDoc(
    shared_ptr<IndexReader> reader, int docId,
    const wstring &content) 
{
  // TODO accept LeafReader instead?
  // note: it'd be nice to accept a std::wstring for content, but we need a
  // CharacterIterator impl for it.
  if (content.length() == 0) {
    return nullptr; // nothing to do
  }

  breakIterator->setText(content);

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (OffsetsEnum offsetsEnums =
  // fieldOffsetStrategy.getOffsetsEnum(reader, docId, content))
  {
    OffsetsEnum offsetsEnums =
        fieldOffsetStrategy->getOffsetsEnum(reader, docId, content);

    // Highlight the offsetsEnum deque against the content to produce Passages.
    std::deque<std::shared_ptr<Passage>> passages =
        highlightOffsetsEnums(offsetsEnums); // and breakIterator & scorer

    // Format the resulting Passages.
    if (passages.empty()) {
      // no passages were returned, so ask for a default summary
      passages = getSummaryPassagesNoHighlight(
          maxNoHighlightPassages == -1 ? maxPassages : maxNoHighlightPassages);
    }

    if (passages.size() > 0) {
      return passageFormatter->format(passages, content);
    } else {
      return nullptr;
    }
  }
}

std::deque<std::shared_ptr<Passage>>
FieldHighlighter::getSummaryPassagesNoHighlight(int maxPassages)
{
  assert(breakIterator->current() == breakIterator->first());

  deque<std::shared_ptr<Passage>> passages =
      deque<std::shared_ptr<Passage>>(min(maxPassages, 10));
  int pos = breakIterator->current();
  assert(pos == 0);
  while (passages.size() < maxPassages) {
    int next = breakIterator->next();
    if (next == BreakIterator::DONE) {
      break;
    }
    shared_ptr<Passage> passage = make_shared<Passage>();
    passage->setStartOffset(pos);
    passage->setEndOffset(next);
    passages.push_back(passage);
    pos = next;
  }

  return passages.toArray(
      std::deque<std::shared_ptr<Passage>>(passages.size()));
}

std::deque<std::shared_ptr<Passage>> FieldHighlighter::highlightOffsetsEnums(
    shared_ptr<OffsetsEnum> off) 
{

  constexpr int contentLength = this->breakIterator->getText().getEndIndex();

  if (off->nextPosition() == false) {
    return std::deque<std::shared_ptr<Passage>>(0);
  }

  shared_ptr<PriorityQueue<std::shared_ptr<Passage>>> passageQueue =
      make_shared<PriorityQueue<std::shared_ptr<Passage>>>(
          min(64, maxPassages + 1), [&](left, right) {
            if (left::getScore() < right::getScore()) {
              return -1;
            } else if (left::getScore() > right::getScore()) {
              return 1;
            } else {
              return left::getStartOffset() - right::getStartOffset();
            }
          });
  shared_ptr<Passage> passage =
      make_shared<Passage>(); // the current passage in-progress.  Will either
                              // get reset or added to queue.

  do {
    int start = off->startOffset();
    if (start == -1) {
      throw invalid_argument(
          L"field '" + field +
          L"' was indexed without offsets, cannot highlight");
    }
    int end = off->endOffset();
    if (start < contentLength && end > contentLength) {
      continue;
    }
    // See if this term should be part of a new passage.
    if (start >= passage->getEndOffset()) {
      passage =
          maybeAddPassage(passageQueue, passageScorer, passage, contentLength);
      // if we exceed limit, we are done
      if (start >= contentLength) {
        break;
      }
      // advance breakIterator
      passage->setStartOffset(
          max(this->breakIterator->preceding(start + 1), 0));
      passage->setEndOffset(
          min(this->breakIterator->following(start), contentLength));
    }
    // Add this term to the passage.
    shared_ptr<BytesRef> term = off->getTerm(); // a reference; safe to refer to
    assert(term != nullptr);
    passage->addMatch(start, end, term, off->freq());
  } while (off->nextPosition());
  maybeAddPassage(passageQueue, passageScorer, passage, contentLength);

  std::deque<std::shared_ptr<Passage>> passages = passageQueue->toArray(
      std::deque<std::shared_ptr<Passage>>(passageQueue->size()));
  // sort in ascending order
  Arrays::sort(passages, Comparator::comparingInt(Passage::getStartOffset));
  return passages;
}

shared_ptr<Passage> FieldHighlighter::maybeAddPassage(
    shared_ptr<PriorityQueue<std::shared_ptr<Passage>>> passageQueue,
    shared_ptr<PassageScorer> scorer, shared_ptr<Passage> passage,
    int contentLength)
{
  if (passage->getStartOffset() == -1) {
    // empty passage, we can ignore it
    return passage;
  }
  passage->setScore(scorer->score(passage, contentLength));
  // new sentence: first add 'passage' to queue
  if (passageQueue->size() == maxPassages &&
      passage->getScore() < passageQueue->peek().getScore()) {
    passage->reset(); // can't compete, just reset it
  } else {
    passageQueue->offer(passage);
    if (passageQueue->size() > maxPassages) {
      passage = passageQueue->poll();
      passage->reset();
    } else {
      passage = make_shared<Passage>();
    }
  }
  return passage;
}
} // namespace org::apache::lucene::search::uhighlight