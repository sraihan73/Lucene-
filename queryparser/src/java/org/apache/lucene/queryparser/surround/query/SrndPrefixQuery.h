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
class Term;
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
using Term = org::apache::lucene::index::Term;
using BytesRef = org::apache::lucene::util::BytesRef;
using IndexReader = org::apache::lucene::index::IndexReader;

/**
 * Query that matches std::wstring prefixes
 */
class SrndPrefixQuery : public SimpleTerm
{
  GET_CLASS_NAME(SrndPrefixQuery)
private:
  const std::shared_ptr<BytesRef> prefixRef;

public:
  SrndPrefixQuery(const std::wstring &prefix, bool quoted, wchar_t truncator);

private:
  const std::wstring prefix;

public:
  virtual std::wstring getPrefix();

private:
  const wchar_t truncator;

public:
  virtual wchar_t getSuffixOperator();

  virtual std::shared_ptr<Term>
  getLucenePrefixTerm(const std::wstring &fieldName);

  std::wstring toStringUnquoted() override;

protected:
  void suffixToString(std::shared_ptr<StringBuilder> r) override;

public:
  void visitMatchingTerms(
      std::shared_ptr<IndexReader> reader, const std::wstring &fieldName,
      std::shared_ptr<MatchingTermVisitor> mtv)  override;

protected:
  std::shared_ptr<SrndPrefixQuery> shared_from_this()
  {
    return std::static_pointer_cast<SrndPrefixQuery>(
        SimpleTerm::shared_from_this());
  }
};

} // namespace org::apache::lucene::queryparser::surround::query
