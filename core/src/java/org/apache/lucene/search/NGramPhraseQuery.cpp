using namespace std;

#include "NGramPhraseQuery.h"

namespace org::apache::lucene::search
{
using IndexReader = org::apache::lucene::index::IndexReader;
using Term = org::apache::lucene::index::Term;

NGramPhraseQuery::NGramPhraseQuery(int n, shared_ptr<PhraseQuery> query)
    : Query(), n(n), phraseQuery(Objects::requireNonNull(query))
{
}

shared_ptr<Query>
NGramPhraseQuery::rewrite(shared_ptr<IndexReader> reader) 
{
  std::deque<std::shared_ptr<Term>> terms = phraseQuery->getTerms();
  const std::deque<int> positions = phraseQuery->getPositions();

  bool isOptimizable = phraseQuery->getSlop() == 0 && n >= 2 &&
                       terms.size() >= 3; // short ones can't be optimized

  if (isOptimizable) {
    for (int i = 1; i < positions.size(); ++i) {
      if (positions[i] != positions[i - 1] + 1) {
        isOptimizable = false;
        break;
      }
    }
  }

  if (isOptimizable == false) {
    return phraseQuery->rewrite(reader);
  }

  shared_ptr<PhraseQuery::Builder> builder =
      make_shared<PhraseQuery::Builder>();
  for (int i = 0; i < terms.size(); ++i) {
    if (i % n == 0 || i == terms.size() - 1) {
      builder->add(terms[i], i);
    }
  }
  return builder->build();
}

bool NGramPhraseQuery::equals(any other)
{
  return sameClassAs(other) && equalsTo(getClass().cast(other));
}

bool NGramPhraseQuery::equalsTo(shared_ptr<NGramPhraseQuery> other)
{
  return n == other->n && phraseQuery->equals(other->phraseQuery);
}

int NGramPhraseQuery::hashCode()
{
  int h = classHash();
  h = 31 * h + phraseQuery->hashCode();
  h = 31 * h + n;
  return h;
}

int NGramPhraseQuery::getN() { return n; }

std::deque<std::shared_ptr<Term>> NGramPhraseQuery::getTerms()
{
  return phraseQuery->getTerms();
}

std::deque<int> NGramPhraseQuery::getPositions()
{
  return phraseQuery->getPositions();
}

wstring NGramPhraseQuery::toString(const wstring &field)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return phraseQuery->toString(field);
}
} // namespace org::apache::lucene::search