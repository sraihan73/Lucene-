using namespace std;

#include "CharFilterFactory.h"

namespace org::apache::lucene::analysis::util
{
using CharFilter = org::apache::lucene::analysis::CharFilter;
const shared_ptr<AnalysisSPILoader<std::shared_ptr<CharFilterFactory>>>
    CharFilterFactory::loader =
        make_shared<AnalysisSPILoader<std::shared_ptr<CharFilterFactory>>>(
            CharFilterFactory::typeid);

shared_ptr<CharFilterFactory>
CharFilterFactory::forName(const wstring &name,
                           unordered_map<wstring, wstring> &args)
{
  return loader->newInstance(name, args);
}

type_info CharFilterFactory::lookupClass(const wstring &name)
{
  return loader->lookupClass(name);
}

shared_ptr<Set<wstring>> CharFilterFactory::availableCharFilters()
{
  return loader->availableServices();
}

void CharFilterFactory::reloadCharFilters(shared_ptr<ClassLoader> classloader)
{
  loader->reload(classloader);
}

CharFilterFactory::CharFilterFactory(unordered_map<wstring, wstring> &args)
    : AbstractAnalysisFactory(args)
{
}
} // namespace org::apache::lucene::analysis::util