using namespace std;

#include "TokenizerFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"

namespace org::apache::lucene::analysis::util
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
const shared_ptr<AnalysisSPILoader<std::shared_ptr<TokenizerFactory>>>
    TokenizerFactory::loader =
        make_shared<AnalysisSPILoader<std::shared_ptr<TokenizerFactory>>>(
            TokenizerFactory::typeid);

shared_ptr<TokenizerFactory>
TokenizerFactory::forName(const wstring &name,
                          unordered_map<wstring, wstring> &args)
{
  return loader->newInstance(name, args);
}

type_info TokenizerFactory::lookupClass(const wstring &name)
{
  return loader->lookupClass(name);
}

shared_ptr<Set<wstring>> TokenizerFactory::availableTokenizers()
{
  return loader->availableServices();
}

void TokenizerFactory::reloadTokenizers(shared_ptr<ClassLoader> classloader)
{
  loader->reload(classloader);
}

TokenizerFactory::TokenizerFactory(unordered_map<wstring, wstring> &args)
    : AbstractAnalysisFactory(args)
{
}

shared_ptr<Tokenizer> TokenizerFactory::create()
{
  return create(TokenStream::DEFAULT_TOKEN_ATTRIBUTE_FACTORY);
}
} // namespace org::apache::lucene::analysis::util