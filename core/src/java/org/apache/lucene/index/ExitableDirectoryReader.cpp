using namespace std;

#include "ExitableDirectoryReader.h"

namespace org::apache::lucene::index
{
using FilterTerms = org::apache::lucene::index::FilterLeafReader::FilterTerms;
using FilterTermsEnum =
    org::apache::lucene::index::FilterLeafReader::FilterTermsEnum;
using BytesRef = org::apache::lucene::util::BytesRef;
using CompiledAutomaton =
    org::apache::lucene::util::automaton::CompiledAutomaton;

ExitableDirectoryReader::ExitingReaderException::ExitingReaderException(
    const wstring &msg)
    : RuntimeException(msg)
{
}

ExitableDirectoryReader::ExitableSubReaderWrapper::ExitableSubReaderWrapper(
    shared_ptr<QueryTimeout> queryTimeout)
{
  this->queryTimeout = queryTimeout;
}

shared_ptr<LeafReader> ExitableDirectoryReader::ExitableSubReaderWrapper::wrap(
    shared_ptr<LeafReader> reader)
{
  return make_shared<ExitableFilterAtomicReader>(reader, queryTimeout);
}

ExitableDirectoryReader::ExitableFilterAtomicReader::ExitableFilterAtomicReader(
    shared_ptr<LeafReader> in_, shared_ptr<QueryTimeout> queryTimeout)
    : FilterLeafReader(in_)
{
  this->queryTimeout = queryTimeout;
}

shared_ptr<Terms> ExitableDirectoryReader::ExitableFilterAtomicReader::terms(
    const wstring &field) 
{
  shared_ptr<Terms> terms = in_->terms(field);
  if (terms == nullptr) {
    return nullptr;
  }
  return (queryTimeout->isTimeoutEnabled())
             ? make_shared<ExitableTerms>(terms, queryTimeout)
             : terms;
}

shared_ptr<CacheHelper>
ExitableDirectoryReader::ExitableFilterAtomicReader::getReaderCacheHelper()
{
  return in_->getReaderCacheHelper();
}

shared_ptr<CacheHelper>
ExitableDirectoryReader::ExitableFilterAtomicReader::getCoreCacheHelper()
{
  return in_->getCoreCacheHelper();
}

ExitableDirectoryReader::ExitableTerms::ExitableTerms(
    shared_ptr<Terms> terms, shared_ptr<QueryTimeout> queryTimeout)
    : org::apache::lucene::index::FilterLeafReader::FilterTerms(terms)
{
  this->queryTimeout = queryTimeout;
}

shared_ptr<TermsEnum> ExitableDirectoryReader::ExitableTerms::intersect(
    shared_ptr<CompiledAutomaton> compiled,
    shared_ptr<BytesRef> startTerm) 
{
  return make_shared<ExitableTermsEnum>(in_->intersect(compiled, startTerm),
                                        queryTimeout);
}

shared_ptr<TermsEnum>
ExitableDirectoryReader::ExitableTerms::iterator() 
{
  return make_shared<ExitableTermsEnum>(in_->begin(), queryTimeout);
}

ExitableDirectoryReader::ExitableTermsEnum::ExitableTermsEnum(
    shared_ptr<TermsEnum> termsEnum, shared_ptr<QueryTimeout> queryTimeout)
    : org::apache::lucene::index::FilterLeafReader::FilterTermsEnum(termsEnum)
{
  this->queryTimeout = queryTimeout;
  checkAndThrow();
}

void ExitableDirectoryReader::ExitableTermsEnum::checkAndThrow()
{
  if (queryTimeout->shouldExit()) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    throw make_shared<ExitingReaderException>(
        L"The request took too long to iterate over terms. Timeout: " +
        queryTimeout->toString() + L", TermsEnum=" + in_);
  } else if (Thread::interrupted()) {
    throw make_shared<ExitingReaderException>(
        L"Interrupted while iterating over terms. TermsEnum=" + in_);
  }
}

shared_ptr<BytesRef>
ExitableDirectoryReader::ExitableTermsEnum::next() 
{
  // Before every iteration, check if the iteration should exit
  checkAndThrow();
  return in_->next();
}

ExitableDirectoryReader::ExitableDirectoryReader(
    shared_ptr<DirectoryReader> in_,
    shared_ptr<QueryTimeout> queryTimeout) 
    : FilterDirectoryReader(in_, new ExitableSubReaderWrapper(queryTimeout))
{
  this->queryTimeout = queryTimeout;
}

shared_ptr<DirectoryReader> ExitableDirectoryReader::doWrapDirectoryReader(
    shared_ptr<DirectoryReader> in_) 
{
  return make_shared<ExitableDirectoryReader>(in_, queryTimeout);
}

shared_ptr<DirectoryReader> ExitableDirectoryReader::wrap(
    shared_ptr<DirectoryReader> in_,
    shared_ptr<QueryTimeout> queryTimeout) 
{
  return make_shared<ExitableDirectoryReader>(in_, queryTimeout);
}

shared_ptr<CacheHelper> ExitableDirectoryReader::getReaderCacheHelper()
{
  return in_->getReaderCacheHelper();
}

wstring ExitableDirectoryReader::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"ExitableDirectoryReader(" + in_->toString() + L")";
}
} // namespace org::apache::lucene::index