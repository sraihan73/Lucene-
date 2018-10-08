#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <type_traits>
#include <typeinfo>

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
namespace org::apache::lucene::mockfile
{

/**
 * A {@code FilterFileStore} contains another
 * {@code FileStore}, which it uses as its basic
 * source of data, possibly transforming the data along the
 * way or providing additional functionality.
 */
class FilterFileStore : public FileStore
{
  GET_CLASS_NAME(FilterFileStore)

  /**
   * The underlying {@code FileStore} instance.
   */
protected:
  const std::shared_ptr<FileStore> delegate_;

  /**
   * URI scheme used for this instance.
   */
  const std::wstring scheme;

  /**
   * Construct a {@code FilterFileStore} based on
   * the specified base store.
   * @param delegate specified base store.
   * @param scheme URI scheme identifying this instance.
   */
public:
  FilterFileStore(std::shared_ptr<FileStore> delegate_,
                  const std::wstring &scheme);

  std::wstring name() override;

  std::wstring type() override;

  virtual std::wstring toString();

  bool isReadOnly() override;

  int64_t getTotalSpace()  override;

  int64_t getUsableSpace()  override;

  int64_t getUnallocatedSpace()  override;

  bool supportsFileAttributeView(std::type_info type) override;

  bool supportsFileAttributeView(const std::wstring &name) override;

  template <typename V>
  V getFileStoreAttributeView(std::type_info<V> &type);

  std::any
  getAttribute(const std::wstring &attribute)  override;

protected:
  std::shared_ptr<FilterFileStore> shared_from_this()
  {
    return std::static_pointer_cast<FilterFileStore>(
        java.nio.file.FileStore::shared_from_this());
  }
};

} // namespace org::apache::lucene::mockfile
