using namespace std;

#include "JustCompileSearchSpans.h"

namespace org::apache::lucene::search::spans
{
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
const wstring JustCompileSearchSpans::UNSUPPORTED_MSG =
    L"unsupported: used for back-compat testing only !";

int JustCompileSearchSpans::JustCompileSpans::docID()
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}

int JustCompileSearchSpans::JustCompileSpans::nextDoc() 
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}

int JustCompileSearchSpans::JustCompileSpans::advance(int target) throw(
    IOException)
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}

int JustCompileSearchSpans::JustCompileSpans::startPosition()
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}

int JustCompileSearchSpans::JustCompileSpans::endPosition()
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}

int JustCompileSearchSpans::JustCompileSpans::width()
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}

void JustCompileSearchSpans::JustCompileSpans::collect(
    shared_ptr<SpanCollector> collector) 
{
}

int JustCompileSearchSpans::JustCompileSpans::nextStartPosition() throw(
    IOException)
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}

int64_t JustCompileSearchSpans::JustCompileSpans::cost()
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}

float JustCompileSearchSpans::JustCompileSpans::positionsCost()
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}

wstring JustCompileSearchSpans::JustCompileSpanQuery::getField()
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}

shared_ptr<SpanWeight>
JustCompileSearchSpans::JustCompileSpanQuery::createWeight(
    shared_ptr<IndexSearcher> searcher, bool needsScores,
    float boost) 
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}

wstring
JustCompileSearchSpans::JustCompileSpanQuery::toString(const wstring &field)
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}

bool JustCompileSearchSpans::JustCompileSpanQuery::equals(any o)
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}

int JustCompileSearchSpans::JustCompileSpanQuery::hashCode()
{
  throw make_shared<UnsupportedOperationException>(UNSUPPORTED_MSG);
}
} // namespace org::apache::lucene::search::spans