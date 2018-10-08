using namespace std;

#include "AnalyzerWrapper.h"
#include "../util/AttributeFactory.h"
#include "TokenStream.h"

namespace org::apache::lucene::analysis
{
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

AnalyzerWrapper::AnalyzerWrapper(shared_ptr<ReuseStrategy> reuseStrategy)
    : Analyzer(reuseStrategy)
{
}

shared_ptr<TokenStreamComponents>
AnalyzerWrapper::wrapComponents(const wstring &fieldName,
                                shared_ptr<TokenStreamComponents> components)
{
  return components;
}

shared_ptr<TokenStream>
AnalyzerWrapper::wrapTokenStreamForNormalization(const wstring &fieldName,
                                                 shared_ptr<TokenStream> in_)
{
  return in_;
}

shared_ptr<Reader> AnalyzerWrapper::wrapReader(const wstring &fieldName,
                                               shared_ptr<Reader> reader)
{
  return reader;
}

shared_ptr<Reader>
AnalyzerWrapper::wrapReaderForNormalization(const wstring &fieldName,
                                            shared_ptr<Reader> reader)
{
  return reader;
}

shared_ptr<TokenStreamComponents>
AnalyzerWrapper::createComponents(const wstring &fieldName)
{
  return wrapComponents(
      fieldName, getWrappedAnalyzer(fieldName)->createComponents(fieldName));
}

shared_ptr<TokenStream> AnalyzerWrapper::normalize(const wstring &fieldName,
                                                   shared_ptr<TokenStream> in_)
{
  return wrapTokenStreamForNormalization(
      fieldName, getWrappedAnalyzer(fieldName)->normalize(fieldName, in_));
}

int AnalyzerWrapper::getPositionIncrementGap(const wstring &fieldName)
{
  return getWrappedAnalyzer(fieldName)->getPositionIncrementGap(fieldName);
}

int AnalyzerWrapper::getOffsetGap(const wstring &fieldName)
{
  return getWrappedAnalyzer(fieldName)->getOffsetGap(fieldName);
}

shared_ptr<Reader> AnalyzerWrapper::initReader(const wstring &fieldName,
                                               shared_ptr<Reader> reader)
{
  return getWrappedAnalyzer(fieldName)->initReader(
      fieldName, wrapReader(fieldName, reader));
}

shared_ptr<Reader>
AnalyzerWrapper::initReaderForNormalization(const wstring &fieldName,
                                            shared_ptr<Reader> reader)
{
  return getWrappedAnalyzer(fieldName)->initReaderForNormalization(
      fieldName, wrapReaderForNormalization(fieldName, reader));
}

shared_ptr<AttributeFactory>
AnalyzerWrapper::attributeFactory(const wstring &fieldName)
{
  return getWrappedAnalyzer(fieldName)->attributeFactory(fieldName);
}
} // namespace org::apache::lucene::analysis