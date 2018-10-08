using namespace std;

#include "InputIterator.h"

namespace org::apache::lucene::search::suggest
{
using LookupResult = org::apache::lucene::search::suggest::Lookup::LookupResult;
using AnalyzingInfixSuggester =
    org::apache::lucene::search::suggest::analyzing::AnalyzingInfixSuggester;
using AnalyzingSuggester =
    org::apache::lucene::search::suggest::analyzing::AnalyzingSuggester;
using FuzzySuggester =
    org::apache::lucene::search::suggest::analyzing::FuzzySuggester;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefIterator = org::apache::lucene::util::BytesRefIterator;
const shared_ptr<InputIterator> InputIterator::EMPTY =
    make_shared<InputIteratorWrapper>(
        org::apache::lucene::util::BytesRefIterator::EMPTY);

InputIteratorWrapper::InputIteratorWrapper(shared_ptr<BytesRefIterator> wrapped)
    : wrapped(wrapped)
{
}

int64_t InputIteratorWrapper::weight() { return 1; }

shared_ptr<BytesRef> InputIteratorWrapper::next() 
{
  return wrapped->next();
}

shared_ptr<BytesRef> InputIteratorWrapper::payload() { return nullptr; }

bool InputIteratorWrapper::hasPayloads() { return false; }

shared_ptr<Set<std::shared_ptr<BytesRef>>> InputIteratorWrapper::contexts()
{
  return nullptr;
}

bool InputIteratorWrapper::hasContexts() { return false; }
} // namespace org::apache::lucene::search::suggest