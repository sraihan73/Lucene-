#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>

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
 * Some useful constants.
 **/

class Constants final : public std::enable_shared_from_this<Constants>
{
  GET_CLASS_NAME(Constants)
private:
  Constants();

  /** JVM vendor info. */
public:
  static const std::wstring JVM_VENDOR;
  static const std::wstring JVM_VERSION;
  static const std::wstring JVM_NAME;
  static const std::wstring JVM_SPEC_VERSION;

  /** The value of <tt>System.getProperty("java.version")</tt>. **/
  static const std::wstring JAVA_VERSION;

  /** The value of <tt>System.getProperty("os.name")</tt>. **/
  static const std::wstring OS_NAME;
  /** True iff running on Linux. */
  static const bool LINUX = StringHelper::startsWith(OS_NAME, L"Linux");
  /** True iff running on Windows. */
  static const bool WINDOWS = StringHelper::startsWith(OS_NAME, L"Windows");
  /** True iff running on SunOS. */
  static const bool SUN_OS = StringHelper::startsWith(OS_NAME, L"SunOS");
  /** True iff running on Mac OS X */
  static const bool MAC_OS_X = StringHelper::startsWith(OS_NAME, L"Mac OS X");
  /** True iff running on FreeBSD */
  static const bool FREE_BSD = StringHelper::startsWith(OS_NAME, L"FreeBSD");

  static const std::wstring OS_ARCH;
  static const std::wstring OS_VERSION;
  static const std::wstring JAVA_VENDOR;

private:
  static const int JVM_MAJOR_VERSION = 0;
  static const int JVM_MINOR_VERSION = 0;

  /** True iff running on a 64bit JVM */
public:
  static const bool JRE_IS_64BIT = false;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static Constants::StaticConstructor staticConstructor;

public:
  static const bool JRE_IS_MINIMUM_JAVA8 = JVM_MAJOR_VERSION > 1 ||
                                           (JVM_MAJOR_VERSION == 1 &&
                                            JVM_MINOR_VERSION >= 8);
  static const bool JRE_IS_MINIMUM_JAVA9 = JVM_MAJOR_VERSION > 1 ||
                                           (JVM_MAJOR_VERSION == 1 &&
                                            JVM_MINOR_VERSION >= 9);

  /**
   * This is the internal Lucene version, including bugfix versions, recorded
   * into each segment.
   * @deprecated Use {@link Version#LATEST}
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static final std::wstring LUCENE_MAIN_VERSION =
  // Version.LATEST.toString(); C++ TODO: There is no native C++ equivalent to
  // 'toString':
  static const std::wstring LUCENE_MAIN_VERSION;

  /**
   * Don't use this constant because the name is not self-describing!
   * @deprecated Use {@link Version#LATEST}
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Deprecated public static final std::wstring LUCENE_VERSION =
  // Version.LATEST.toString(); C++ TODO: There is no native C++ equivalent to
  // 'toString':
  static const std::wstring LUCENE_VERSION;
};

} // #include  "core/src/java/org/apache/lucene/util/
