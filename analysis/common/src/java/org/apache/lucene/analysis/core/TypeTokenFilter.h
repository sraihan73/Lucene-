#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/FilteringTokenFilter.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/TypeAttribute.h"

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

using FilteringTokenFilter =
    org::apache::lucene::analysis::FilteringTokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;

/**
 * Removes tokens whose types appear in a set of blocked types from a token
 * stream.
 */
class TypeTokenFilter final : public FilteringTokenFilter
{
  GET_CLASS_NAME(TypeTokenFilter)

private:
  const std::shared_ptr<Set<std::wstring>> stopTypes;
  const std::shared_ptr<TypeAttribute> typeAttribute =
      addAttribute(TypeAttribute::typeid);
  const bool useWhiteList;

  /**
   * Create a new {@link TypeTokenFilter}.
   * @param input        the {@link TokenStream} to consume
   * @param stopTypes    the types to filter
   * @param useWhiteList if true, then tokens whose type is in stopTypes will
   *                     be kept, otherwise they will be filtered out
   */
public:
  TypeTokenFilter(std::shared_ptr<TokenStream> input,
                  std::shared_ptr<Set<std::wstring>> stopTypes,
                  bool useWhiteList);

  /**
   * Create a new {@link TypeTokenFilter} that filters tokens out
   * (useWhiteList=false).
   */
  TypeTokenFilter(std::shared_ptr<TokenStream> input,
                  std::shared_ptr<Set<std::wstring>> stopTypes);

  /**
   * By default accept the token if its type is not a stop type.
   * When the useWhiteList parameter is set to true then accept the token if its
   * type is contained in the stopTypes
   */
protected:
  bool accept() override;

protected:
  std::shared_ptr<TypeTokenFilter> shared_from_this()
  {
    return std::static_pointer_cast<TypeTokenFilter>(
        org.apache.lucene.analysis.FilteringTokenFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/core/
