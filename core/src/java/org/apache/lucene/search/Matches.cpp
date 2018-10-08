using namespace std;

#include "Matches.h"

namespace org::apache::lucene::search
{

const shared_ptr<Matches> Matches::MATCH_WITH_NO_TERMS =
    make_shared<MatchesAnonymousInnerClass>();

MatchesAnonymousInnerClass::MatchesAnonymousInnerClass() {}

shared_ptr<Iterator<wstring>> MatchesAnonymousInnerClass::iterator()
{
  return Collections::emptyIterator();
}

shared_ptr<MatchesIterator>
MatchesAnonymousInnerClass::getMatches(const wstring &field)
{
  return nullptr;
}
} // namespace org::apache::lucene::search