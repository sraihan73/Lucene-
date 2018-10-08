using namespace std;

#include "CrankyTokenFilter.h"

namespace org::apache::lucene::analysis
{

CrankyTokenFilter::CrankyTokenFilter(shared_ptr<TokenStream> input,
                                     shared_ptr<Random> random)
    : TokenFilter(input), random(random)
{
}

bool CrankyTokenFilter::incrementToken() 
{
  if (thingToDo == 0 && random->nextBoolean()) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(
        L"Fake IOException from TokenStream.incrementToken()");
  }
  return input->incrementToken();
}

void CrankyTokenFilter::end() 
{
  TokenFilter::end();
  if (thingToDo == 1 && random->nextBoolean()) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(L"Fake IOException from TokenStream.end()");
  }
}

void CrankyTokenFilter::reset() 
{
  TokenFilter::reset();
  thingToDo = random->nextInt(100);
  if (thingToDo == 2 && random->nextBoolean()) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(L"Fake IOException from TokenStream.reset()");
  }
}

CrankyTokenFilter::~CrankyTokenFilter()
{
  // C++ NOTE: There is no explicit call to the base class destructor in C++:
  //      super.close();
  if (thingToDo == 3 && random->nextBoolean()) {
    // C++ TODO: The following line could not be converted:
    throw java.io.IOException(L"Fake IOException from TokenStream.close()");
  }
}
} // namespace org::apache::lucene::analysis