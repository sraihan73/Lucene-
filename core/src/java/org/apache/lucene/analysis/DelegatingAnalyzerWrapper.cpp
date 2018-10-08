using namespace std;

#include "DelegatingAnalyzerWrapper.h"
#include "Analyzer.h"
#include "TokenStream.h"

namespace org::apache::lucene::analysis
{

DelegatingAnalyzerWrapper::DelegatingAnalyzerWrapper(
    shared_ptr<ReuseStrategy> fallbackStrategy)
    : AnalyzerWrapper(new DelegatingReuseStrategy(fallbackStrategy))
{
  // häckidy-hick-hack, because we cannot call super() with a reference to
  // "this":
  // C++ TODO: You cannot use 'shared_from_this' in a constructor:
  (std::static_pointer_cast<DelegatingReuseStrategy>(getReuseStrategy()))
      ->wrapper = shared_from_this();
}

shared_ptr<TokenStreamComponents> DelegatingAnalyzerWrapper::wrapComponents(
    const wstring &fieldName, shared_ptr<TokenStreamComponents> components)
{
  return AnalyzerWrapper::wrapComponents(fieldName, components);
}

shared_ptr<TokenStream>
DelegatingAnalyzerWrapper::wrapTokenStreamForNormalization(
    const wstring &fieldName, shared_ptr<TokenStream> in_)
{
  return AnalyzerWrapper::wrapTokenStreamForNormalization(fieldName, in_);
}

shared_ptr<Reader>
DelegatingAnalyzerWrapper::wrapReader(const wstring &fieldName,
                                      shared_ptr<Reader> reader)
{
  return AnalyzerWrapper::wrapReader(fieldName, reader);
}

shared_ptr<Reader>
DelegatingAnalyzerWrapper::wrapReaderForNormalization(const wstring &fieldName,
                                                      shared_ptr<Reader> reader)
{
  return AnalyzerWrapper::wrapReaderForNormalization(fieldName, reader);
}

DelegatingAnalyzerWrapper::DelegatingReuseStrategy::DelegatingReuseStrategy(
    shared_ptr<ReuseStrategy> fallbackStrategy)
    : fallbackStrategy(fallbackStrategy)
{
}

shared_ptr<TokenStreamComponents>
DelegatingAnalyzerWrapper::DelegatingReuseStrategy::getReusableComponents(
    shared_ptr<Analyzer> analyzer, const wstring &fieldName)
{
  if (analyzer == wrapper) {
    shared_ptr<Analyzer> *const wrappedAnalyzer =
        wrapper->getWrappedAnalyzer(fieldName);
    return wrappedAnalyzer->getReuseStrategy()->getReusableComponents(
        wrappedAnalyzer, fieldName);
  } else {
    return fallbackStrategy->getReusableComponents(analyzer, fieldName);
  }
}

void DelegatingAnalyzerWrapper::DelegatingReuseStrategy::setReusableComponents(
    shared_ptr<Analyzer> analyzer, const wstring &fieldName,
    shared_ptr<TokenStreamComponents> components)
{
  if (analyzer == wrapper) {
    shared_ptr<Analyzer> *const wrappedAnalyzer =
        wrapper->getWrappedAnalyzer(fieldName);
    wrappedAnalyzer->getReuseStrategy()->setReusableComponents(
        wrappedAnalyzer, fieldName, components);
  } else {
    fallbackStrategy->setReusableComponents(analyzer, fieldName, components);
  }
}
} // namespace org::apache::lucene::analysis