using namespace std;

#include "DaitchMokotoffSoundexFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "DaitchMokotoffSoundexFilter.h"

namespace org::apache::lucene::analysis::phonetic
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
const wstring DaitchMokotoffSoundexFilterFactory::INJECT = L"inject";

DaitchMokotoffSoundexFilterFactory::DaitchMokotoffSoundexFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args),
      inject(getBoolean(args, INJECT, true))
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<DaitchMokotoffSoundexFilter>
DaitchMokotoffSoundexFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<DaitchMokotoffSoundexFilter>(input, inject);
}
} // namespace org::apache::lucene::analysis::phonetic