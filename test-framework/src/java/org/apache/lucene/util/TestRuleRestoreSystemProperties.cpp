using namespace std;

#include "TestRuleRestoreSystemProperties.h"

namespace org::apache::lucene::util
{
using com::carrotsearch::randomizedtesting::rules::SystemPropertiesRestoreRule;
using com::carrotsearch::randomizedtesting::rules::TestRuleAdapter;

TestRuleRestoreSystemProperties::TestRuleRestoreSystemProperties(
    deque<wstring> &propertyNames)
    : propertyNames(propertyNames)
{

  if (propertyNames.empty()) {
    throw invalid_argument(L"No properties to restore? Odd.");
  }
}

void TestRuleRestoreSystemProperties::before() 
{
  TestRuleAdapter::before();

  assert(restore.empty());
  for (auto key : propertyNames) {
    restore.emplace(key, System::getProperty(key));
  }
}

void TestRuleRestoreSystemProperties::afterAlways(
    deque<runtime_error> &errors) 
{
  for (auto key : propertyNames) {
    try {
      wstring value = restore[key];
      if (value == L"") {
        System::clearProperty(key);
      } else {
        System::setProperty(key, value);
      }
    } catch (const SecurityException &e) {
      // We should have permission to write but if we don't, record the error
      errors.push_back(e);
    }
  }
  restore.clear();

  TestRuleAdapter::afterAlways(errors);
}
} // namespace org::apache::lucene::util