#pragma once
#include "../util/TokenFilterFactory.h"
#include "MinHashFilter.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class TokenStream;
}

/*
 * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * sraihan.com licenses this file to You under GPLv3 License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

namespace org::apache::lucene::analysis::minhash
{

using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

/**
 * {@link TokenFilterFactory} for {@link MinHashFilter}.
 * @since 6.2.0
 */
class MinHashFilterFactory : public TokenFilterFactory
{
  GET_CLASS_NAME(MinHashFilterFactory)
private:
  int hashCount = MinHashFilter::DEFAULT_HASH_COUNT;

  int bucketCount = MinHashFilter::DEFAULT_BUCKET_COUNT;

  int hashSetSize = MinHashFilter::DEFAULT_HASH_SET_SIZE;

  bool withRotation = false;

  /**
   * Create a {@link MinHashFilterFactory}.
   */
public:
  MinHashFilterFactory(std::unordered_map<std::wstring, std::wstring> &args);

  /*
   * (non-Javadoc)
   *
   * @see
   * org.apache.lucene.analysis.util.TokenFilterFactory#create(org.apache.lucene.analysis.TokenStream)
   */
  std::shared_ptr<TokenStream>
  create(std::shared_ptr<TokenStream> input) override;

protected:
  std::shared_ptr<MinHashFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<MinHashFilterFactory>(
        org.apache.lucene.analysis.util.TokenFilterFactory::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::minhash
