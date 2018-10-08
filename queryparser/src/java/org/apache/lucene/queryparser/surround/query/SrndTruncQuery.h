#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRef;
}

namespace org::apache::lucene::index
{
class IndexReader;
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
namespace org::apache::lucene::queryparser::surround::query
{
using BytesRef = org::apache::lucene::util::BytesRef;
using IndexReader = org::apache::lucene::index::IndexReader;

/**
 * Query that matches wildcards
 */
class SrndTruncQuery : public SimpleTerm
{
  GET_CLASS_NAME(SrndTruncQuery)
public:
  SrndTruncQuery(const std::wstring &truncated, wchar_t unlimited,
                 wchar_t mask); // not quoted

private:
  const std::wstring truncated;
  const wchar_t unlimited;
  const wchar_t mask;

  std::wstring prefix;
  std::shared_ptr<BytesRef> prefixRef;
  std::shared_ptr<Pattern> pattern;

public:
  virtual std::wstring getTruncated();

  std::wstring toStringUnquoted() override;

protected:
  virtual bool matchingChar(wchar_t c);

  virtual void appendRegExpForChar(wchar_t c,
                                   std::shared_ptr<StringBuilder> re);

  virtual void truncatedToPrefixAndPattern();

public:
  void visitMatchingTerms(
      std::shared_ptr<IndexReader> reader, const std::wstring &fieldName,
      std::shared_ptr<MatchingTermVisitor> mtv)  override;

protected:
  std::shared_ptr<SrndTruncQuery> shared_from_this()
  {
    return std::static_pointer_cast<SrndTruncQuery>(
        SimpleTerm::shared_from_this());
  }
};

} // namespace org::apache::lucene::queryparser::surround::query
