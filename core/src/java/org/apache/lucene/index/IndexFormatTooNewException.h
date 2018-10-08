#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/DataInput.h"

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
 * an index that is newer than this Lucene version.
 */
class IndexFormatTooNewException : public IOException
{
  GET_CLASS_NAME(IndexFormatTooNewException)

private:
  const std::wstring resourceDescription;
  const int version;
  const int minVersion;
  const int maxVersion;

  /** Creates an {@code IndexFormatTooNewException}
   *
   *  @param resourceDescription describes the file that was too new
   *  @param version the version of the file that was too new
   *  @param minVersion the minimum version accepted
   *  @param maxVersion the maximum version accepted
   *
   * @lucene.internal */
public:
  IndexFormatTooNewException(const std::wstring &resourceDescription,
                             int version, int minVersion, int maxVersion);

  /** Creates an {@code IndexFormatTooNewException}
   *
   *  @param in the open file that's too new
   *  @param version the version of the file that was too new
   *  @param minVersion the minimum version accepted
   *  @param maxVersion the maximum version accepted
   *
   * @lucene.internal */
  IndexFormatTooNewException(std::shared_ptr<DataInput> in_, int version,
                             int minVersion, int maxVersion);

  /**
   * Returns a description of the file that was too new
   */
  virtual std::wstring getResourceDescription();

  /**
   * Returns the version of the file that was too new
   */
  virtual int getVersion();

  /**
   * Returns the maximum version accepted
   */
  virtual int getMaxVersion();

  /**
   * Returns the minimum version accepted
   */
  virtual int getMinVersion();

protected:
  std::shared_ptr<IndexFormatTooNewException> shared_from_this()
  {
    return std::static_pointer_cast<IndexFormatTooNewException>(
        java.io.IOException::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
