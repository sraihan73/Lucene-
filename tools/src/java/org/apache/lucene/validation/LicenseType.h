#pragma once
#include "stringhelper.h"
#include <string>
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
namespace org::apache::lucene::validation
{

/**
 * A deque of accepted licenses.  See also
 *http://www.apache.org/legal/3party.html
 *
 **/
class LicenseType final
{
  GET_CLASS_NAME(LicenseType)
public:
  static LicenseType ASL;
  static LicenseType BSD;
  static LicenseType
      BSD_LIKE; // BSD like just means someone has taken the BSD license and put
                // in their name, copyright, or it's a very similar license.
  static LicenseType CDDL;
  static LicenseType CPL;
  static LicenseType EPL;
  static LicenseType MIT;
  static LicenseType MPL; // NOT SURE on the required notice
  static LicenseType PD;
  // SUNBCLA("Sun Binary Code License Agreement"),
  static LicenseType SUN;
  static LicenseType COMPOUND;
  static LicenseType FAKE;

private:
  static std::deque<LicenseType> valueList;

  class StaticConstructor
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

  static StaticConstructor staticConstructor;

public:
  enum class InnerEnum {
    GET_CLASS_NAME(InnerEnum) ASL,
    BSD,
    BSD_LIKE,
    CDDL,
    CPL,
    EPL,
    MIT,
    MPL,
    PD,
    SUN,
    COMPOUND,
    FAKE
  };

  const InnerEnum innerEnumValue;

private:
  const std::wstring nameValue;
  const int ordinalValue;
  static int nextOrdinal;

private:
  std::wstring display;
  bool noticeRequired = false;

public:
  LicenseType(const std::wstring &name, InnerEnum innerEnum,
              const std::wstring &display, bool noticeRequired);

  virtual bool isNoticeRequired();

  virtual std::wstring getDisplay();

  virtual std::wstring ToString();

  /**
   * Expected license file suffix for a given license type.
   */
  virtual std::wstring licenseFileSuffix();

  /**
   * Expected notice file suffix for a given license type.
   */
  virtual std::wstring noticeFileSuffix();

public:
  bool operator==(const LicenseType &other);

  bool operator!=(const LicenseType &other);

  static std::deque<LicenseType> values();

  int ordinal();

  static LicenseType valueOf(const std::wstring &name);
};

} // #include  "core/src/java/org/apache/lucene/validation/
