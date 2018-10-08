using namespace std;

#include "TestRunWithRestrictedPermissions.h"

namespace org::apache::lucene::util
{

void TestRunWithRestrictedPermissions::testDefaultsPass() 
{
  runWithRestrictedPermissions(shared_from_this()::doSomeForbiddenStuff,
                               {make_shared<AllPermission>()});
}

void TestRunWithRestrictedPermissions::testNormallyAllowedStuff() throw(
    runtime_error)
{
  try {
    runWithRestrictedPermissions(shared_from_this()::doSomeForbiddenStuff);
    fail(L"this should not pass!");
  } catch (const SecurityException &se) {
    // pass
  }
}

void TestRunWithRestrictedPermissions::testCompletelyForbidden1() throw(
    runtime_error)
{
  try {
    runWithRestrictedPermissions(
        shared_from_this()::doSomeCompletelyForbiddenStuff);
    fail(L"this should not pass!");
  } catch (const SecurityException &se) {
    // pass
  }
}

void TestRunWithRestrictedPermissions::testCompletelyForbidden2() throw(
    runtime_error)
{
  try {
    runWithRestrictedPermissions(
        shared_from_this()::doSomeCompletelyForbiddenStuff,
        {make_shared<AllPermission>()});
    fail(L"this should not pass (not even with AllPermission)");
  } catch (const SecurityException &se) {
    // pass
  }
}

shared_ptr<Void>
TestRunWithRestrictedPermissions::doSomeForbiddenStuff() 
{
  createTempDir(L"cannot_create_temp_folder");
  return nullptr; // Void
}

shared_ptr<Void>
TestRunWithRestrictedPermissions::doSomeCompletelyForbiddenStuff() throw(
    IOException)
{
  Files::createFile(Paths->get(L"denied"));
  return nullptr; // Void
}
} // namespace org::apache::lucene::util