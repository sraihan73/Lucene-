#pragma once
#include "stringhelper.h"
#include <memory>
#include <optional>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class DataInput;
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

using DataInput = org::apache::lucene::store::DataInput;

/**
 * This exception is thrown when Lucene detects
 * an index that is too old for this Lucene version
 */
class IndexFormatTooOldException : public IOException
{
  GET_CLASS_NAME(IndexFormatTooOldException)

private:
  const std::wstring resourceDescription;
  const std::wstring reason;
  const std::optional<int> version;
  const std::optional<int> minVersion;
  const std::optional<int> maxVersion;

  /** Creates an {@code IndexFormatTooOldException}.
   *
   *  @param resourceDescription describes the file that was too old
   *  @param reason the reason for this exception if the version is not
   * available
   *
   * @lucene.internal */
public:
  IndexFormatTooOldException(const std::wstring &resourceDescription,
                             const std::wstring &reason);

  /** Creates an {@code IndexFormatTooOldException}.
   *
   *  @param in the open file that's too old
   *  @param reason the reason for this exception if the version is not
   * available
   *
   * @lucene.internal */
  IndexFormatTooOldException(std::shared_ptr<DataInput> in_,
                             const std::wstring &reason);

  /** Creates an {@code IndexFormatTooOldException}.
   *
   *  @param resourceDescription describes the file that was too old
   *  @param version the version of the file that was too old
   *  @param minVersion the minimum version accepted
   *  @param maxVersion the maximum version accepted
   *
   * @lucene.internal */
  IndexFormatTooOldException(const std::wstring &resourceDescription,
                             int version, int minVersion, int maxVersion);

  /** Creates an {@code IndexFormatTooOldException}.
   *
   *  @param in the open file that's too old
   *  @param version the version of the file that was too old
   *  @param minVersion the minimum version accepted
   *  @param maxVersion the maximum version accepted
   *
   * @lucene.internal */
  IndexFormatTooOldException(std::shared_ptr<DataInput> in_, int version,
                             int minVersion, int maxVersion);

  /**
   * Returns a description of the file that was too old
   */
  virtual std::wstring getResourceDescription();

  /**
   * Returns an optional reason for this exception if the version information
   * was not available. Otherwise <code>null</code>
   */
  virtual std::wstring getReason();

  /**
   * Returns the version of the file that was too old.
   * This method will return <code>null</code> if an alternative {@link
   * #getReason()} is provided.
   */
  virtual std::optional<int> getVersion();

  /**
   * Returns the maximum version accepted.
   * This method will return <code>null</code> if an alternative {@link
   * #getReason()} is provided.
   */
  virtual std::optional<int> getMaxVersion();

  /**
   * Returns the minimum version accepted
   * This method will return <code>null</code> if an alternative {@link
   * #getReason()} is provided.
   */
  virtual std::optional<int> getMinVersion();

protected:
  std::shared_ptr<IndexFormatTooOldException> shared_from_this()
  {
    return std::static_pointer_cast<IndexFormatTooOldException>(
        java.io.IOException::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
