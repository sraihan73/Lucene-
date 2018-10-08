#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/SPIClassIterator.h"
#include "exceptionhelper.h"
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
namespace org::apache::lucene::analysis::util
{

/**
 * Helper class for loading named SPIs from classpath (e.g. Tokenizers,
 * TokenStreams).
 * @lucene.internal
 */
template <typename S>
class AnalysisSPILoader final
    : public std::enable_shared_from_this<AnalysisSPILoader>
{
  GET_CLASS_NAME(AnalysisSPILoader)
  static_assert(std::is_base_of<AbstractAnalysisFactory, S>::value,
                L"S must inherit from AbstractAnalysisFactory");

private:
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile java.util.Map<std::wstring,Class> services =
  // java.util.Collections.emptyMap();
  std::unordered_map<std::wstring, std::type_info> services =
      Collections::emptyMap();
  const std::type_info<S> clazz;
  std::deque<std::wstring> const suffixes;

public:
  AnalysisSPILoader(std::type_info<S> &clazz)
      : AnalysisSPILoader(clazz, new std::wstring[]{clazz.name()})
  {
  }

  AnalysisSPILoader(std::type_info<S> &clazz,
                    std::shared_ptr<ClassLoader> loader)
      : AnalysisSPILoader(clazz, new std::wstring[]{clazz.name()}, loader)
  {
  }

  AnalysisSPILoader(std::type_info<S> &clazz,
                    std::deque<std::wstring> &suffixes)
      : AnalysisSPILoader(clazz, suffixes, nullptr)
  {
  }

  AnalysisSPILoader(std::type_info<S> &clazz,
                    std::deque<std::wstring> &suffixes,
                    std::shared_ptr<ClassLoader> classloader)
      : clazz(clazz), suffixes(suffixes)
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
   * iterators (e.g., from {@link #availableServices()},...) stay consistent.
   *
   * <p><b>NOTE:</b> Only new service providers are added, existing ones are
   * never removed or replaced.
   *
   * <p><em>This method is expensive and should only be called for discovery
   * of new service providers on the given classpath/classloader!</em>
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  void reload(std::shared_ptr<ClassLoader> classloader)
  {
GET_CLASS_NAME(loader))
        Objects::requireNonNull(classloader, L"classloader");
std::shared_ptr<LinkedHashMap<std::wstring, std::type_info>> *const services =
    std::make_shared<LinkedHashMap<std::wstring, std::type_info>>(
        this->services);
std::shared_ptr<SPIClassIterator<S>> *const loader =
    SPIClassIterator::get(clazz, classloader);
while (loader->hasNext()) {
  constexpr std::type_info service = loader->next();
  const std::wstring clazzName = service.name();
  std::wstring name = L"";
  for (auto suffix : suffixes) {
    if (StringHelper::endsWith(clazzName, suffix)) {
      name = clazzName.substr(0, clazzName.length() - suffix.length())
                 ->toLowerCase(Locale::ROOT);
      break;
    }
  }
  if (name == L"") {
    // C++ TODO: The following line could not be converted:
    throw java.util.ServiceConfigurationError(
        L"The class name " + service.getName() +
        L" has wrong suffix, allowed are: " +
        java.util.Arrays.toString(suffixes));
  }
  // only add the first one for each name, later services will be ignored
  // this allows to place services before others in classpath to make
  // them used instead of others
  //
  // TODO: Should we disallow duplicate names here?
  // Allowing it may get confusing on collisions, as different packages
  // could contain same factory class, which is a naming bug!
  // When changing this be careful to allow reload()!
  if (!services->containsKey(name)) {
    services->put(name, service);
  }
  loader++;
}
this->services = services;
  }

  S newInstance(const std::wstring &name,
                std::unordered_map<std::wstring, std::wstring> &args)
  {
    constexpr std::type_info service = lookupClass(name);
    return newFactoryClassInstance(service, args);
  }

  std::type_info lookupClass(const std::wstring &name)
  {
    constexpr std::type_info service = services[name.toLowerCase(Locale::ROOT)];
    if (service != nullptr) {
      return service;
    } else {
      throw std::invalid_argument(
          "A SPI class of type " + clazz.getName() + L" with name '" + name +
          L"' does not exist. " +
          L"You need to add the corresponding JAR file supporting this SPI to "
          L"your classpath. " +
          L"The current classpath supports the following names: " +
          availableServices());
    }
  }

  std::shared_ptr<Set<std::wstring>> availableServices()
  {
    return services.keySet();
  }

  /** Creates a new instance of the given {@link AbstractAnalysisFactory} by
   * invoking the constructor, passing the given argument map_obj. */
  template <typename T>
  static T
  newFactoryClassInstance(std::type_info<T> &clazz,
                          std::unordered_map<std::wstring, std::wstring> &args)
  {
    static_assert(std::is_base_of<AbstractAnalysisFactory, T>::value,
                  L"T must inherit from AbstractAnalysisFactory");

    try {
      return clazz.getConstructor(std::unordered_map::typeid).newInstance(args);
    } catch (const InvocationTargetException &ite) {
      constexpr std::runtime_error cause = ite->getCause();
      if (dynamic_cast<std::runtime_error>(cause) != nullptr) {
        throw static_cast<std::runtime_error>(cause);
      }
      if (std::dynamic_pointer_cast<Error>(cause) != nullptr) {
        throw std::static_pointer_cast<Error>(cause);
      }
      // C++ TODO: This exception's constructor requires only one argument:
      // ORIGINAL LINE: throw new RuntimeException("Unexpected checked exception
      // while calling constructor of "+clazz.getName(), cause);
      throw std::runtime_error(
          "Unexpected checked exception while calling constructor of " +
          clazz.getName());
    } catch (const ReflectiveOperationException &e) {
      throw std::make_shared<UnsupportedOperationException>(
          L"Factory " + clazz.getName() +
              L" cannot be instantiated. This is likely due to missing "
              L"Map<std::wstring,std::wstring> constructor.",
          e);
    }
  }
};

} // namespace org::apache::lucene::analysis::util
