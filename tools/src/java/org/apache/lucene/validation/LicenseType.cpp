using namespace std;

#include "LicenseType.h"

namespace org::apache::lucene::validation
{

LicenseType LicenseType::ASL(L"ASL", InnerEnum::ASL,
                             L"Apache Software License 2.0", true);
LicenseType LicenseType::BSD(L"BSD", InnerEnum::BSD,
                             L"Berkeley Software Distribution", true);
LicenseType LicenseType::BSD_LIKE(L"BSD_LIKE", InnerEnum::BSD_LIKE,
                                  L"BSD like license", true);
LicenseType LicenseType::CDDL(L"CDDL", InnerEnum::CDDL,
                              L"Common Development and Distribution License",
                              false);
LicenseType LicenseType::CPL(L"CPL", InnerEnum::CPL, L"Common Public License",
                             true);
LicenseType LicenseType::EPL(L"EPL", InnerEnum::EPL,
                             L"Eclipse Public License Version 1.0", false);
LicenseType LicenseType::MIT(L"MIT", InnerEnum::MIT,
                             L"Massachusetts Institute of Tech. License",
                             false);
LicenseType LicenseType::MPL(L"MPL", InnerEnum::MPL, L"Mozilla Public License",
                             false);
LicenseType LicenseType::PD(L"PD", InnerEnum::PD, L"Public Domain", false);
LicenseType LicenseType::SUN(L"SUN", InnerEnum::SUN, L"Sun Open Source License",
                             false);
LicenseType LicenseType::COMPOUND(L"COMPOUND", InnerEnum::COMPOUND,
                                  L"Compound license (see NOTICE).", true);
LicenseType LicenseType::FAKE(L"FAKE", InnerEnum::FAKE,
                              L"FAKE license - not needed", false);

deque<LicenseType> LicenseType::valueList;

LicenseType::StaticConstructor::StaticConstructor()
{
  valueList.push_back(ASL);
  valueList.push_back(BSD);
  valueList.push_back(BSD_LIKE);
  valueList.push_back(CDDL);
  valueList.push_back(CPL);
  valueList.push_back(EPL);
  valueList.push_back(MIT);
  valueList.push_back(MPL);
  valueList.push_back(PD);
  valueList.push_back(SUN);
  valueList.push_back(COMPOUND);
  valueList.push_back(FAKE);
}

LicenseType::StaticConstructor LicenseType::staticConstructor;
int LicenseType::nextOrdinal = 0;

LicenseType::LicenseType(const wstring &name, InnerEnum innerEnum,
                         const wstring &display, bool noticeRequired)
    : nameValue(name), ordinalValue(nextOrdinal++), innerEnumValue(innerEnum)
{
  this->display = display;
  this->noticeRequired = noticeRequired;
}

bool LicenseType::isNoticeRequired() { return noticeRequired; }

wstring LicenseType::getDisplay() { return display; }

wstring LicenseType::ToString()
{
  return wstring(L"LicenseType{") + L"display='" + display + L'\'' + L'}';
}

wstring LicenseType::licenseFileSuffix()
{
  return L"-LICENSE-" + this->name() + L".txt";
}

wstring LicenseType::noticeFileSuffix() { return L"-NOTICE.txt"; }

bool LicenseType::operator==(const LicenseType &other)
{
  return this->ordinalValue == other.ordinalValue;
}

bool LicenseType::operator!=(const LicenseType &other)
{
  return this->ordinalValue != other.ordinalValue;
}

deque<LicenseType> LicenseType::values() { return valueList; }

int LicenseType::ordinal() { return ordinalValue; }

LicenseType LicenseType::valueOf(const wstring &name)
{
  for (auto enumInstance : LicenseType::valueList) {
    if (enumInstance.nameValue == name) {
      return enumInstance;
    }
  }
}
} // namespace org::apache::lucene::validation