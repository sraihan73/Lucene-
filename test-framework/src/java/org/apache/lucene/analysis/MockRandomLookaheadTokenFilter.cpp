using namespace std;

#include "MockRandomLookaheadTokenFilter.h"

namespace org::apache::lucene::analysis
{
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

MockRandomLookaheadTokenFilter::MockRandomLookaheadTokenFilter(
    shared_ptr<Random> random, shared_ptr<TokenStream> in_)
    : LookaheadTokenFilter<LookaheadTokenFilter::Position>(in_),
      random(make_shared<Random>(seed)), seed(random->nextLong())
{
}

shared_ptr<Position> MockRandomLookaheadTokenFilter::newPosition()
{
  return make_shared<Position>();
}

void MockRandomLookaheadTokenFilter::afterPosition() 
{
  if (!end && random->nextInt(4) == 2) {
    peekToken();
  }
}

bool MockRandomLookaheadTokenFilter::incrementToken() 
{
  if (DEBUG) {
    wcout << L"\n"
          << Thread::currentThread().getName() << L": MRLTF.incrToken" << endl;
  }

  if (!end) {
    while (true) {
      if (random->nextInt(3) == 1) {
        if (!peekToken()) {
          if (DEBUG) {
            wcout << L"  peek; inputPos=" << inputPos << L" END" << endl;
          }
          break;
        }
        if (DEBUG) {
          wcout << L"  peek; inputPos=" << inputPos << L" token=" << termAtt
                << endl;
        }
      } else {
        if (DEBUG) {
          wcout << L"  done peek" << endl;
        }
        break;
      }
    }
  }

  constexpr bool result = nextToken();
  if (result) {
    if (DEBUG) {
      wcout << L"  return nextToken token=" << termAtt << endl;
    }
  } else {
    if (DEBUG) {
      wcout << L"  return nextToken END" << endl;
    }
  }
  return result;
}

void MockRandomLookaheadTokenFilter::reset() 
{
  LookaheadTokenFilter<LookaheadTokenFilter::Position>::reset();
  random->setSeed(seed);
}
} // namespace org::apache::lucene::analysis