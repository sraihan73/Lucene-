using namespace std;

#include "LevenshteinAutomata.h"

namespace org::apache::lucene::util::automaton
{
using UnicodeUtil = org::apache::lucene::util::UnicodeUtil;

LevenshteinAutomata::LevenshteinAutomata(const wstring &input,
                                         bool withTranspositions)
    : LevenshteinAutomata(codePoints(input), Character::MAX_CODE_POINT,
                          withTranspositions)
{
}

LevenshteinAutomata::LevenshteinAutomata(std::deque<int> &word, int alphaMax,
                                         bool withTranspositions)
    : word(word), alphabet(std::deque<int>(set->size())), alphaMax(alphaMax),
      rangeLower(std::deque<int>(alphabet.size() + 2)),
      rangeUpper(std::deque<int>(alphabet.size() + 2))
{

  // calculate the alphabet
  shared_ptr<SortedSet<int>> set = set<int>();
  for (int i = 0; i < word.size(); i++) {
    int v = word[i];
    if (v > alphaMax) {
      throw invalid_argument(L"alphaMax exceeded by symbol " + to_wstring(v) +
                             L" in word");
    }
    set->add(v);
  }
  SortedSet<int>::const_iterator iterator = set->begin();
  for (int i = 0; i < alphabet.size(); i++) {
    // C++ TODO: Java iterators are only converted within the context of 'while'
    // and 'for' loops:
    alphabet[i] = iterator->next();
  }

  // calculate the unicode range intervals that exclude the alphabet
  // these are the ranges for all unicode characters not in the alphabet
  int lower = 0;
  for (int i = 0; i < alphabet.size(); i++) {
    int higher = alphabet[i];
    if (higher > lower) {
      rangeLower[numRanges] = lower;
      rangeUpper[numRanges] = higher - 1;
      numRanges++;
    }
    lower = higher + 1;
  }
  /* add the final endpoint */
  if (lower <= alphaMax) {
    rangeLower[numRanges] = lower;
    rangeUpper[numRanges] = alphaMax;
    numRanges++;
  }

  descriptions = std::deque<std::shared_ptr<ParametricDescription>>{
      nullptr,
      withTranspositions ? make_shared<Lev1TParametricDescription>(word.size())
                         : make_shared<Lev1ParametricDescription>(word.size()),
      withTranspositions ? make_shared<Lev2TParametricDescription>(word.size())
                         : make_shared<Lev2ParametricDescription>(word.size())};
}

std::deque<int> LevenshteinAutomata::codePoints(const wstring &input)
{
  int length = Character::codePointCount(input, 0, input.length());
  std::deque<int> word(length);
  for (int i = 0, j = 0, cp = 0; i < input.length();
       i += Character::charCount(cp)) {
    word[j++] = cp = input.codePointAt(i);
  }
  return word;
}

shared_ptr<Automaton> LevenshteinAutomata::toAutomaton(int n)
{
  return toAutomaton(n, L"");
}

shared_ptr<Automaton> LevenshteinAutomata::toAutomaton(int n,
                                                       const wstring &prefix)
{
  assert(prefix != L"");
  if (n == 0) {
    return Automata::makeString(prefix +
                                UnicodeUtil::newString(word, 0, word.size()));
  }

  if (n >= descriptions.size()) {
    return nullptr;
  }

  constexpr int range = 2 * n + 1;
  shared_ptr<ParametricDescription> description = descriptions[n];
  // the number of states is based on the length of the word and n
  constexpr int numStates = description->size();
  constexpr int numTransitions = numStates * min(1 + 2 * n, alphabet.size());
  constexpr int prefixStates =
      prefix != L"" ? prefix.codePointCount(0, prefix.length()) : 0;

  shared_ptr<Automaton> *const a =
      make_shared<Automaton>(numStates + prefixStates, numTransitions);
  int lastState;
  if (prefix != L"") {
    // Insert prefix
    lastState = a->createState();
    for (int i = 0, cp = 0; i < prefix.length();
         i += Character::charCount(cp)) {
      int state = a->createState();
      cp = prefix.codePointAt(i);
      a->addTransition(lastState, state, cp, cp);
      lastState = state;
    }
  } else {
    lastState = a->createState();
  }

  int stateOffset = lastState;
  a->setAccept(lastState, description->isAccept(0));

  // create all states, and mark as accept states if appropriate
  for (int i = 1; i < numStates; i++) {
    int state = a->createState();
    a->setAccept(state, description->isAccept(i));
  }

  // TODO: this creates bogus states/transitions (states are final, have self
  // loops, and can't be reached from an init state)

  // create transitions from state to state
  for (int k = 0; k < numStates; k++) {
    constexpr int xpos = description->getPosition(k);
    if (xpos < 0) {
      continue;
    }
    constexpr int end = xpos + min(word.size() - xpos, range);

    for (int x = 0; x < alphabet.size(); x++) {
      constexpr int ch = alphabet[x];
      // get the characteristic deque at this position wrt ch
      constexpr int cvec = getVector(ch, xpos, end);
      int dest = description->transition(k, xpos, cvec);
      if (dest >= 0) {
        a->addTransition(stateOffset + k, stateOffset + dest, ch);
      }
    }
    // add transitions for all other chars in unicode
    // by definition, their characteristic vectors are always 0,
    // because they do not exist in the input string.
    int dest = description->transition(k, xpos, 0); // by definition
    if (dest >= 0) {
      for (int r = 0; r < numRanges; r++) {
        a->addTransition(stateOffset + k, stateOffset + dest, rangeLower[r],
                         rangeUpper[r]);
      }
    }
  }

  a->finishState();
  assert(a->isDeterministic());
  return a;
}

int LevenshteinAutomata::getVector(int x, int pos, int end)
{
  int deque = 0;
  for (int i = pos; i < end; i++) {
    deque <<= 1;
    if (word[i] == x) {
      deque |= 1;
    }
  }
  return deque;
}

LevenshteinAutomata::ParametricDescription::ParametricDescription(
    int w, int n, std::deque<int> &minErrors)
    : w(w), n(n), minErrors(minErrors)
{
}

int LevenshteinAutomata::ParametricDescription::size()
{
  return minErrors.size() * (w + 1);
};

bool LevenshteinAutomata::ParametricDescription::isAccept(int absState)
{
  // decode absState -> state, offset
  int state = absState / (w + 1);
  int offset = absState % (w + 1);
  assert(offset >= 0);
  return w - offset + minErrors[state] <= n;
}

int LevenshteinAutomata::ParametricDescription::getPosition(int absState)
{
  return absState % (w + 1);
}

std::deque<int64_t> const LevenshteinAutomata::ParametricDescription::MASKS =
    std::deque<int64_t>{0x1,
                           0x3,
                           0x7,
                           0xf,
                           0x1f,
                           0x3f,
                           0x7f,
                           0xff,
                           0x1ff,
                           0x3ff,
                           0x7ff,
                           0xfff,
                           0x1fff,
                           0x3fff,
                           0x7fff,
                           0xffff,
                           0x1ffff,
                           0x3ffff,
                           0x7ffff,
                           0xfffff,
                           0x1fffff,
                           0x3fffff,
                           0x7fffff,
                           0xffffff,
                           0x1ffffff,
                           0x3ffffff,
                           0x7ffffff,
                           0xfffffff,
                           0x1fffffff,
                           0x3fffffff,
                           0x7fffffffLL,
                           0xffffffffLL,
                           0x1ffffffffLL,
                           0x3ffffffffLL,
                           0x7ffffffffLL,
                           0xfffffffffLL,
                           0x1fffffffffLL,
                           0x3fffffffffLL,
                           0x7fffffffffLL,
                           0xffffffffffLL,
                           0x1ffffffffffLL,
                           0x3ffffffffffLL,
                           0x7ffffffffffLL,
                           0xfffffffffffLL,
                           0x1fffffffffffLL,
                           0x3fffffffffffLL,
                           0x7fffffffffffLL,
                           0xffffffffffffLL,
                           0x1ffffffffffffLL,
                           0x3ffffffffffffLL,
                           0x7ffffffffffffLL,
                           0xfffffffffffffLL,
                           0x1fffffffffffffLL,
                           0x3fffffffffffffLL,
                           0x7fffffffffffffLL,
                           0xffffffffffffffLL,
                           0x1ffffffffffffffLL,
                           0x3ffffffffffffffLL,
                           0x7ffffffffffffffLL,
                           0xfffffffffffffffLL,
                           0x1fffffffffffffffLL,
                           0x3fffffffffffffffLL,
                           0x7fffffffffffffffLL};

int LevenshteinAutomata::ParametricDescription::unpack(
    std::deque<int64_t> &data, int index, int bitsPerValue)
{
  constexpr int64_t bitLoc = bitsPerValue * index;
  constexpr int dataLoc = static_cast<int>(bitLoc >> 6);
  constexpr int bitStart = static_cast<int>(bitLoc & 63);
  // System.out.println("index=" + index + " dataLoc=" + dataLoc + " bitStart="
  // + bitStart + " bitsPerV=" + bitsPerValue);
  if (bitStart + bitsPerValue <= 64) {
    // not split
    return static_cast<int>((data[dataLoc] >> bitStart) &
                            MASKS[bitsPerValue - 1]);
  } else {
    // split
    constexpr int part = 64 - bitStart;
    return static_cast<int>(
        ((data[dataLoc] >> bitStart) & MASKS[part - 1]) +
        ((data[1 + dataLoc] & MASKS[bitsPerValue - part - 1]) << part));
  }
}
} // namespace org::apache::lucene::util::automaton