#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>
#include <deque>

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
namespace org::apache::lucene::util
{

/**
 * Helper class for loading named SPIs from classpath (e.g. Codec,
 * PostingsFormat).
 * @lucene.internal
 */
template <typename S>
class NamedSPILoader final
    : public std::enable_shared_from_this<NamedSPILoader>,
      public std::deque<S>
{
  GET_CLASS_NAME(NamedSPILoader)
  static_assert(std::is_base_of<NamedSPILoader.NamedSPI, S>::value,
                L"S must inherit from NamedSPILoader.NamedSPI");

private:
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile java.util.Map<std::wstring,S> services =
  // java.util.Collections.emptyMap();
  std::unordered_map<std::wstring, S> services = Collections::emptyMap();
  const std::type_info<S> clazz;

public:
  NamedSPILoader(std::type_info<S> &clazz) : NamedSPILoader(clazz, nullptr) {}

  NamedSPILoader(std::type_info<S> &clazz,
                 std::shared_ptr<ClassLoader> classloader)
      : clazz(clazz)
  {
GET_CLASS_NAME(loader))
        // if clazz' classloader is not a parent of the given one, we scan clazz's classloader, too:
        std::shared_ptr<ClassLoader> * const clazzClassloader = clazz.getClassLoader();
if (classloader == nullptr) {
  GET_CLASS_NAME(loader)
  classloader = clazzClassloader;
}
if (clazzClassloader != nullptr &&
    !SPIClassIterator::isParentClassLoader(clazzClassloader, classloader)) {
GET_CLASS_NAME(loader)))
          reload(clazzClassloader);
}
reload(classloader);
  }

  /**
   * Reloads the internal SPI deque from the given {@link ClassLoader}.
   * Changes to the service deque are visible after the method ends, all
   * iterators ({@link #iterator()},...) stay consistent.
   *
   * <p><b>NOTE:</b> Only new service providers are added, existing ones are
   * never removed or replaced.
   *
   * <p><em>This method is expensive and should only be called for discovery
   * of new service providers on the given classpath/classloader!</em>
   */
  void reload(std::shared_ptr<ClassLoader> classloader)
  {
GET_CLASS_NAME(loader))
        Objects::requireNonNull(classloader, L"classloader");
std::shared_ptr<LinkedHashMap<std::wstring, S>> *const services =
    std::make_shared<LinkedHashMap<std::wstring, S>>(this->services);
std::shared_ptr<SPIClassIterator<S>> *const loader =
    SPIClassIterator::get(clazz, classloader);
while (loader->hasNext()) {
  constexpr std::type_info c = loader->next();
  try {
    constexpr S service = c.newInstance();
    const std::wstring name = service->getName();
    // only add the first one for each name, later services will be ignored
    // this allows to place services before others in classpath to make
    // them used instead of others
    if (!services->containsKey(name)) {
      checkServiceName(name);
      services->put(name, service);
    }
  } catch (const std::runtime_error &e) {
    throw std::make_shared<ServiceConfigurationError>(
        L"Cannot instantiate SPI class: " + c.getName(), e);
  }
  loader++;
}
this->services = services;
  }

  /**
   * Validates that a service name meets the requirements of {@link NamedSPI}
   */
  static void checkServiceName(const std::wstring &name)
  {
    // based on harmony charset.java
    if (name.length() >= 128) {
      throw std::invalid_argument("Illegal service name: '" + name +
                                  L"' is too long (must be < 128 chars).");
    }
    for (int i = 0, len = name.length(); i < len; i++) {
      wchar_t c = name[i];
      if (!isLetterOrDigit(c)) {
        throw std::invalid_argument("Illegal service name: '" + name +
                                    L"' must be simple ascii alphanumeric.");
      }
    }
  }

  /**
   * Checks whether a character is a letter or digit (ascii) which are defined
   * in the spec.
   */
private:
  static bool isLetterOrDigit(wchar_t c)
  {
    return (L'a' <= c && c <= L'z') || (L'A' <= c && c <= L'Z') ||
           (L'0' <= c && c <= L'9');
  }

public:
  S lookup(const std::wstring &name)
  {
    constexpr S service = services[name];
    if (service != nullptr) {
      return service;
    }
    throw std::invalid_argument(
        "An SPI class of type " + clazz.getName() + L" with name '" + name +
        L"' does not exist." +
        L"  You need to add the corresponding JAR file supporting this SPI to "
        L"your classpath." +
        L"  The current classpath supports the following names: " +
        availableServices());
  }

  std::shared_ptr<Set<std::wstring>> availableServices()
  {
    return services.keySet();
  }

  std::shared_ptr<Iterator<S>> iterator() override
  {
    return services.values().begin();
  }

  /**
   * Interface to support {@link NamedSPILoader#lookup(std::wstring)} by name.
   * <p>
   * Names must be all ascii alphanumeric, and less than 128 characters in
   * length.
   */
public:
  class NamedSPI
  {
    GET_CLASS_NAME(NamedSPI)
  public:
    virtual std::wstring getName() = 0;
  };
};

} // #include  "core/src/java/org/apache/lucene/util/
