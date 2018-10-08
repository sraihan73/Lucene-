#pragma once
#include "ResourceLoader.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
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
namespace org::apache::lucene::analysis::util
{

/**
 * Simple {@link ResourceLoader} that uses {@link
 * ClassLoader#getResourceAsStream(std::wstring)} and {@link
 * Class#forName(std::wstring,bool,ClassLoader)} to open resources and classes,
 * respectively.
 */
class ClasspathResourceLoader final
    : public std::enable_shared_from_this<ClasspathResourceLoader>,
      public ResourceLoader
{
  GET_CLASS_NAME(ClasspathResourceLoader)
private:
  const std::type_info clazz;
  const std::shared_ptr<ClassLoader> loader;

  /**
   * Creates an instance using the context classloader to load resources and
classes.
   * Resource paths must be absolute.
   *
   * @deprecated You should not use this ctor, because it uses the thread's
context
   * class loader, which is bad programming style. Please specify a reference
class or
   * a {@link ClassLoader} instead.
GET_CLASS_NAME(loader,)
   * @see #ClasspathResourceLoader(ClassLoader)
   * @see #ClasspathResourceLoader(Class)
   */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated @SuppressForbidden(reason = "Deprecated method
  // uses thread's context classloader, but there for backwards compatibility")
  // public ClasspathResourceLoader()
  ClasspathResourceLoader();

  /**
   * Creates an instance using the given classloader to load Resources and
   * classes. Resource paths must be absolute.
   */
  ClasspathResourceLoader(std::shared_ptr<ClassLoader> loader);

  /**
   * Creates an instance using the context classloader to load Resources and
   * classes Resources are resolved relative to the given class, if path is not
   * absolute.
   */
  ClasspathResourceLoader(std::type_info clazz);

private:
  ClasspathResourceLoader(std::type_info clazz,
                          std::shared_ptr<ClassLoader> loader);

public:
  std::shared_ptr<InputStream>
  openResource(const std::wstring &resource)  override;

  template <typename T>
  std::type_info findClass(const std::wstring &cname,
                           std::type_info<T> &expectedType);

  template <typename T>
  T newInstance(const std::wstring &cname, std::type_info<T> &expectedType);
};

} // namespace org::apache::lucene::analysis::util
