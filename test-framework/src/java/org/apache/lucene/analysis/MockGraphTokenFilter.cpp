using namespace std;

#include "MockGraphTokenFilter.h"

namespace org::apache::lucene::analysis
{
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using TestUtil = org::apache::lucene::util::TestUtil;
bool MockGraphTokenFilter::DEBUG = false;

MockGraphTokenFilter::MockGraphTokenFilter(shared_ptr<Random> random,
                                           shared_ptr<TokenStream> input)
    : LookaheadTokenFilter<LookaheadTokenFilter::Position>(input),
      seed(random->nextLong())
{
}

shared_ptr<Position> MockGraphTokenFilter::newPosition()
{
  return make_shared<Position>();
}

void MockGraphTokenFilter::afterPosition() 
{
  if (DEBUG) {
    wcout << L"MockGraphTF.afterPos" << endl;
  }
  if (random->nextInt(7) == 5) {

    constexpr int posLength = TestUtil::nextInt(random, 1, 5);

    if (DEBUG) {
      wcout << L"  do insert! posLen=" << posLength << endl;
    }

    shared_ptr<Position> *const posEndData =
        positions->get(outputPos + posLength);

    // Look ahead as needed until we figure out the right
    // endOffset:
    while (!end && posEndData->endOffset == -1 &&
           inputPos <= (outputPos + posLength)) {
      if (!peekToken()) {
        break;
      }
    }

    if (posEndData->endOffset != -1) {
      // Notify super class that we are injecting a token:
      insertToken();
      clearAttributes();
      posLenAtt->setPositionLength(posLength);
      termAtt->append(TestUtil::randomUnicodeString(random));
      posIncAtt->setPositionIncrement(0);
      offsetAtt->setOffset(positions->get(outputPos).startOffset,
                           posEndData->endOffset);
      if (DEBUG) {
        wcout << L"  inject: outputPos=" << outputPos << L" startOffset="
              << offsetAtt->startOffset() << L" endOffset="
              << offsetAtt->endOffset() << L" posLength="
              << posLenAtt->getPositionLength() << endl;
      }
      // TODO: set TypeAtt too?
    } else {
      // Either 1) the tokens ended before our posLength,
      // or 2) our posLength ended inside a hole from the
      // input.  In each case we just skip the inserted
      // token.
    }
  }
}

void MockGraphTokenFilter::reset() 
{
  LookaheadTokenFilter<LookaheadTokenFilter::Position>::reset();
  // NOTE: must be "deterministically random" because
  // BaseTokenStreamTestCase pulls tokens twice on the
  // same input and asserts they are the same:
  this->random = make_shared<Random>(seed);
}

MockGraphTokenFilter::~MockGraphTokenFilter()
{
  // C++ NOTE: There is no explicit call to the base class destructor in C++:
  //      super.close();
  this->random.reset();
}

bool MockGraphTokenFilter::incrementToken() 
{
  if (DEBUG) {
    wcout << L"MockGraphTF.incr inputPos=" << inputPos << L" outputPos="
          << outputPos << endl;
  }
  if (random == nullptr) {
    throw make_shared<IllegalStateException>(
        L"incrementToken called in wrong state!");
  }
  return nextToken();
}
} // namespace org::apache::lucene::analysis