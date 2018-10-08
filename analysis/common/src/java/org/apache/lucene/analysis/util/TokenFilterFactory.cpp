using namespace std;

#include "TokenFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"

namespace org::apache::lucene::analysis::util
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
const shared_ptr<AnalysisSPILoader<std::shared_ptr<TokenFilterFactory>>>
    TokenFilterFactory::loader =
        make_shared<AnalysisSPILoader<std::shared_ptr<TokenFilterFactory>>>(
            TokenFilterFactory::typeid,
            std::deque<wstring>{L"TokenFilterFactory", L"FilterFactory"});

shared_ptr<TokenFilterFactory>
TokenFilterFactory::forName(const wstring &name,
                            unordered_map<wstring, wstring> &args)
{
  return loader->newInstance(name, args);
}

type_info TokenFilterFactory::lookupClass(const wstring &name)
{
  return loader->lookupClass(name);
}

shared_ptr<Set<wstring>> TokenFilterFactory::availableTokenFilters()
{
  return loader->availableServices();
}

void TokenFilterFactory::reloadTokenFilters(shared_ptr<ClassLoader> classloader)
{
  loader->reload(classloader);
}

TokenFilterFactory::TokenFilterFactory(unordered_map<wstring, wstring> &args)
    : AbstractAnalysisFactory(args)
{
}
} // namespace org::apache::lucene::analysis::util