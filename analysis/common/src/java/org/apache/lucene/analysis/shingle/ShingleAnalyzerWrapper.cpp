using namespace std;

#include "ShingleAnalyzerWrapper.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "ShingleFilter.h"

namespace org::apache::lucene::analysis::shingle
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using AnalyzerWrapper = org::apache::lucene::analysis::AnalyzerWrapper;
using StandardAnalyzer =
    org::apache::lucene::analysis::standard::StandardAnalyzer;

ShingleAnalyzerWrapper::ShingleAnalyzerWrapper(
    shared_ptr<Analyzer> defaultAnalyzer)
    : ShingleAnalyzerWrapper(defaultAnalyzer,
                             ShingleFilter::DEFAULT_MAX_SHINGLE_SIZE)
{
}

ShingleAnalyzerWrapper::ShingleAnalyzerWrapper(
    shared_ptr<Analyzer> defaultAnalyzer, int maxShingleSize)
    : ShingleAnalyzerWrapper(defaultAnalyzer,
                             ShingleFilter::DEFAULT_MIN_SHINGLE_SIZE,
                             maxShingleSize)
{
}

ShingleAnalyzerWrapper::ShingleAnalyzerWrapper(
    shared_ptr<Analyzer> defaultAnalyzer, int minShingleSize,
    int maxShingleSize)
    : ShingleAnalyzerWrapper(defaultAnalyzer, minShingleSize, maxShingleSize,
                             ShingleFilter::DEFAULT_TOKEN_SEPARATOR, true,
                             false, ShingleFilter::DEFAULT_FILLER_TOKEN)
{
}

ShingleAnalyzerWrapper::ShingleAnalyzerWrapper(
    shared_ptr<Analyzer> delegate_, int minShingleSize, int maxShingleSize,
    const wstring &tokenSeparator, bool outputUnigrams,
    bool outputUnigramsIfNoShingles, const wstring &fillerToken)
    : org::apache::lucene::analysis::AnalyzerWrapper(
          delegate_->getReuseStrategy()),
      delegate_(delegate_), maxShingleSize(maxShingleSize),
      minShingleSize(minShingleSize),
      tokenSeparator((tokenSeparator == L"" ? L"" : tokenSeparator)),
      outputUnigrams(outputUnigrams),
      outputUnigramsIfNoShingles(outputUnigramsIfNoShingles),
      fillerToken(fillerToken)
{

  if (maxShingleSize < 2) {
    throw invalid_argument(L"Max shingle size must be >= 2");
  }

  if (minShingleSize < 2) {
    throw invalid_argument(L"Min shingle size must be >= 2");
  }
  if (minShingleSize > maxShingleSize) {
    throw invalid_argument(L"Min shingle size must be <= max shingle size");
  }
}

ShingleAnalyzerWrapper::ShingleAnalyzerWrapper()
    : ShingleAnalyzerWrapper(ShingleFilter::DEFAULT_MIN_SHINGLE_SIZE,
                             ShingleFilter::DEFAULT_MAX_SHINGLE_SIZE)
{
}

ShingleAnalyzerWrapper::ShingleAnalyzerWrapper(int minShingleSize,
                                               int maxShingleSize)
    : ShingleAnalyzerWrapper(new StandardAnalyzer(), minShingleSize,
                             maxShingleSize)
{
}

int ShingleAnalyzerWrapper::getMaxShingleSize() { return maxShingleSize; }

int ShingleAnalyzerWrapper::getMinShingleSize() { return minShingleSize; }

wstring ShingleAnalyzerWrapper::getTokenSeparator() { return tokenSeparator; }

bool ShingleAnalyzerWrapper::isOutputUnigrams() { return outputUnigrams; }

bool ShingleAnalyzerWrapper::isOutputUnigramsIfNoShingles()
{
  return outputUnigramsIfNoShingles;
}

wstring ShingleAnalyzerWrapper::getFillerToken() { return fillerToken; }

shared_ptr<Analyzer>
ShingleAnalyzerWrapper::getWrappedAnalyzer(const wstring &fieldName)
{
  return delegate_;
}

shared_ptr<Analyzer::TokenStreamComponents>
ShingleAnalyzerWrapper::wrapComponents(
    const wstring &fieldName,
    shared_ptr<Analyzer::TokenStreamComponents> components)
{
  shared_ptr<ShingleFilter> filter = make_shared<ShingleFilter>(
      components->getTokenStream(), minShingleSize, maxShingleSize);
  filter->setMinShingleSize(minShingleSize);
  filter->setMaxShingleSize(maxShingleSize);
  filter->setTokenSeparator(tokenSeparator);
  filter->setOutputUnigrams(outputUnigrams);
  filter->setOutputUnigramsIfNoShingles(outputUnigramsIfNoShingles);
  filter->setFillerToken(fillerToken);
  return make_shared<Analyzer::TokenStreamComponents>(
      components->getTokenizer(), filter);
}
} // namespace org::apache::lucene::analysis::shingle