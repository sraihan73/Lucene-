#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
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

using IndexReader = org::apache::lucene::index::IndexReader;
using Term = org::apache::lucene::index::Term;

/**
 * Simple single-term clause
 */
class SrndTermQuery : public SimpleTerm
{
  GET_CLASS_NAME(SrndTermQuery)
public:
  SrndTermQuery(const std::wstring &termText, bool quoted);

private:
  const std::wstring termText;

public:
  virtual std::wstring getTermText();

  virtual std::shared_ptr<Term> getLuceneTerm(const std::wstring &fieldName);

  std::wstring toStringUnquoted() override;
  void visitMatchingTerms(
      std::shared_ptr<IndexReader> reader, const std::wstring &fieldName,
      std::shared_ptr<MatchingTermVisitor> mtv)  override;

protected:
  std::shared_ptr<SrndTermQuery> shared_from_this()
  {
    return std::static_pointer_cast<SrndTermQuery>(
        SimpleTerm::shared_from_this());
  }
};

} // namespace org::apache::lucene::queryparser::surround::query
