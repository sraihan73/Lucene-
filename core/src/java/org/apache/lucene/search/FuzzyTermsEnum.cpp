using namespace std;

#include "FuzzyTermsEnum.h"

namespace org::apache::lucene::search
{
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Term = org::apache::lucene::index::Term;
using TermState = org::apache::lucene::index::TermState;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using Attribute = org::apache::lucene::util::Attribute;
using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using AttributeReflector = org::apache::lucene::util::AttributeReflector;
using AttributeSource = org::apache::lucene::util::AttributeSource;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using UnicodeUtil = org::apache::lucene::util::UnicodeUtil;
using Automaton = org::apache::lucene::util::automaton::Automaton;
using CompiledAutomaton =
    org::apache::lucene::util::automaton::CompiledAutomaton;
using LevenshteinAutomata =
    org::apache::lucene::util::automaton::LevenshteinAutomata;

FuzzyTermsEnum::FuzzyTermsEnum(shared_ptr<Terms> terms,
                               shared_ptr<AttributeSource> atts,
                               shared_ptr<Term> term, int const maxEdits,
                               int const prefixLength,
                               bool transpositions) 
    : boostAtt(attributes()->addAttribute(BoostAttribute::typeid)),
      maxBoostAtt(atts->addAttribute(MaxNonCompetitiveBoostAttribute::typeid)),
      dfaAtt(atts->addAttribute(LevenshteinAutomataAttribute::typeid)),
      automata(prevAutomata), termLength(termText.size()), terms(terms),
      term(term),
      termText(std::deque<int>(utf16::codePointCount(0, utf16->length()))),
      realPrefixLength(prefixLength > termLength ? termLength : prefixLength),
      transpositions(transpositions)
{
  if (maxEdits < 0 ||
      maxEdits > LevenshteinAutomata::MAXIMUM_SUPPORTED_DISTANCE) {
    throw invalid_argument(
        L"max edits must be 0.." +
        to_wstring(LevenshteinAutomata::MAXIMUM_SUPPORTED_DISTANCE) +
        L", inclusive; got: " + to_wstring(maxEdits));
  }
  if (prefixLength < 0) {
    throw invalid_argument(L"prefixLength cannot be less than 0");
  }
  this->maxEdits = maxEdits;

  // convert the string into a utf32 int[] representation for fast comparisons
  const wstring utf16 = term->text();
  for (shared_ptr<int cp, > i = 0, j = 0; i < utf16.length();
       i += Character::charCount(cp)) {
    termText[j++] = cp = utf16.codePointAt(i);
  }

  // NOTE: boostAtt must pulled from attributes() not from atts!  This is
  // because TopTermsRewrite looks for boostAtt from this TermsEnum's private
  // attributes() and not the global atts passed to us from MultiTermQuery:

  // The prefix could be longer than the word.
  // It's kind of silly though.  It means we must match the entire word.

  std::deque<std::shared_ptr<CompiledAutomaton>> prevAutomata =
      dfaAtt->automata();
  if (prevAutomata.empty()) {
    prevAutomata =
        std::deque<std::shared_ptr<CompiledAutomaton>>(maxEdits + 1);

    shared_ptr<LevenshteinAutomata> builder = make_shared<LevenshteinAutomata>(
        UnicodeUtil::newString(termText, realPrefixLength,
                               termText.size() - realPrefixLength),
        transpositions);

    wstring prefix = UnicodeUtil::newString(termText, 0, realPrefixLength);
    for (int i = 0; i <= maxEdits; i++) {
      shared_ptr<Automaton> a = builder->toAutomaton(i, prefix);
      prevAutomata[i] = make_shared<CompiledAutomaton>(a, true, false);
    }

    // first segment computes the automata, and we share with subsequent
    // segments via this Attribute:
    dfaAtt->setAutomata(prevAutomata);
  }

  bottom = maxBoostAtt->getMaxNonCompetitiveBoost();
  bottomTerm = maxBoostAtt->getCompetitiveTerm();
  bottomChanged(nullptr);
}

shared_ptr<TermsEnum> FuzzyTermsEnum::getAutomatonEnum(
    int editDistance, shared_ptr<BytesRef> lastTerm) 
{
  assert(editDistance < automata.size());
  shared_ptr<CompiledAutomaton> *const compiled = automata[editDistance];
  shared_ptr<BytesRef> initialSeekTerm;
  if (lastTerm == nullptr) {
    // This is the first enum we are pulling:
    initialSeekTerm.reset();
  } else {
    // We are pulling this enum (e.g., ed=1) after iterating for a while already
    // (e.g., ed=2):
    initialSeekTerm = compiled->floor(lastTerm, make_shared<BytesRefBuilder>());
  }
  return terms->intersect(compiled, initialSeekTerm);
}

void FuzzyTermsEnum::bottomChanged(shared_ptr<BytesRef> lastTerm) throw(
    IOException)
{
  int oldMaxEdits = maxEdits;

  // true if the last term encountered is lexicographically equal or after the
  // bottom term in the PQ
  bool termAfter =
      bottomTerm == nullptr ||
      (lastTerm != nullptr && lastTerm->compareTo(bottomTerm) >= 0);

  // as long as the max non-competitive boost is >= the max boost
  // for some edit distance, keep dropping the max edit distance.
  while (maxEdits > 0) {
    float maxBoost =
        1.0f - (static_cast<float>(maxEdits) / static_cast<float>(termLength));
    if (bottom < maxBoost || (bottom == maxBoost && termAfter == false)) {
      break;
    }
    maxEdits--;
  }

  if (oldMaxEdits != maxEdits || lastTerm == nullptr) {
    // This is a very powerful optimization: the maximum edit distance has
    // changed.  This happens because we collect only the top scoring N (= 50,
    // by default) terms, and if e.g. maxEdits=2, and the queue is now full of
    // matching terms, and we notice that the worst entry in that queue is ed=1,
    // then we can switch the automata here to ed=1 which is a big speedup.
    actualEnum = getAutomatonEnum(maxEdits, lastTerm);
  }
}

shared_ptr<BytesRef> FuzzyTermsEnum::next() 
{

  if (queuedBottom != nullptr) {
    bottomChanged(queuedBottom);
    queuedBottom.reset();
  }

  shared_ptr<BytesRef> term;

  term = actualEnum->next();
  if (term == nullptr) {
    // end
    return nullptr;
  }

  int ed = maxEdits;

  // we know the outer DFA always matches.
  // now compute exact edit distance
  while (ed > 0) {
    if (matches(term, ed - 1)) {
      ed--;
    } else {
      break;
    }
  }

  if (ed == 0) { // exact match
    boostAtt->setBoost(1.0F);
  } else {
    constexpr int codePointCount = UnicodeUtil::codePointCount(term);
    int minTermLength = min(codePointCount, termLength);

    float similarity =
        1.0f - static_cast<float>(ed) / static_cast<float>(minTermLength);
    boostAtt->setBoost(similarity);
  }

  constexpr float bottom = maxBoostAtt->getMaxNonCompetitiveBoost();
  shared_ptr<BytesRef> *const bottomTerm = maxBoostAtt->getCompetitiveTerm();
  if (term != nullptr &&
      (bottom != this->bottom || bottomTerm != this->bottomTerm)) {
    this->bottom = bottom;
    this->bottomTerm = bottomTerm;
    // clone the term before potentially doing something with it
    // this is a rare but wonderful occurrence anyway

    // We must delay bottomChanged until the next next() call otherwise we mess
    // up docFreq(), etc., for the current term:
    queuedBottom = BytesRef::deepCopyOf(term);
  }

  return term;
}

bool FuzzyTermsEnum::matches(shared_ptr<BytesRef> termIn, int k)
{
  return k == 0 ? termIn->equals(term_->bytes())
                : automata[k]->runAutomaton.run(termIn->bytes, termIn->offset,
                                                termIn->length);
}

int FuzzyTermsEnum::docFreq() 
{
  return actualEnum->docFreq();
}

int64_t FuzzyTermsEnum::totalTermFreq() 
{
  return actualEnum->totalTermFreq();
}

shared_ptr<PostingsEnum>
FuzzyTermsEnum::postings(shared_ptr<PostingsEnum> reuse,
                         int flags) 
{
  return actualEnum->postings(reuse, flags);
}

void FuzzyTermsEnum::seekExact(shared_ptr<BytesRef> term,
                               shared_ptr<TermState> state) 
{
  actualEnum->seekExact(term, state);
}

shared_ptr<TermState> FuzzyTermsEnum::termState() 
{
  return actualEnum->termState();
}

int64_t FuzzyTermsEnum::ord()  { return actualEnum->ord(); }

bool FuzzyTermsEnum::seekExact(shared_ptr<BytesRef> text) 
{
  return actualEnum->seekExact(text);
}

SeekStatus
FuzzyTermsEnum::seekCeil(shared_ptr<BytesRef> text) 
{
  return actualEnum->seekCeil(text);
}

void FuzzyTermsEnum::seekExact(int64_t ord) 
{
  actualEnum->seekExact(ord);
}

shared_ptr<BytesRef> FuzzyTermsEnum::term() 
{
  return actualEnum->term();
}

std::deque<std::shared_ptr<CompiledAutomaton>>
FuzzyTermsEnum::LevenshteinAutomataAttributeImpl::automata()
{
  return automata_;
}

void FuzzyTermsEnum::LevenshteinAutomataAttributeImpl::setAutomata(
    std::deque<std::shared_ptr<CompiledAutomaton>> &automata)
{
  this->automata_ = automata;
}

void FuzzyTermsEnum::LevenshteinAutomataAttributeImpl::clear()
{
  automata_.clear();
}

int FuzzyTermsEnum::LevenshteinAutomataAttributeImpl::hashCode()
{
  if (automata_.empty()) {
    return 0;
  } else {
    return automata_.hashCode();
  }
}

bool FuzzyTermsEnum::LevenshteinAutomataAttributeImpl::equals(any other)
{
  if (shared_from_this() == other) {
    return true;
  }
  if (!(std::dynamic_pointer_cast<LevenshteinAutomataAttributeImpl>(other) !=
        nullptr)) {
    return false;
  }
  return Arrays::equals(
      automata_,
      (any_cast<std::shared_ptr<LevenshteinAutomataAttributeImpl>>(other))
          .automata_);
}

void FuzzyTermsEnum::LevenshteinAutomataAttributeImpl::copyTo(
    shared_ptr<AttributeImpl> _target)
{
  shared_ptr<LevenshteinAutomataAttribute> target =
      std::static_pointer_cast<LevenshteinAutomataAttribute>(_target);
  if (automata_.empty()) {
    target->setAutomata(nullptr);
  } else {
    target->setAutomata(automata_);
  }
}

void FuzzyTermsEnum::LevenshteinAutomataAttributeImpl::reflectWith(
    AttributeReflector reflector)
{
  reflector(LevenshteinAutomataAttribute::typeid, L"automata", automata_);
}
} // namespace org::apache::lucene::search