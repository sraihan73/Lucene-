#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class Directory;
}

namespace org::apache::lucene::store
{
class IOContext;
}
namespace org::apache::lucene::store
{
class ChecksumIndexInput;
}
namespace org::apache::lucene::store
{
class AlreadyClosedException;
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
namespace org::apache::lucene::store
{

/**
 * Delegates all operations, even optional ones, to the wrapped directory.
 * <p>
 * This class is used if you want the most realistic testing, but still
 * with a checkindex on close. If you want asserts and evil things,
 * use MockDirectoryWrapper instead.
 */
class RawDirectoryWrapper final : public BaseDirectoryWrapper
{
  GET_CLASS_NAME(RawDirectoryWrapper)

public:
  RawDirectoryWrapper(std::shared_ptr<Directory> delegate_);

  void copyFrom(std::shared_ptr<Directory> from, const std::wstring &src,
                const std::wstring &dest,
                std::shared_ptr<IOContext> context)  override;

  std::shared_ptr<ChecksumIndexInput> openChecksumInput(
      const std::wstring &name,
      std::shared_ptr<IOContext> context)  override;

protected:
  void ensureOpen()  override;

protected:
  std::shared_ptr<RawDirectoryWrapper> shared_from_this()
  {
    return std::static_pointer_cast<RawDirectoryWrapper>(
        BaseDirectoryWrapper::shared_from_this());
  }
};

} // namespace org::apache::lucene::store
