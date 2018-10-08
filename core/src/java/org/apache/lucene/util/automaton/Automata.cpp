using namespace std;

#include "Automata.h"

namespace org::apache::lucene::util::automaton
{
using BytesRef = org::apache::lucene::util::BytesRef;
using StringHelper = org::apache::lucene::util::StringHelper;

Automata::Automata() {}

shared_ptr<Automaton> Automata::makeEmpty()
{
  shared_ptr<Automaton> a = make_shared<Automaton>();
  a->finishState();
  return a;
}

shared_ptr<Automaton> Automata::makeEmptyString()
{
  shared_ptr<Automaton> a = make_shared<Automaton>();
  a->createState();
  a->setAccept(0, true);
  return a;
}

shared_ptr<Automaton> Automata::makeAnyString()
{
  shared_ptr<Automaton> a = make_shared<Automaton>();
  int s = a->createState();
  a->setAccept(s, true);
  a->addTransition(s, s, Character::MIN_CODE_POINT, Character::MAX_CODE_POINT);
  a->finishState();
  return a;
}

shared_ptr<Automaton> Automata::makeAnyBinary()
{
  shared_ptr<Automaton> a = make_shared<Automaton>();
  int s = a->createState();
  a->setAccept(s, true);
  a->addTransition(s, s, 0, 255);
  a->finishState();
  return a;
}

shared_ptr<Automaton> Automata::makeAnyChar()
{
  return makeCharRange(Character::MIN_CODE_POINT, Character::MAX_CODE_POINT);
}

int Automata::appendAnyChar(shared_ptr<Automaton> a, int state)
{
  int newState = a->createState();
  a->addTransition(state, newState, Character::MIN_CODE_POINT,
                   Character::MAX_CODE_POINT);
  return newState;
}

shared_ptr<Automaton> Automata::makeChar(int c) { return makeCharRange(c, c); }

int Automata::appendChar(shared_ptr<Automaton> a, int state, int c)
{
  int newState = a->createState();
  a->addTransition(state, newState, c, c);
  return newState;
}

shared_ptr<Automaton> Automata::makeCharRange(int min, int max)
{
  if (min > max) {
    return makeEmpty();
  }
  shared_ptr<Automaton> a = make_shared<Automaton>();
  int s1 = a->createState();
  int s2 = a->createState();
  a->setAccept(s2, true);
  a->addTransition(s1, s2, min, max);
  a->finishState();
  return a;
}

int Automata::anyOfRightLength(shared_ptr<Automaton::Builder> builder,
                               const wstring &x, int n)
{
  int s = builder->createState();
  if (x.length() == n) {
    builder->setAccept(s, true);
  } else {
    builder->addTransition(s, anyOfRightLength(builder, x, n + 1), L'0', L'9');
  }
  return s;
}

int Automata::atLeast(shared_ptr<Automaton::Builder> builder, const wstring &x,
                      int n, shared_ptr<deque<int>> initials, bool zeros)
{
  int s = builder->createState();
  if (x.length() == n) {
    builder->setAccept(s, true);
  } else {
    if (zeros) {
      initials->add(s);
    }
    wchar_t c = x[n];
    builder->addTransition(
        s, atLeast(builder, x, n + 1, initials, zeros && c == L'0'), c);
    if (c < L'9') {
      builder->addTransition(s, anyOfRightLength(builder, x, n + 1),
                             static_cast<wchar_t>(c + 1), L'9');
    }
  }
  return s;
}

int Automata::atMost(shared_ptr<Automaton::Builder> builder, const wstring &x,
                     int n)
{
  int s = builder->createState();
  if (x.length() == n) {
    builder->setAccept(s, true);
  } else {
    wchar_t c = x[n];
    builder->addTransition(s, atMost(builder, x, static_cast<wchar_t>(n) + 1),
                           c);
    if (c > L'0') {
      builder->addTransition(s, anyOfRightLength(builder, x, n + 1), L'0',
                             static_cast<wchar_t>(c - 1));
    }
  }
  return s;
}

int Automata::between(shared_ptr<Automaton::Builder> builder, const wstring &x,
                      const wstring &y, int n,
                      shared_ptr<deque<int>> initials, bool zeros)
{
  int s = builder->createState();
  if (x.length() == n) {
    builder->setAccept(s, true);
  } else {
    if (zeros) {
      initials->add(s);
    }
    wchar_t cx = x[n];
    wchar_t cy = y[n];
    if (cx == cy) {
      builder->addTransition(
          s, between(builder, x, y, n + 1, initials, zeros && cx == L'0'), cx);
    } else { // cx<cy
      builder->addTransition(
          s, atLeast(builder, x, n + 1, initials, zeros && cx == L'0'), cx);
      builder->addTransition(s, atMost(builder, y, n + 1), cy);
      if (cx + 1 < cy) {
        builder->addTransition(s, anyOfRightLength(builder, x, n + 1),
                               static_cast<wchar_t>(cx + 1),
                               static_cast<wchar_t>(cy - 1));
      }
    }
  }

  return s;
}

bool Automata::suffixIsZeros(shared_ptr<BytesRef> br, int len)
{
  for (int i = len; i < br->length; i++) {
    if (br->bytes[br->offset + i] != 0) {
      return false;
    }
  }

  return true;
}

shared_ptr<Automaton> Automata::makeBinaryInterval(shared_ptr<BytesRef> min,
                                                   bool minInclusive,
                                                   shared_ptr<BytesRef> max,
                                                   bool maxInclusive)
{

  if (min == nullptr && minInclusive == false) {
    throw invalid_argument(
        L"minInclusive must be true when min is null (open ended)");
  }

  if (max == nullptr && maxInclusive == false) {
    throw invalid_argument(
        L"maxInclusive must be true when max is null (open ended)");
  }

  if (min == nullptr) {
    min = make_shared<BytesRef>();
    minInclusive = true;
  }

  int cmp;
  if (max != nullptr) {
    cmp = min->compareTo(max);
  } else {
    cmp = -1;
    if (min->length == 0 && minInclusive) {
      return makeAnyBinary();
    }
  }

  if (cmp == 0) {
    if (minInclusive == false || maxInclusive == false) {
      return makeEmpty();
    } else {
      return makeBinary(min);
    }
  } else if (cmp > 0) {
    // max > min
    return makeEmpty();
  }

  if (max != nullptr && StringHelper::startsWith(max, min) &&
      suffixIsZeros(max, min->length)) {

    // Finite case: no sink state!

    int maxLength = max->length;

    // the == case was handled above
    assert(maxLength > min->length);

    //  bar -> bar\0+
    if (maxInclusive == false) {
      maxLength--;
    }

    if (maxLength == min->length) {
      if (minInclusive == false) {
        return makeEmpty();
      } else {
        return makeBinary(min);
      }
    }

    shared_ptr<Automaton> a = make_shared<Automaton>();
    int lastState = a->createState();
    for (int i = 0; i < min->length; i++) {
      int state = a->createState();
      int label = min->bytes[min->offset + i] & 0xff;
      a->addTransition(lastState, state, label);
      lastState = state;
    }

    if (minInclusive) {
      a->setAccept(lastState, true);
    }

    for (int i = min->length; i < maxLength; i++) {
      int state = a->createState();
      a->addTransition(lastState, state, 0);
      a->setAccept(state, true);
      lastState = state;
    }
    a->finishState();
    return a;
  }

  shared_ptr<Automaton> a = make_shared<Automaton>();
  int startState = a->createState();

  int sinkState = a->createState();
  a->setAccept(sinkState, true);

  // This state accepts all suffixes:
  a->addTransition(sinkState, sinkState, 0, 255);

  bool equalPrefix = true;
  int lastState = startState;
  int firstMaxState = -1;
  int sharedPrefixLength = 0;
  for (int i = 0; i < min->length; i++) {
    int minLabel = min->bytes[min->offset + i] & 0xff;

    int maxLabel;
    if (max != nullptr && equalPrefix && i < max->length) {
      maxLabel = max->bytes[max->offset + i] & 0xff;
    } else {
      maxLabel = -1;
    }

    int nextState;
    if (minInclusive && i == min->length - 1 &&
        (equalPrefix == false || minLabel != maxLabel)) {
      nextState = sinkState;
    } else {
      nextState = a->createState();
    }

    if (equalPrefix) {

      if (minLabel == maxLabel) {
        // Still in shared prefix
        a->addTransition(lastState, nextState, minLabel);
      } else if (max == nullptr) {
        equalPrefix = false;
        sharedPrefixLength = 0;
        a->addTransition(lastState, sinkState, minLabel + 1, 0xff);
        a->addTransition(lastState, nextState, minLabel);
      } else {
        // This is the first point where min & max diverge:
        assert(maxLabel > minLabel);

        a->addTransition(lastState, nextState, minLabel);

        if (maxLabel > minLabel + 1) {
          a->addTransition(lastState, sinkState, minLabel + 1, maxLabel - 1);
        }

        // Now fork off path for max:
        if (maxInclusive || i < max->length - 1) {
          firstMaxState = a->createState();
          if (i < max->length - 1) {
            a->setAccept(firstMaxState, true);
          }
          a->addTransition(lastState, firstMaxState, maxLabel);
        }
        equalPrefix = false;
        sharedPrefixLength = i;
      }
    } else {
      // OK, already diverged:
      a->addTransition(lastState, nextState, minLabel);
      if (minLabel < 255) {
        a->addTransition(lastState, sinkState, minLabel + 1, 255);
      }
    }
    lastState = nextState;
  }

  // Accept any suffix appended to the min term:
  if (equalPrefix == false && lastState != sinkState &&
      lastState != startState) {
    a->addTransition(lastState, sinkState, 0, 255);
  }

  if (minInclusive) {
    // Accept exactly the min term:
    a->setAccept(lastState, true);
  }

  if (max != nullptr) {

    // Now do max:
    if (firstMaxState == -1) {
      // Min was a full prefix of max
      sharedPrefixLength = min->length;
    } else {
      lastState = firstMaxState;
      sharedPrefixLength++;
    }
    for (int i = sharedPrefixLength; i < max->length; i++) {
      int maxLabel = max->bytes[max->offset + i] & 0xff;
      if (maxLabel > 0) {
        a->addTransition(lastState, sinkState, 0, maxLabel - 1);
      }
      if (maxInclusive || i < max->length - 1) {
        int nextState = a->createState();
        if (i < max->length - 1) {
          a->setAccept(nextState, true);
        }
        a->addTransition(lastState, nextState, maxLabel);
        lastState = nextState;
      }
    }

    if (maxInclusive) {
      a->setAccept(lastState, true);
    }
  }

  a->finishState();

  assert((a->isDeterministic(), a->toDot()));

  return a;
}

shared_ptr<Automaton>
Automata::makeDecimalInterval(int min, int max,
                              int digits) 
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring x = Integer::toString(min);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring y = Integer::toString(max);
  if (min > max || (digits > 0 && y.length() > digits)) {
    // C++ TODO: This exception's constructor requires an argument:
    // ORIGINAL LINE: throw new IllegalArgumentException();
    throw invalid_argument();
  }
  int d;
  if (digits > 0) {
    d = digits;
  } else {
    d = y.length();
  }
  shared_ptr<StringBuilder> bx = make_shared<StringBuilder>();
  for (int i = x.length(); i < d; i++) {
    bx->append(L'0');
  }
  bx->append(x);
  x = bx->toString();
  shared_ptr<StringBuilder> by = make_shared<StringBuilder>();
  for (int i = y.length(); i < d; i++) {
    by->append(L'0');
  }
  by->append(y);
  y = by->toString();

