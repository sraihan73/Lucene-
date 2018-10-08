using namespace std;

#include "AssertingMatches.h"

namespace org::apache::lucene::search
{

AssertingMatches::AssertingMatches(shared_ptr<Matches> matches) : in_(matches)
{
}

shared_ptr<MatchesIterator>
AssertingMatches::getMatches(const wstring &field) 
{
  shared_ptr<MatchesIterator> mi = in_->getMatches(field);
  if (mi == nullptr) {
    return nullptr;
  }
  return make_shared<AssertingMatchesIterator>(mi);
}

shared_ptr<Iterator<wstring>> AssertingMatches::iterator()
{
  return in_->begin();
}
} // namespace org::apache::lucene::search