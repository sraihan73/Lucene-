using namespace std;

#include "UTF32ToUTF8.h"

namespace org::apache::lucene::util::automaton
{

std::deque<int> const UTF32ToUTF8::startCodes =
    std::deque<int>{0, 128, 2048, 65536};
std::deque<int> const UTF32ToUTF8::endCodes =
    std::deque<int>{127, 2047, 65535, 1114111};
std::deque<int> UTF32ToUTF8::MASKS(32);

UTF32ToUTF8::StaticConstructor::StaticConstructor()
{
  int v = 2;
  for (int i = 0; i < 32; i++) {
    MASKS[i] = v - 1;
    v *= 2;
  }
}

UTF32ToUTF8::StaticConstructor UTF32ToUTF8::staticConstructor;

UTF32ToUTF8::UTF8Sequence::UTF8Sequence()
    : bytes(std::deque<std::shared_ptr<UTF8Byte>>(4))
{
  for (int i = 0; i < 4; i++) {
    bytes[i] = make_shared<UTF8Byte>();
  }
}

int UTF32ToUTF8::UTF8Sequence::byteAt(int idx) { return bytes[idx]->value; }

int UTF32ToUTF8::UTF8Sequence::numBits(int idx) { return bytes[idx]->bits; }

void UTF32ToUTF8::UTF8Sequence::set(int code)
{
  if (code < 128) {
    // 0xxxxxxx
    bytes[0]->value = code;
    bytes[0]->bits = 7;
    len = 1;
  } else if (code < 2048) {
    // 110yyyxx 10xxxxxx
    bytes[0]->value = (6 << 5) | (code >> 6);
    bytes[0]->bits = 5;
    setRest(code, 1);
    len = 2;
  } else if (code < 65536) {
    // 1110yyyy 10yyyyxx 10xxxxxx
    bytes[0]->value = (14 << 4) | (code >> 12);
    bytes[0]->bits = 4;
    setRest(code, 2);
    len = 3;
  } else {
    // 11110zzz 10zzyyyy 10yyyyxx 10xxxxxx
    bytes[0]->value = (30 << 3) | (code >> 18);
    bytes[0]->bits = 3;
    setRest(code, 3);
    len = 4;
  }
}

void UTF32ToUTF8::UTF8Sequence::setRest(int code, int numBytes)
{
  for (int i = 0; i < numBytes; i++) {
    bytes[numBytes - i]->value = 128 | (code & MASKS[5]);
    bytes[numBytes - i]->bits = 6;
    code = code >> 6;
  }
}

wstring UTF32ToUTF8::UTF8Sequence::toString()
{
  shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
  for (int i = 0; i < len; i++) {
    if (i > 0) {
      b->append(L' ');
    }
    b->append(Integer::toBinaryString(bytes[i]->value));
  }
  return b->toString();
}

UTF32ToUTF8::UTF32ToUTF8() {}

void UTF32ToUTF8::convertOneEdge(int start, int end, int startCodePoint,
                                 int endCodePoint)
{
  startUTF8->set(startCodePoint);
  endUTF8->set(endCodePoint);
  build(start, end, startUTF8, endUTF8, 0);
}

void UTF32ToUTF8::build(int start, int end, shared_ptr<UTF8Sequence> startUTF8,
                        shared_ptr<UTF8Sequence> endUTF8, int upto)
{

  // Break into start, middle, end:
  if (startUTF8->byteAt(upto) == endUTF8->byteAt(upto)) {
    // Degen case: lead with the same byte:
    if (upto == startUTF8->len - 1 && upto == endUTF8->len - 1) {
      // Super degen: just single edge, one UTF8 byte:
      utf8->addTransition(start, end, startUTF8->byteAt(upto),
                          endUTF8->byteAt(upto));
      return;
    } else {
      assert(startUTF8->len > upto + 1);
      assert(endUTF8->len > upto + 1);
      int n = utf8->createState();

      // Single value leading edge
      utf8->addTransition(start, n, startUTF8->byteAt(upto));
      // start.addTransition(new Transition(startUTF8.byteAt(upto), n));  //
      // type=single

      // Recurse for the rest
      build(n, end, startUTF8, endUTF8, 1 + upto);
    }
  } else if (startUTF8->len == endUTF8->len) {
    if (upto == startUTF8->len - 1) {
      // start.addTransition(new Transition(startUTF8.byteAt(upto),
      // endUTF8.byteAt(upto), end));        // type=startend
      utf8->addTransition(start, end, startUTF8->byteAt(upto),
                          endUTF8->byteAt(upto));
    } else {
      this->start(start, end, startUTF8, upto, false);
      if (endUTF8->byteAt(upto) - startUTF8->byteAt(upto) > 1) {
        // There is a middle
        all(start, end, startUTF8->byteAt(upto) + 1, endUTF8->byteAt(upto) - 1,
            startUTF8->len - upto - 1);
      }
      this->end(start, end, endUTF8, upto, false);
    }
  } else {

    // start
    this->start(start, end, startUTF8, upto, true);

    // possibly middle, spanning multiple num bytes
    int byteCount = 1 + startUTF8->len - upto;
    constexpr int limit = endUTF8->len - upto;
    while (byteCount < limit) {
      // wasteful: we only need first byte, and, we should
      // statically encode this first byte:
      tmpUTF8a->set(startCodes[byteCount - 1]);
      tmpUTF8b->set(endCodes[byteCount - 1]);
      all(start, end, tmpUTF8a->byteAt(0), tmpUTF8b->byteAt(0),
          tmpUTF8a->len - 1);
      byteCount++;
    }

    // end
    this->end(start, end, endUTF8, upto, true);
  }
}

void UTF32ToUTF8::start(int start, int end, shared_ptr<UTF8Sequence> startUTF8,
                        int upto, bool doAll)
{
  if (upto == startUTF8->len - 1) {
    // Done recursing
    utf8->addTransition(start, end, startUTF8->byteAt(upto),
                        startUTF8->byteAt(upto) |
                            MASKS[startUTF8->numBits(upto) - 1]); // type=start
    // start.addTransition(new Transition(startUTF8.byteAt(upto),
    // startUTF8.byteAt(upto) | MASKS[startUTF8.numBits(upto)-1], end));  //
    // type=start
  } else {
    int n = utf8->createState();
    utf8->addTransition(start, n, startUTF8->byteAt(upto));
    // start.addTransition(new Transition(startUTF8.byteAt(upto), n));  //
    // type=start
    this->start(n, end, startUTF8, 1 + upto, true);
    int endCode = startUTF8->byteAt(upto) | MASKS[startUTF8->numBits(upto) - 1];
    if (doAll && startUTF8->byteAt(upto) != endCode) {
      all(start, end, startUTF8->byteAt(upto) + 1, endCode,
          startUTF8->len - upto - 1);
    }
  }
}

void UTF32ToUTF8::end(int start, int end, shared_ptr<UTF8Sequence> endUTF8,
                      int upto, bool doAll)
{
  if (upto == endUTF8->len - 1) {
    // Done recursing
    // start.addTransition(new Transition(endUTF8.byteAt(upto) &
    // (~MASKS[endUTF8.numBits(upto)-1]), endUTF8.byteAt(upto), end));   //
    // type=end
    utf8->addTransition(start, end,
                        endUTF8->byteAt(upto) &
                            (~MASKS[endUTF8->numBits(upto) - 1]),
                        endUTF8->byteAt(upto));
  } else {
    constexpr int startCode;
    if (endUTF8->numBits(upto) == 5) {
      // special case -- avoid created unused edges (endUTF8
      // doesn't accept certain byte sequences) -- there
      // are other cases we could optimize too:
      startCode = 194;
    } else {
      startCode = endUTF8->byteAt(upto) & (~MASKS[endUTF8->numBits(upto) - 1]);
    }
    if (doAll && endUTF8->byteAt(upto) != startCode) {
      all(start, end, startCode, endUTF8->byteAt(upto) - 1,
          endUTF8->len - upto - 1);
    }
    int n = utf8->createState();
    // start.addTransition(new Transition(endUTF8.byteAt(upto), n));  //
    // type=end
    utf8->addTransition(start, n, endUTF8->byteAt(upto));
    this->end(n, end, endUTF8, 1 + upto, true);
  }
}

void UTF32ToUTF8::all(int start, int end, int startCode, int endCode, int left)
{
  if (left == 0) {
    // start.addTransition(new Transition(startCode, endCode, end));  //
    // type=all
    utf8->addTransition(start, end, startCode, endCode);
  } else {
    int lastN = utf8->createState();
    // start.addTransition(new Transition(startCode, endCode, lastN));  //
    // type=all
    utf8->addTransition(start, lastN, startCode, endCode);
    while (left > 1) {
      int n = utf8->createState();
      // lastN.addTransition(new Transition(128, 191, n));  // type=all*
      utf8->addTransition(lastN, n, 128, 191); // type=all*
      left--;
      lastN = n;
    }
    // lastN.addTransition(new Transition(128, 191, end)); // type = all*
    utf8->addTransition(lastN, end, 128, 191); // type = all*
  }
}

shared_ptr<Automaton> UTF32ToUTF8::convert(shared_ptr<Automaton> utf32)
{
  if (utf32->getNumStates() == 0) {
    return utf32;
  }

  std::deque<int> map_obj(utf32->getNumStates());
  Arrays::fill(map_obj, -1);

  deque<int> pending = deque<int>();
  int utf32State = 0;
  pending.push_back(utf32State);
  utf8 = make_shared<Automaton::Builder>();

  int utf8State = utf8->createState();

  utf8->setAccept(utf8State, utf32->isAccept(utf32State));

  map_obj[utf32State] = utf8State;

  shared_ptr<Transition> scratch = make_shared<Transition>();

  while (pending.size() != 0) {
    utf32State = pending.pop_back();
    utf8State = map_obj[utf32State];
    assert(utf8State != -1);

    int numTransitions = utf32->getNumTransitions(utf32State);
    utf32->initTransition(utf32State, scratch);
    for (int i = 0; i < numTransitions; i++) {
      utf32->getNextTransition(scratch);
      int destUTF32 = scratch->dest;
      int destUTF8 = map_obj[destUTF32];
      if (destUTF8 == -1) {
        destUTF8 = utf8->createState();
        utf8->setAccept(destUTF8, utf32->isAccept(destUTF32));
        map_obj[destUTF32] = destUTF8;
        pending.push_back(destUTF32);
      }

      // Writes new transitions into pendingTransitions:
      convertOneEdge(utf8State, destUTF8, scratch->min, scratch->max);
    }
  }

  return utf8->finish();
}
} // namespace org::apache::lucene::util::automaton