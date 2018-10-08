#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <typeinfo>
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
 * Helper class for loading SPI classes from classpath (META-INF files).
 * This is a light impl of {@link java.util.ServiceLoader} but is guaranteed to
GET_CLASS_NAME(for)
 * be bug-free regarding classpath order and does not instantiate or initialize
 * the classes found.
 *
 * @lucene.internal
 */
template <typename S>
class SPIClassIterator final
    : public std::enable_shared_from_this<SPIClassIterator>,
      public Iterator<std::type_info>
{
  GET_CLASS_NAME(SPIClassIterator)
private:
  // C++ TODO: Native C++ does not allow initialization of static
  // non-const/integral fields in their declarations - choose the conversion
  // option for separate .h and .cpp files:
  static const std::wstring META_INF_SERVICES = L"META-INF/services/";

  const std::type_info<S> clazz;
  const std::shared_ptr<ClassLoader> loader;
  const std::shared_ptr<Iterator<std::shared_ptr<URL>>> profilesEnum;
  std::shared_ptr<Iterator<std::wstring>> linesIterator;

  /** Creates a new SPI iterator to lookup services of type {@code clazz} using
   * the same {@link ClassLoader} as the argument. */
public:
  template <typename S>
  static std::shared_ptr<SPIClassIterator<S>> get(std::type_info<S> &clazz)
  {
    return std::make_shared<SPIClassIterator<S>>(
        clazz, Objects::requireNonNull(clazz.getClassLoader(), [&]() {
          return clazz + L" has no classloader.";
        }));
  }

  /** Creates a new SPI iterator to lookup services of type {@code clazz} using
   * the given classloader. */
  template <typename S>
  static std::shared_ptr<SPIClassIterator<S>>
  get(std::type_info<S> &clazz, std::shared_ptr<ClassLoader> loader)
  {
    return std::make_shared<SPIClassIterator<S>>(clazz, loader);
  }

  /**
   * Utility method to check if some class loader is a (grand-)parent of or the
   * same as another one. This means the child will be able to load all classes
   * from the parent, too. <p> If caller's codesource doesn't have enough
   * permissions to do the check, {@code false} is returned (this is fine,
   * because if we get a {@code SecurityException} it is for sure no parent).
   */
  static bool isParentClassLoader(std::shared_ptr<ClassLoader> parent,
                                  std::shared_ptr<ClassLoader> child)
  {
    try {
      std::shared_ptr<ClassLoader> cl = child;
      while (cl != nullptr) {
        if (cl == parent) {
          return true;
        }
        cl = cl->getParent();
      }
      return false;
    } catch (const SecurityException &se) {
      return false;
    }
  }

private:
  SPIClassIterator(std::type_info<S> &clazz,
                   std::shared_ptr<ClassLoader> loader)
      : clazz(Objects::requireNonNull(clazz, L"clazz")),
        loader(Objects::requireNonNull(loader, L"loader"))
  {
    try {
      const std::wstring fullName = META_INF_SERVICES + clazz.getName();
      this->profilesEnum = loader->getResources(fullName);
    } catch (const IOException &ioe) {
      throw std::make_shared<ServiceConfigurationError>(
          L"Error loading SPI profiles for type " + clazz.getName() +
              L" from classpath",
          ioe);
    }
    this->linesIterator = Collections::emptySet<std::wstring>().begin();
  }

  bool loadNextProfile()
  {
    std::deque<std::wstring> lines;
    while (profilesEnum->hasMoreElements()) {
      if (lines.size() > 0) {
        lines.clear();
      } else {
        lines = std::deque<std::wstring>();
      }
      std::shared_ptr<URL> *const url = profilesEnum->nextElement();
      try {
        std::shared_ptr<InputStream> *const in_ = url->openStream();
        bool success = false;
        try {
          std::shared_ptr<BufferedReader> *const reader =
              std::make_shared<BufferedReader>(
                  std::make_shared<InputStreamReader>(in_,
                                                      StandardCharsets::UTF_8));
          std::wstring line;
          while ((line = reader->readLine()) != L"") {
            constexpr int pos = (int)line.find(L'#');
            if (pos >= 0) {
              line = line.substr(0, pos);
            }
            line = StringHelper::trim(line);
            if (line.length() > 0) {
              lines.push_back(line);
            }
          }
          success = true;
        }
        // C++ TODO: There is no native C++ equivalent to the exception
        // 'finally' clause:
        finally {
          if (success) {
            IOUtils::close({in_});
          } else {
            IOUtils::closeWhileHandlingException({in_});
          }
        }
      } catch (const IOException &ioe) {
        throw std::make_shared<ServiceConfigurationError>(
            L"Error loading SPI class deque from URL: " + url, ioe);
      }
      if (!lines.empty()) {
        this->linesIterator = lines.begin();
        return true;
      }
    }
    return false;
  }

public:
  bool hasNext() override
  {
    return linesIterator->hasNext() || loadNextProfile();
  }

  std::type_info next() override
  {
    // hasNext() implicitely loads the next profile, so it is essential to call
    // this here!
    if (!hasNext()) {
      // C++ TODO: The following line could not be converted:
      throw java.util.NoSuchElementException();
    }
    assert(linesIterator->hasNext());
    const std::wstring c = linesIterator->next();
    try {
      // don't initialize the class (pass false as 2nd parameter):
      return std::type_info::forName(c, false, loader).asSubclass(clazz);
    } catch (const ClassNotFoundException &cnfe) {
      throw std::make_shared<ServiceConfigurationError>(std::wstring::format(
          Locale::ROOT,
          std::wstring(
              L"An SPI class of type %s with classname %s does not exist, ") +
              L"please fix the file '%s%1$s' in your classpath.",
          clazz.getName(), c, META_INF_SERVICES));
    }
  }

  void remove() override
  {
    throw std::make_shared<UnsupportedOperationException>();
  }
};

} // namespace org::apache::lucene::util
