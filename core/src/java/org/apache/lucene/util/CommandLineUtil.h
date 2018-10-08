#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/FSDirectory.h"

#include  "core/src/java/org/apache/lucene/store/LockFactory.h"

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

using FSDirectory = org::apache::lucene::store::FSDirectory;
using LockFactory = org::apache::lucene::store::LockFactory;

/**
 * Class containing some useful methods used by command line tools
 *
 */
class CommandLineUtil final
    : public std::enable_shared_from_this<CommandLineUtil>
{
  GET_CLASS_NAME(CommandLineUtil)

private:
  CommandLineUtil();

  /**
   * Creates a specific FSDirectory instance starting from its class name, using
   * the default lock factory
   * @param clazzName The name of the FSDirectory class to load
   * @param path The path to be used as parameter constructor
   * @return the new FSDirectory instance
   */
public:
  static std::shared_ptr<FSDirectory>
  newFSDirectory(const std::wstring &clazzName, std::shared_ptr<Path> path);

  /**
   * Creates a specific FSDirectory instance starting from its class name
   * @param clazzName The name of the FSDirectory class to load
   * @param path The path to be used as parameter constructor
   * @param lf The lock factory to be used
   * @return the new FSDirectory instance
   */
  static std::shared_ptr<FSDirectory>
  newFSDirectory(const std::wstring &clazzName, std::shared_ptr<Path> path,
                 std::shared_ptr<LockFactory> lf);

  /**
   * Loads a specific Directory implementation
   * @param clazzName The name of the Directory class to load
   * @return The Directory class loaded
   * @throws ClassNotFoundException If the specified class cannot be found.
   */
  static std::type_info loadDirectoryClass(const std::wstring &clazzName) throw(
      ClassNotFoundException);

  /**
   * Loads a specific FSDirectory implementation
   * @param clazzName The name of the FSDirectory class to load
   * @return The FSDirectory class loaded
   * @throws ClassNotFoundException If the specified class cannot be found.
   */
  static std::type_info loadFSDirectoryClass(
      const std::wstring &clazzName) ;

private:
  static std::wstring adjustDirectoryClassName(const std::wstring &clazzName);

  /**
   * Creates a new specific FSDirectory instance
   * @param clazz The class of the object to be created
   * @param path The file to be used as parameter constructor
   * @return The new FSDirectory instance
   * @throws NoSuchMethodException If the Directory does not have a constructor
   * that takes <code>Path</code>.
   * @throws InstantiationException If the class is abstract or an interface.
   * @throws IllegalAccessException If the constructor does not have public
   * visibility.
   * @throws InvocationTargetException If the constructor throws an exception
   */
public:
  static std::shared_ptr<FSDirectory> newFSDirectory(
      std::type_info clazz,
      std::shared_ptr<Path> path) ;

  /**
   * Creates a new specific FSDirectory instance
   * @param clazz The class of the object to be created
   * @param path The file to be used as parameter constructor
   * @param lf The lock factory to be used
   * @return The new FSDirectory instance
   * @throws NoSuchMethodException If the Directory does not have a constructor
   * that takes <code>Path</code>.
   * @throws InstantiationException If the class is abstract or an interface.
   * @throws IllegalAccessException If the constructor does not have public
   * visibility.
   * @throws InvocationTargetException If the constructor throws an exception
   */
  static std::shared_ptr<FSDirectory> newFSDirectory(
      std::type_info clazz, std::shared_ptr<Path> path,
      std::shared_ptr<LockFactory> lf) ;
};

} // #include  "core/src/java/org/apache/lucene/util/
