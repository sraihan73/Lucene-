using namespace std;

#include "Constants.h"

namespace org::apache::lucene::util
{

Constants::Constants() {} // can't construct

const wstring Constants::JVM_VENDOR = System::getProperty(L"java.vm.vendor");
const wstring Constants::JVM_VERSION = System::getProperty(L"java.vm.version");
const wstring Constants::JVM_NAME = System::getProperty(L"java.vm.name");
const wstring Constants::JVM_SPEC_VERSION =
    System::getProperty(L"java.specification.version");
const wstring Constants::JAVA_VERSION = System::getProperty(L"java.version");
const wstring Constants::OS_NAME = System::getProperty(L"os.name");
const wstring Constants::OS_ARCH = System::getProperty(L"os.arch");
const wstring Constants::OS_VERSION = System::getProperty(L"os.version");
const wstring Constants::JAVA_VENDOR = System::getProperty(L"java.vendor");

Constants::StaticConstructor::StaticConstructor()
{
  shared_ptr<StringTokenizer> *const st =
      make_shared<StringTokenizer>(JVM_SPEC_VERSION, L".");
  JVM_MAJOR_VERSION = static_cast<Integer>(st->nextToken());
  if (st->hasMoreTokens()) {
    JVM_MINOR_VERSION = static_cast<Integer>(st->nextToken());
  } else {
    JVM_MINOR_VERSION = 0;
  }
  bool is64Bit = false;
  wstring datamodel = L"";
  try {
    datamodel = System::getProperty(L"sun.arch.data.model");
    if (datamodel != L"") {
      is64Bit = datamodel.find(L"64") != wstring::npos;
    }
  } catch (const SecurityException &ex) {
  }
  if (datamodel == L"") {
    if (OS_ARCH != L"" && OS_ARCH.find(L"64") != wstring::npos) {
      is64Bit = true;
    } else {
      is64Bit = false;
    }
  }
  JRE_IS_64BIT = is64Bit;
}

Constants::StaticConstructor Constants::staticConstructor;
const wstring Constants::LUCENE_MAIN_VERSION = Version::LATEST->toString();
const wstring Constants::LUCENE_VERSION = Version::LATEST->toString();
} // namespace org::apache::lucene::util