#pragma once
#include "ResourceLoader.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/util/ResourceLoader.h"

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
namespace org::apache::lucene::analysis::util
{

/**
 * Simple {@link ResourceLoader} that opens resource files
 * from the local file system, optionally resolving against
 * a base directory.
 *
 * <p>This loader wraps a delegate {@link ResourceLoader}
 * that is used to resolve all files, the current base directory
 * does not contain. {@link #newInstance} is always resolved
 * against the delegate, as a {@link ClassLoader} is needed.
 *
 * <p>You can chain several {@code FilesystemResourceLoader}s
 * to allow lookup of files in more than one base directory.
 */
class FilesystemResourceLoader final
    : public std::enable_shared_from_this<FilesystemResourceLoader>,
      public ResourceLoader
{
  GET_CLASS_NAME(FilesystemResourceLoader)
private:
  const std::shared_ptr<Path> baseDirectory;
  const std::shared_ptr<ResourceLoader> delegate_;

  /**
   * Creates a resource loader that resolves resources against the given
   * base directory (may be {@code null} to refer to CWD).
   * Files not found in file system and class lookups are delegated to context
   * classloader.
   *
   * @deprecated You should not use this ctor, because it uses the thread's
context
   * class loader as fallback for resource lookups, which is bad programming
style.
   * Please specify a {@link ClassLoader} instead.
GET_CLASS_NAME(loader)
   * @see #FilesystemResourceLoader(Path, ClassLoader)
   */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public
  // FilesystemResourceLoader(java.nio.file.Path baseDirectory)
  FilesystemResourceLoader(std::shared_ptr<Path> baseDirectory);

  /**
   * Creates a resource loader that resolves resources against the given
   * base directory (may be {@code null} to refer to CWD).
   * Files not found in file system and class lookups are delegated to context
   * classloader.
   */
  FilesystemResourceLoader(std::shared_ptr<Path> baseDirectory,
                           std::shared_ptr<ClassLoader> delegate_);

  /**
   * Creates a resource loader that resolves resources against the given
   * base directory (may be {@code null} to refer to CWD).
   * Files not found in file system and class lookups are delegated
   * to the given delegate {@link ResourceLoader}.
GET_CLASS_NAME(lookups)
   */
  FilesystemResourceLoader(std::shared_ptr<Path> baseDirectory,
                           std::shared_ptr<ResourceLoader> delegate_);

  std::shared_ptr<InputStream>
  openResource(const std::wstring &resource)  override;

  template <typename T>
  T newInstance(const std::wstring &cname, std::type_info<T> &expectedType);

  template <typename T>
  std::type_info findClass(const std::wstring &cname,
                           std::type_info<T> &expectedType);
};

} // #include  "core/src/java/org/apache/lucene/analysis/util/
