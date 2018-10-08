#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

#include  "core/src/java/org/apache/lucene/store/IOContext.h"

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
 * This directory wrapper overrides {@link Directory#copyFrom(Directory, std::wstring,
 * std::wstring, IOContext)} in order to optionally use a hard-link instead of a full
 * byte by byte file copy if applicable. Hard-links are only used if the
 * underlying filesystem supports it and if the {@link
 * java.nio.file.LinkPermission} "hard" is granted.
 *
 * <p><b>NOTE:</b> Using hard-links changes the copy semantics of
 * {@link Directory#copyFrom(Directory, std::wstring, std::wstring, IOContext)}. When
 * hard-links are used changes to the source file will be reflected in the
 * target file and vice-versa. Within Lucene, files are write once and should
 * not be modified after they have been written. This directory should not be
 * used in situations where files change after they have been written.
 * </p>
 */
class HardlinkCopyDirectoryWrapper final : public FilterDirectory
{
  GET_CLASS_NAME(HardlinkCopyDirectoryWrapper)
  /**
   * Creates a new HardlinkCopyDirectoryWrapper delegating to the given
   * directory
   */
public:
  HardlinkCopyDirectoryWrapper(std::shared_ptr<Directory> in_);

  void copyFrom(std::shared_ptr<Directory> from, const std::wstring &srcFile,
                const std::wstring &destFile,
                std::shared_ptr<IOContext> context)  override;

protected:
  std::shared_ptr<HardlinkCopyDirectoryWrapper> shared_from_this()
  {
    return std::static_pointer_cast<HardlinkCopyDirectoryWrapper>(
        FilterDirectory::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/store/
