#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class DataInput;
}

namespace org::apache::lucene::store
{
class DataOutput;
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
using DataOutput = org::apache::lucene::store::DataOutput;

/**
 * This exception is thrown when Lucene detects
 * an inconsistency in the index.
 */
class CorruptIndexException : public IOException
{
  GET_CLASS_NAME(CorruptIndexException)

private:
  const std::wstring message;
  const std::wstring resourceDescription;

  /** Create exception with a message only */
public:
  CorruptIndexException(const std::wstring &message,
                        std::shared_ptr<DataInput> input);

  /** Create exception with a message only */
  CorruptIndexException(const std::wstring &message,
                        std::shared_ptr<DataOutput> output);

  /** Create exception with message and root cause. */
  CorruptIndexException(const std::wstring &message,
                        std::shared_ptr<DataInput> input,
                        std::runtime_error cause);

  /** Create exception with message and root cause. */
  CorruptIndexException(const std::wstring &message,
                        std::shared_ptr<DataOutput> output,
                        std::runtime_error cause);

  /** Create exception with a message only */
  CorruptIndexException(const std::wstring &message,
                        const std::wstring &resourceDescription);

  /** Create exception with message and root cause. */
  CorruptIndexException(const std::wstring &message,
                        const std::wstring &resourceDescription,
                        std::runtime_error cause);

  /**
   * Returns a description of the file that was corrupted
   */
  virtual std::wstring getResourceDescription();

  /**
   * Returns the original exception message without the corrupted file
   * description.
   */
  virtual std::wstring getOriginalMessage();

protected:
  std::shared_ptr<CorruptIndexException> shared_from_this()
  {
    return std::static_pointer_cast<CorruptIndexException>(
        java.io.IOException::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
