using namespace std;

#include "FieldOffsetStrategy.h"

namespace org::apache::lucene::search::uhighlight
{
using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReader = org::apache::lucene::index::LeafReader;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using BytesRef = org::apache::lucene::util::BytesRef;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;
using CharacterRunAutomaton =
    org::apache::lucene::util::automaton::CharacterRunAutomaton;

FieldOffsetStrategy::FieldOffsetStrategy(
    const wstring &field, std::deque<std::shared_ptr<BytesRef>> &queryTerms,
    shared_ptr<PhraseHelper> phraseHelper,
    std::deque<std::shared_ptr<CharacterRunAutomaton>> &automata)
    : field(field), phraseHelper(phraseHelper), terms(queryTerms),
      automata(automata)
{
}

wstring FieldOffsetStrategy::getField() { return field; }

shared_ptr<OffsetsEnum> FieldOffsetStrategy::createOffsetsEnumFromReader(
    shared_ptr<LeafReader> leafReader, int doc) 
{
  shared_ptr<Terms> *const termsIndex = leafReader->terms(field);
  if (termsIndex == nullptr) {
    return OffsetsEnum::EMPTY;
  }

  const deque<std::shared_ptr<OffsetsEnum>> offsetsEnums =
      deque<std::shared_ptr<OffsetsEnum>>(terms.size() + automata.size());

  // Handle position insensitive terms (a subset of this.terms field):
  std::deque<std::shared_ptr<BytesRef>> insensitiveTerms;
  if (phraseHelper->hasPositionSensitivity()) {
    insensitiveTerms = phraseHelper->getAllPositionInsensitiveTerms();
    assert((insensitiveTerms.size() <= terms.size(),
            L"insensitive terms should be smaller set of all terms"));
  } else {
    insensitiveTerms = terms;
  }
  if (insensitiveTerms.size() > 0) {
    createOffsetsEnumsForTerms(insensitiveTerms, termsIndex, doc, offsetsEnums);
  }

  // Handle spans
  if (phraseHelper->hasPositionSensitivity()) {
    phraseHelper->createOffsetsEnumsForSpans(leafReader, doc, offsetsEnums);
  }

  // Handle automata
  if (automata.size() > 0) {
    createOffsetsEnumsForAutomata(termsIndex, doc, offsetsEnums);
  }

  return make_shared<OffsetsEnum::MultiOffsetsEnum>(offsetsEnums);
}

void FieldOffsetStrategy::createOffsetsEnumsForTerms(
    std::deque<std::shared_ptr<BytesRef>> &sourceTerms,
    shared_ptr<Terms> termsIndex, int doc,
    deque<std::shared_ptr<OffsetsEnum>> &results) 
{
  shared_ptr<TermsEnum> termsEnum = termsIndex->begin(); // does not return null
  for (auto term : sourceTerms) {
    if (termsEnum->seekExact(term)) {
      shared_ptr<PostingsEnum> postingsEnum =
          termsEnum->postings(nullptr, PostingsEnum::OFFSETS);
      if (postingsEnum == nullptr) {
        // no offsets or positions available
        throw invalid_argument(
            L"field '" + field +
            L"' was indexed without offsets, cannot highlight");
      }
      if (doc == postingsEnum->advance(
                     doc)) { // now it's positioned, although may be exhausted
        results.push_back(
            make_shared<OffsetsEnum::OfPostings>(term, postingsEnum));
      }
    }
  }
}

void FieldOffsetStrategy::createOffsetsEnumsForAutomata(
    shared_ptr<Terms> termsIndex, int doc,
    deque<std::shared_ptr<OffsetsEnum>> &results) 
{
  deque<deque<std::shared_ptr<PostingsEnum>>> automataPostings =
      deque<deque<std::shared_ptr<PostingsEnum>>>(automata.size());
  for (int i = 0; i < automata.size(); i++) {
    automataPostings.push_back(deque<>());
  }

  shared_ptr<TermsEnum> termsEnum = termsIndex->begin();
  shared_ptr<BytesRef> term;

  shared_ptr<CharsRefBuilder> refBuilder = make_shared<CharsRefBuilder>();
  while ((term = termsEnum->next()) != nullptr) {
    for (int i = 0; i < automata.size(); i++) {
      shared_ptr<CharacterRunAutomaton> automaton = automata[i];
      refBuilder->copyUTF8Bytes(term);
      if (automaton->run(refBuilder->chars(), 0, refBuilder->length())) {
        shared_ptr<PostingsEnum> postings =
            termsEnum->postings(nullptr, PostingsEnum::OFFSETS);
        if (doc == postings->advance(doc)) {
          automataPostings[i].push_back(postings);
        }
      }
    }
  }

  for (int i = 0; i < automata.size(); i++) {
    shared_ptr<CharacterRunAutomaton> automaton = automata[i];
    deque<std::shared_ptr<PostingsEnum>> postingsEnums = automataPostings[i];
    if (postingsEnums.empty()) {
      continue;
    }
    // Build one OffsetsEnum exposing the automata.toString as the term, and the
    // sum of freq
    // C++ TODO: There is no native C++ equivalent to 'toString':
    shared_ptr<BytesRef> wildcardTerm =
        make_shared<BytesRef>(automaton->toString());
    int sumFreq = 0;
    for (auto postingsEnum : postingsEnums) {
      sumFreq += postingsEnum->freq();
    }
    for (auto postingsEnum : postingsEnums) {
      results.push_back(make_shared<OffsetsEnum::OfPostings>(
          wildcardTerm, sumFreq, postingsEnum));
    }
  }
}
} // namespace org::apache::lucene::search::uhighlight