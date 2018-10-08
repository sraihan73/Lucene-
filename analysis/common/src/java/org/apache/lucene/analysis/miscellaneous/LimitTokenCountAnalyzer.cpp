using namespace std;

#include "LimitTokenCountAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "LimitTokenCountFilter.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using AnalyzerWrapper = org::apache::lucene::analysis::AnalyzerWrapper;

LimitTokenCountAnalyzer::LimitTokenCountAnalyzer(shared_ptr<Analyzer> delegate_,
                                                 int maxTokenCount)
    : LimitTokenCountAnalyzer(delegate_, maxTokenCount, false)
{
}

LimitTokenCountAnalyzer::LimitTokenCountAnalyzer(shared_ptr<Analyzer> delegate_,
                                                 int maxTokenCount,
                                                 bool consumeAllTokens)
    : org::apache::lucene::analysis::AnalyzerWrapper(
          delegate_->getReuseStrategy()),
      delegate_(delegate_), maxTokenCount(maxTokenCount),
      consumeAllTokens(consumeAllTokens)
{
}

shared_ptr<Analyzer>
LimitTokenCountAnalyzer::getWrappedAnalyzer(const wstring &fieldName)
{
  return delegate_;
}

shared_ptr<Analyzer::TokenStreamComponents>
LimitTokenCountAnalyzer::wrapComponents(
    const wstring &fieldName,
    shared_ptr<Analyzer::TokenStreamComponents> components)
{
  return make_shared<Analyzer::TokenStreamComponents>(
      components->getTokenizer(),
      make_shared<LimitTokenCountFilter>(components->getTokenStream(),
                                         maxTokenCount, consumeAllTokens));
}

wstring LimitTokenCountAnalyzer::toString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return L"LimitTokenCountAnalyzer(" + delegate_->toString() +
         L", maxTokenCount=" + to_wstring(maxTokenCount) +
         L", consumeAllTokens=" + StringHelper::toString(consumeAllTokens) +
         L")";
}
} // namespace org::apache::lucene::analysis::miscellaneous