  shared_ptr<Automaton::Builder> builder = make_shared<Automaton::Builder>();

  if (digits <= 0) {
    // Reserve the "real" initial state:
    builder->createState();
  }

  shared_ptr<deque<int>> initials = deque<int>();

  between(builder, x, y, 0, initials, digits <= 0);

  shared_ptr<Automaton> a1 = builder->finish();

  if (digits <= 0) {
    a1->addTransition(0, 0, L'0');
    for (auto p : initials) {
      a1->addEpsilon(0, p);
    }
    a1->finishState();
  }

  return a1;
}

shared_ptr<Automaton> Automata::makeString(const wstring &s)
{
  shared_ptr<Automaton> a = make_shared<Automaton>();
  int lastState = a->createState();
  for (int i = 0, cp = 0; i < s.length(); i += Character::charCount(cp)) {
    int state = a->createState();
    cp = s.codePointAt(i);
    a->addTransition(lastState, state, cp);
    lastState = state;
  }

  a->setAccept(lastState, true);
  a->finishState();

  assert(a->isDeterministic());
  assert(Operations::hasDeadStates(a) == false);

  return a;
}

shared_ptr<Automaton> Automata::makeBinary(shared_ptr<BytesRef> term)
{
  shared_ptr<Automaton> a = make_shared<Automaton>();
  int lastState = a->createState();
  for (int i = 0; i < term->length; i++) {
    int state = a->createState();
    int label = term->bytes[term->offset + i] & 0xff;
    a->addTransition(lastState, state, label);
    lastState = state;
  }

  a->setAccept(lastState, true);
  a->finishState();

  assert(a->isDeterministic());
  assert(Operations::hasDeadStates(a) == false);

  return a;
}

shared_ptr<Automaton> Automata::makeString(std::deque<int> &word, int offset,
                                           int length)
{
  shared_ptr<Automaton> a = make_shared<Automaton>();
  a->createState();
  int s = 0;
  for (int i = offset; i < offset + length; i++) {
    int s2 = a->createState();
    a->addTransition(s, s2, word[i]);
    s = s2;
  }
  a->setAccept(s, true);
  a->finishState();

  return a;
}

shared_ptr<Automaton> Automata::makeStringUnion(
    shared_ptr<deque<std::shared_ptr<BytesRef>>> utf8Strings)
{
  if (utf8Strings->isEmpty()) {
    return makeEmpty();
  } else {
    return DaciukMihovAutomatonBuilder::build(utf8Strings);
  }
}
} // namespace org::apache::lucene::util::automaton