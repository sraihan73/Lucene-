using namespace std;

#include "MinHashFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"

namespace org::apache::lucene::analysis::minhash
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

MinHashFilterFactory::MinHashFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args)
{
  hashCount = getInt(args, L"hashCount", MinHashFilter::DEFAULT_HASH_COUNT);
  bucketCount =
      getInt(args, L"bucketCount", MinHashFilter::DEFAULT_BUCKET_COUNT);
  hashSetSize =
      getInt(args, L"hashSetSize", MinHashFilter::DEFAULT_HASH_SET_SIZE);
  withRotation = getBoolean(args, L"withRotation", bucketCount > 1);
}

shared_ptr<TokenStream>
MinHashFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<MinHashFilter>(input, hashCount, bucketCount, hashSetSize,
                                    withRotation);
}
} // namespace org::apache::lucene::analysis::minhash