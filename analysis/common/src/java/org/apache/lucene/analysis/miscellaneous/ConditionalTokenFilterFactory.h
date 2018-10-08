#pragma once
#include "../util/ResourceLoaderAware.h"
#include "../util/TokenFilterFactory.h"
#include "stringhelper.h"
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::util
{
class TokenFilterFactory;
}

namespace org::apache::lucene::analysis
{
class TokenStream;
}
namespace org::apache::lucene::analysis::util
{
class ResourceLoader;
}
namespace org::apache::lucene::analysis::miscellaneous
{
class ConditionalTokenFilter;
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

namespace org::apache::lucene::analysis::miscellaneous
{

using TokenStream = org::apache::lucene::analysis::TokenStream;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

/**
 * Abstract parent class for analysis factories that create {@link
 * ConditionalTokenFilter} instances
 */
class ConditionalTokenFilterFactory : public TokenFilterFactory,
                                      public ResourceLoaderAware
{
  GET_CLASS_NAME(ConditionalTokenFilterFactory)

private:
  std::deque<std::shared_ptr<TokenFilterFactory>> innerFilters;

protected:
  ConditionalTokenFilterFactory(
      std::unordered_map<std::wstring, std::wstring> &args);

  /**
   * Set the inner filter factories to produce the {@link TokenFilter}s that
   * will be wrapped by the {@link ConditionalTokenFilter}
   */
public:
  virtual void setInnerFilters(
      std::deque<std::shared_ptr<TokenFilterFactory>> &innerFilters);

  std::shared_ptr<TokenStream>
  create(std::shared_ptr<TokenStream> input) override;

  void
  inform(std::shared_ptr<ResourceLoader> loader)  override;

  /**
   * Initialises this component with the corresponding {@link ResourceLoader}
   */
protected:
  virtual void
  doInform(std::shared_ptr<ResourceLoader> loader) ;

  /**
   * Modify the incoming {@link TokenStream} with a {@link
   * ConditionalTokenFilter}
   */
  virtual std::shared_ptr<ConditionalTokenFilter>
  create(std::shared_ptr<TokenStream> input,
         std::function<TokenStream *(TokenStream *)> &inner) = 0;

protected:
  std::shared_ptr<ConditionalTokenFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<ConditionalTokenFilterFactory>(
        org.apache.lucene.analysis.util.TokenFilterFactory::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::miscellaneous
