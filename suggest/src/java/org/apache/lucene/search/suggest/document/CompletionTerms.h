#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/suggest/document/CompletionsTermsReader.h"

#include  "core/src/java/org/apache/lucene/index/Terms.h"
#include  "core/src/java/org/apache/lucene/search/suggest/document/NRTSuggester.h"

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
namespace org::apache::lucene::search::suggest::document
{

using FilterLeafReader = org::apache::lucene::index::FilterLeafReader;
using Terms = org::apache::lucene::index::Terms;

/**
 * Wrapped {@link org.apache.lucene.index.Terms}
 * used by {@link SuggestField} and {@link ContextSuggestField}
 * to access corresponding suggester and their attributes
 *
 * @lucene.experimental
 */
class CompletionTerms final : public FilterLeafReader::FilterTerms
{
  GET_CLASS_NAME(CompletionTerms)

private:
  const std::shared_ptr<CompletionsTermsReader> reader;

  /**
   * Creates a completionTerms based on {@link CompletionsTermsReader}
   */
public:
  CompletionTerms(std::shared_ptr<Terms> in_,
                  std::shared_ptr<CompletionsTermsReader> reader);

  /**
   * Returns the type of FST, either {@link SuggestField#TYPE} or
   * {@link ContextSuggestField#TYPE}
   */
  char getType();

  /**
   * Returns the minimum weight of all entries in the weighted FST
   */
  int64_t getMinWeight();

  /**
   * Returns the maximum weight of all entries in the weighted FST
   */
  int64_t getMaxWeight();

  /**
   * Returns a {@link NRTSuggester} for the field
   * or <code>null</code> if no FST
   * was indexed for this field
   */
  std::shared_ptr<NRTSuggester> suggester() ;

protected:
  std::shared_ptr<CompletionTerms> shared_from_this()
  {
    return std::static_pointer_cast<CompletionTerms>(
        org.apache.lucene.index.FilterLeafReader
            .FilterTerms::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/suggest/document/
