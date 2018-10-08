#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
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
namespace org::apache::lucene::queryparser::flexible::messages
{

/**
 * MessageBundles classes extend this class, to implement a bundle.
 *
 * For Native Language Support (NLS), system of software internationalization.
 *
 * This interface is similar to the NLS class in eclipse.osgi.util.NLS class -
 * initializeMessages() method resets the values of all static strings, should
 * only be called by classes that extend from NLS (see TestMessages.java for
 * reference) - performs validation of all message in a bundle, at class load
 * time - performs per message validation at runtime - see NLSTest.java for
 * usage reference
 *
 * MessageBundle classes may subclass this type.
 */
class NLS : public std::enable_shared_from_this<NLS>
{
  GET_CLASS_NAME(NLS)

private:
  static std::unordered_map<std::wstring, std::type_info> bundles;

protected:
  NLS();

public:
  static std::wstring getLocalizedMessage(const std::wstring &key);

  static std::wstring getLocalizedMessage(const std::wstring &key,
                                          std::shared_ptr<Locale> locale);

  static std::wstring getLocalizedMessage(const std::wstring &key,
                                          std::shared_ptr<Locale> locale,
                                          std::deque<std::any> &args);

  static std::wstring getLocalizedMessage(const std::wstring &key,
                                          std::deque<std::any> &args);

  /**
   * Initialize a given class with the message bundle Keys Should be called from
   * a class that extends NLS in a static block at class load time.
   *
   * @param bundleName
   *          Property file with that contains the message bundle
   * @param clazz
   *          where constants will reside
   */
protected:
  static void initializeMessages(const std::wstring &bundleName,
                                 std::type_info clazz);

private:
  static std::any getResourceBundleObject(const std::wstring &messageKey,
                                          std::shared_ptr<Locale> locale);

  static void load(std::type_info clazz);

  static void loadfieldValue(std::shared_ptr<Field> field,
                             std::type_info clazz);

  /**
   * @param key
   *          - Message Key
   */
  static void validateMessage(const std::wstring &key, std::type_info clazz);
};

} // #include  "core/src/java/org/apache/lucene/queryparser/flexible/messages/
