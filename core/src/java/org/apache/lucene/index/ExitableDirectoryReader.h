#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class QueryTimeout;
}

namespace org::apache::lucene::index
{
class LeafReader;
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
class BytesRef;
}
namespace org::apache::lucene::util::automaton
{
class CompiledAutomaton;
}
namespace org::apache::lucene::index
{
class DirectoryReader;
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
namespace org::apache::lucene::index
{

using FilterTerms = org::apache::lucene::index::FilterLeafReader::FilterTerms;
using FilterTermsEnum =
    org::apache::lucene::index::FilterLeafReader::FilterTermsEnum;
using BytesRef = org::apache::lucene::util::BytesRef;
using CompiledAutomaton =
    org::apache::lucene::util::automaton::CompiledAutomaton;

/**
 * The {@link ExitableDirectoryReader} wraps a real index {@link
 * DirectoryReader} and allows for a {@link QueryTimeout} implementation object
 * to be checked periodically to see if the thread should exit or not.  If
 * {@link QueryTimeout#shouldExit()} returns true, an {@link
 * ExitingReaderException} is thrown.
 */
class ExitableDirectoryReader : public FilterDirectoryReader
{
  GET_CLASS_NAME(ExitableDirectoryReader)

private:
  std::shared_ptr<QueryTimeout> queryTimeout;

  /**
   * Exception that is thrown to prematurely terminate a term enumeration.
   */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("serial") public static class
  // ExitingReaderException extends RuntimeException
  class ExitingReaderException : public std::runtime_error
  {

    /** Constructor **/
  public:
    ExitingReaderException(const std::wstring &msg);

  protected:
    std::shared_ptr<ExitingReaderException> shared_from_this()
    {
      return std::static_pointer_cast<ExitingReaderException>(
          RuntimeException::shared_from_this());
    }
  };

  /**
   * Wrapper class for a SubReaderWrapper that is used by the
   * ExitableDirectoryReader.
   */
public:
  class ExitableSubReaderWrapper : public SubReaderWrapper
  {
    GET_CLASS_NAME(ExitableSubReaderWrapper)
  private:
    std::shared_ptr<QueryTimeout> queryTimeout;

    /** Constructor **/
  public:
    ExitableSubReaderWrapper(std::shared_ptr<QueryTimeout> queryTimeout);

    std::shared_ptr<LeafReader>
    wrap(std::shared_ptr<LeafReader> reader) override;

  protected:
    std::shared_ptr<ExitableSubReaderWrapper> shared_from_this()
    {
      return std::static_pointer_cast<ExitableSubReaderWrapper>(
          SubReaderWrapper::shared_from_this());
    }
  };

  /**
   * Wrapper class for another FilterAtomicReader. This is used by
   * ExitableSubReaderWrapper.
   */
public:
  class ExitableFilterAtomicReader : public FilterLeafReader
  {
    GET_CLASS_NAME(ExitableFilterAtomicReader)

  private:
    std::shared_ptr<QueryTimeout> queryTimeout;

    /** Constructor **/
  public:
    ExitableFilterAtomicReader(std::shared_ptr<LeafReader> in_,
                               std::shared_ptr<QueryTimeout> queryTimeout);

    std::shared_ptr<Terms>
    terms(const std::wstring &field)  override;

    // this impl does not change deletes or data so we can delegate the
    // CacheHelpers
    std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

    std::shared_ptr<CacheHelper> getCoreCacheHelper() override;

  protected:
    std::shared_ptr<ExitableFilterAtomicReader> shared_from_this()
    {
      return std::static_pointer_cast<ExitableFilterAtomicReader>(
          FilterLeafReader::shared_from_this());
    }
  };

  /**
   * Wrapper class for another Terms implementation that is used by
   * ExitableFields.
   */
public:
  class ExitableTerms : public FilterTerms
  {
    GET_CLASS_NAME(ExitableTerms)

  private:
    std::shared_ptr<QueryTimeout> queryTimeout;

    /** Constructor **/
  public:
    ExitableTerms(std::shared_ptr<Terms> terms,
                  std::shared_ptr<QueryTimeout> queryTimeout);

    std::shared_ptr<TermsEnum>
    intersect(std::shared_ptr<CompiledAutomaton> compiled,
              std::shared_ptr<BytesRef> startTerm)  override;

    std::shared_ptr<TermsEnum> iterator()  override;

  protected:
    std::shared_ptr<ExitableTerms> shared_from_this()
    {
      return std::static_pointer_cast<ExitableTerms>(
          org.apache.lucene.index.FilterLeafReader
              .FilterTerms::shared_from_this());
    }
  };

  /**
   * Wrapper class for TermsEnum that is used by ExitableTerms for implementing
   * an exitable enumeration of terms.
   */
public:
  class ExitableTermsEnum : public FilterTermsEnum
  {
    GET_CLASS_NAME(ExitableTermsEnum)

  private:
    std::shared_ptr<QueryTimeout> queryTimeout;

    /** Constructor **/
  public:
    ExitableTermsEnum(std::shared_ptr<TermsEnum> termsEnum,
                      std::shared_ptr<QueryTimeout> queryTimeout);

    /**
     * Throws {@link ExitingReaderException} if {@link
     * QueryTimeout#shouldExit()} returns true, or if {@link
     * Thread#interrupted()} returns true.
     */
  private:
    void checkAndThrow();

  public:
    std::shared_ptr<BytesRef> next()  override;

  protected:
    std::shared_ptr<ExitableTermsEnum> shared_from_this()
    {
      return std::static_pointer_cast<ExitableTermsEnum>(
          org.apache.lucene.index.FilterLeafReader
              .FilterTermsEnum::shared_from_this());
    }
  };

  /**
   * Constructor
   * @param in DirectoryReader that this ExitableDirectoryReader wraps around to
   * make it Exitable.
   * @param queryTimeout The object to periodically check if the query should
   * time out.
   */
public:
  ExitableDirectoryReader(
      std::shared_ptr<DirectoryReader> in_,
      std::shared_ptr<QueryTimeout> queryTimeout) ;

protected:
  std::shared_ptr<DirectoryReader> doWrapDirectoryReader(
      std::shared_ptr<DirectoryReader> in_)  override;

  /**
   * Wraps a provided DirectoryReader. Note that for convenience, the returned
   * reader can be used normally (e.g. passed to {@link
   * DirectoryReader#openIfChanged(DirectoryReader)}) and so on.
   */
public:
  static std::shared_ptr<DirectoryReader>
  wrap(std::shared_ptr<DirectoryReader> in_,
       std::shared_ptr<QueryTimeout> queryTimeout) ;

  std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<ExitableDirectoryReader> shared_from_this()
  {
    return std::static_pointer_cast<ExitableDirectoryReader>(
        FilterDirectoryReader::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
