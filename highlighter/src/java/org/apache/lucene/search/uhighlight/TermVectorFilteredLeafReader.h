#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class Terms;
}

namespace org::apache::lucene::index
{
class LeafReader;
}
namespace org::apache::lucene::index
{
class TermsEnum;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::util::automaton
{
class CompiledAutomaton;
}
namespace org::apache::lucene::index
{
class PostingsEnum;
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
namespace org::apache::lucene::search::uhighlight
{

using FilterLeafReader = org::apache::lucene::index::FilterLeafReader;
using LeafReader = org::apache::lucene::index::LeafReader;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using BytesRef = org::apache::lucene::util::BytesRef;
using CompiledAutomaton =
    org::apache::lucene::util::automaton::CompiledAutomaton;

/**
 * A filtered LeafReader that only includes the terms that are also in a
 * provided set of terms. Certain methods may be unimplemented or cause large
 * operations on the underlying reader and be slow.
 *
 * @lucene.internal
 */
class TermVectorFilteredLeafReader final : public FilterLeafReader
{
  GET_CLASS_NAME(TermVectorFilteredLeafReader)
  // NOTE: super ("in") is baseLeafReader

private:
  const std::shared_ptr<Terms> filterTerms;

  /**
   * <p>Construct a FilterLeafReader based on the specified base reader.
   * <p>Note that base reader is closed if this FilterLeafReader is closed.</p>
   *
   * @param baseLeafReader full/original reader.
   * @param filterTerms set of terms to filter by -- probably from a TermVector
   * or MemoryIndex.
   */
public:
  TermVectorFilteredLeafReader(std::shared_ptr<LeafReader> baseLeafReader,
                               std::shared_ptr<Terms> filterTerms);

  std::shared_ptr<Terms>
  terms(const std::wstring &field)  override;

private:
  class TermsFilteredTerms final : public FilterLeafReader::FilterTerms
  {
    GET_CLASS_NAME(TermsFilteredTerms)
    // NOTE: super ("in") is the baseTerms

  private:
    const std::shared_ptr<Terms> filterTerms;

  public:
    TermsFilteredTerms(std::shared_ptr<Terms> baseTerms,
                       std::shared_ptr<Terms> filterTerms);

    // TODO delegate size() ?

    // TODO delegate getMin, getMax to filterTerms

    std::shared_ptr<TermsEnum> iterator()  override;

    std::shared_ptr<TermsEnum>
    intersect(std::shared_ptr<CompiledAutomaton> compiled,
              std::shared_ptr<BytesRef> startTerm)  override;

  protected:
    std::shared_ptr<TermsFilteredTerms> shared_from_this()
    {
      return std::static_pointer_cast<TermsFilteredTerms>(
          org.apache.lucene.index.FilterLeafReader
              .FilterTerms::shared_from_this());
    }
  };

private:
  class TermVectorFilteredTermsEnum final
      : public FilterLeafReader::FilterTermsEnum
  {
    GET_CLASS_NAME(TermVectorFilteredTermsEnum)
    // NOTE: super ("in") is the filteredTermsEnum. This is different than
    // wrappers above because we
    //    navigate the terms using the filter.

    // TODO: track the last term state from the term state method and do some
    // potential optimizations
  private:
    const std::shared_ptr<TermsEnum> baseTermsEnum;

  public:
    TermVectorFilteredTermsEnum(
        std::shared_ptr<TermsEnum> baseTermsEnum,
        std::shared_ptr<TermsEnum>
            filteredTermsEnum); // note this is reversed from constructors above

    // TODO delegate docFreq & ttf (moveToCurrentTerm() then call on full?

    std::shared_ptr<PostingsEnum>
    postings(std::shared_ptr<PostingsEnum> reuse,
             int flags)  override;

    void moveToCurrentTerm() ;

  protected:
    std::shared_ptr<TermVectorFilteredTermsEnum> shared_from_this()
    {
      return std::static_pointer_cast<TermVectorFilteredTermsEnum>(
          org.apache.lucene.index.FilterLeafReader
              .FilterTermsEnum::shared_from_this());
    }
  };

public:
  std::shared_ptr<CacheHelper> getCoreCacheHelper() override;

  std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

protected:
  std::shared_ptr<TermVectorFilteredLeafReader> shared_from_this()
  {
    return std::static_pointer_cast<TermVectorFilteredLeafReader>(
        org.apache.lucene.index.FilterLeafReader::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::uhighlight
