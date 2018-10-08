using namespace std;

#include "DelimitedPayloadTokenFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../util/ResourceLoader.h"
#include "DelimitedPayloadTokenFilter.h"
#include "FloatEncoder.h"
#include "IdentityEncoder.h"
#include "IntegerEncoder.h"
#include "PayloadEncoder.h"

namespace org::apache::lucene::analysis::payloads
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
const wstring DelimitedPayloadTokenFilterFactory::ENCODER_ATTR = L"encoder";
const wstring DelimitedPayloadTokenFilterFactory::DELIMITER_ATTR = L"delimiter";

DelimitedPayloadTokenFilterFactory::DelimitedPayloadTokenFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args),
      encoderClass(require(args, ENCODER_ATTR)),
      delimiter(getChar(args, DELIMITER_ATTR, L'|'))
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<DelimitedPayloadTokenFilter>
DelimitedPayloadTokenFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<DelimitedPayloadTokenFilter>(input, delimiter, encoder);
}

void DelimitedPayloadTokenFilterFactory::inform(
    shared_ptr<ResourceLoader> loader)
{
  if (encoderClass == L"float") {
    encoder = make_shared<FloatEncoder>();
  } else if (encoderClass == L"integer") {
    encoder = make_shared<IntegerEncoder>();
  } else if (encoderClass == L"identity") {
    encoder = make_shared<IdentityEncoder>();
  } else {
    encoder = loader->newInstance(encoderClass, PayloadEncoder::typeid);
  }
}
} // namespace org::apache::lucene::analysis::payloads