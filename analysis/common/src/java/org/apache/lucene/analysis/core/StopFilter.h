#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/CharArraySet.h"

#include  "core/src/java/org/apache/lucene/analysis/StopFilter.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"

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
namespace org::apache::lucene::analysis::core
{

using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using TokenStream = org::apache::lucene::analysis::TokenStream;

/**
 * Removes stop words from a token stream.
 * <p>
 * This class moved to Lucene Core, but a reference in the {@code
 * analysis/common} module is preserved for documentation purposes and
 * consistency with filter factory.
 * @see org.apache.lucene.analysis.StopFilter
 * @see StopFilterFactory
 */
class StopFilter final : public org::apache::lucene::analysis::StopFilter
{
  GET_CLASS_NAME(StopFilter)

  /**
   * Constructs a filter which removes words from the input TokenStream that are
   * named in the Set.
   *
   * @param in
   *          Input stream
   * @param stopWords
   *          A {@link CharArraySet} representing the stopwords.
   * @see #makeStopSet(java.lang.std::wstring...)
   */
public:
  StopFilter(std::shared_ptr<TokenStream> in_,
             std::shared_ptr<CharArraySet> stopWords);

protected:
  std::shared_ptr<StopFilter> shared_from_this()
  {
    return std::static_pointer_cast<StopFilter>(
        org.apache.lucene.analysis.StopFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/core/
