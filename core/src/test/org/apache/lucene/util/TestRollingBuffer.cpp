using namespace std;

#include "TestRollingBuffer.h"

namespace org::apache::lucene::util
{

void TestRollingBuffer::Position::reset() { pos = -1; }

void TestRollingBuffer::test()
{

  shared_ptr<RollingBuffer<std::shared_ptr<Position>>> *const buffer =
      make_shared<RollingBufferAnonymousInnerClass>(shared_from_this());

  for (int iter = 0; iter < 100 * RANDOM_MULTIPLIER; iter++) {

    int freeBeforePos = 0;
    constexpr int maxPos = atLeast(10000);
    shared_ptr<FixedBitSet> *const posSet =
        make_shared<FixedBitSet>(maxPos + 1000);
    int posUpto = 0;
    shared_ptr<Random> random = TestRollingBuffer::random();
    while (freeBeforePos < maxPos) {
      if (random->nextInt(4) == 1) {
        constexpr int limit = rarely() ? 1000 : 20;
        constexpr int inc = random->nextInt(limit);
        constexpr int pos = freeBeforePos + inc;
        posUpto = max(posUpto, pos);
        if (VERBOSE) {
          wcout << L"  check pos=" << pos << L" posUpto=" << posUpto << endl;
        }
        shared_ptr<Position> *const posData = buffer->get(pos);
        if (!posSet->getAndSet(pos)) {
          assertEquals(-1, posData->pos);
          posData->pos = pos;
        } else {
          assertEquals(pos, posData->pos);
        }
      } else {
        if (posUpto > freeBeforePos) {
          freeBeforePos += random->nextInt(posUpto - freeBeforePos);
        }
        if (VERBOSE) {
          wcout << L"  freeBeforePos=" << freeBeforePos << endl;
        }
        buffer->freeBefore(freeBeforePos);
      }
    }

    buffer->reset();
  }
}

TestRollingBuffer::RollingBufferAnonymousInnerClass::
    RollingBufferAnonymousInnerClass(
        shared_ptr<TestRollingBuffer> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Position>
TestRollingBuffer::RollingBufferAnonymousInnerClass::newInstance()
{
  shared_ptr<Position> *const pos = make_shared<Position>();
  pos->pos = -1;
  return pos;
}
} // namespace org::apache::lucene::util