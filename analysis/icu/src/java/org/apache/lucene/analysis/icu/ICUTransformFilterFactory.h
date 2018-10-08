#pragma once
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/MultiTermAwareComponent.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/TokenFilterFactory.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"

#include  "core/src/java/org/apache/lucene/analysis/util/AbstractAnalysisFactory.h"

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
namespace org::apache::lucene::analysis::icu
{

using TokenStream = org::apache::lucene::analysis::TokenStream;
using AbstractAnalysisFactory =
    org::apache::lucene::analysis::util::AbstractAnalysisFactory; // javadocs
using MultiTermAwareComponent =
    org::apache::lucene::analysis::util::MultiTermAwareComponent;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

using com::ibm::icu::text::Transliterator;

/**
 * Factory for {@link ICUTransformFilter}.
 * <p>
 * Supports the following attributes:
 * <ul>
 *   <li>id (mandatory): A Transliterator ID, one from {@link
 * Transliterator#getAvailableIDs()} <li>direction (optional): Either 'forward'
 * or 'reverse'. Default is forward.
 * </ul>
 * @see Transliterator
 * @since 3.1.0
 */
class ICUTransformFilterFactory : public TokenFilterFactory,
                                  public MultiTermAwareComponent
{
  GET_CLASS_NAME(ICUTransformFilterFactory)
private:
  const std::shared_ptr<Transliterator> transliterator;

  // TODO: add support for custom rules
  /** Creates a new ICUTransformFilterFactory */
public:
  ICUTransformFilterFactory(
      std::unordered_map<std::wstring, std::wstring> &args);

  std::shared_ptr<TokenStream>
  create(std::shared_ptr<TokenStream> input) override;

  std::shared_ptr<AbstractAnalysisFactory> getMultiTermComponent() override;

protected:
  std::shared_ptr<ICUTransformFilterFactory> shared_from_this()
  {
    return std::static_pointer_cast<ICUTransformFilterFactory>(
        org.apache.lucene.analysis.util.TokenFilterFactory::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/icu/
