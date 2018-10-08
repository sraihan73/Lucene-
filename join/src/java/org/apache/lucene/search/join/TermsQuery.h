#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRefHash;
}

namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::index
{
class Terms;
}
namespace org::apache::lucene::index
{
class TermsEnum;
}
namespace org::apache::lucene::util
{
class AttributeSource;
}
namespace org::apache::lucene::util
{
class BytesRef;
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
namespace org::apache::lucene::search::join
{

using FilteredTermsEnum = org::apache::lucene::index::FilteredTermsEnum;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using MultiTermQuery = org::apache::lucene::search::MultiTermQuery;
using Query = org::apache::lucene::search::Query;
using AttributeSource = org::apache::lucene::util::AttributeSource;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefHash = org::apache::lucene::util::BytesRefHash;

/**
 * A query that has an array of terms from a specific field. This query will
 * match documents have one or more terms in the specified field that match with
 * the terms specified in the array.
 *
 * @lucene.experimental
 */
class TermsQuery : public MultiTermQuery
{
  GET_CLASS_NAME(TermsQuery)

private:
  const std::shared_ptr<BytesRefHash> terms;
  std::deque<int> const ords;

  // These fields are used for equals() and hashcode() only
  const std::wstring fromField;
  const std::shared_ptr<Query> fromQuery;
  // id of the context rather than the context itself in order not to hold
  // references to index readers
  const std::any indexReaderContextId;

  /**
   * @param toField               The field that should contain terms that are
   * specified in the next parameter.
   * @param terms                 The terms that matching documents should have.
   * The terms must be sorted by natural order.
   * @param indexReaderContextId  Refers to the top level index reader used to
   * create the set of terms in the previous parameter.
   */
public:
  TermsQuery(const std::wstring &toField, std::shared_ptr<BytesRefHash> terms,
             const std::wstring &fromField, std::shared_ptr<Query> fromQuery,
             std::any indexReaderContextId);

protected:
  std::shared_ptr<TermsEnum> getTermsEnum(
      std::shared_ptr<Terms> terms,
      std::shared_ptr<AttributeSource> atts)  override;

public:
  std::wstring toString(const std::wstring &string) override;

  bool equals(std::any obj) override;

  virtual int hashCode();

public:
  class SeekingTermSetTermsEnum : public FilteredTermsEnum
  {
    GET_CLASS_NAME(SeekingTermSetTermsEnum)

  private:
    const std::shared_ptr<BytesRefHash> terms;
    std::deque<int> const ords;
    const int lastElement;

    const std::shared_ptr<BytesRef> lastTerm;
    const std::shared_ptr<BytesRef> spare = std::make_shared<BytesRef>();

    std::shared_ptr<BytesRef> seekTerm;
    int upto = 0;

  public:
    SeekingTermSetTermsEnum(std::shared_ptr<TermsEnum> tenum,
                            std::shared_ptr<BytesRefHash> terms,
                            std::deque<int> &ords);

  protected:
    std::shared_ptr<BytesRef> nextSeekTerm(
        std::shared_ptr<BytesRef> currentTerm)  override;

    AcceptStatus
    accept(std::shared_ptr<BytesRef> term)  override;

  protected:
    std::shared_ptr<SeekingTermSetTermsEnum> shared_from_this()
    {
      return std::static_pointer_cast<SeekingTermSetTermsEnum>(
          org.apache.lucene.index.FilteredTermsEnum::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TermsQuery> shared_from_this()
  {
    return std::static_pointer_cast<TermsQuery>(
        org.apache.lucene.search.MultiTermQuery::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::join
