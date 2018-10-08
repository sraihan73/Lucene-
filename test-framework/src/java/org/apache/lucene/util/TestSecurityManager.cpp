using namespace std;

#include "TestSecurityManager.h"

namespace org::apache::lucene::util
{

const wstring TestSecurityManager::JUNIT4_TEST_RUNNER_PACKAGE =
    L"com.carrotsearch.ant.tasks.junit4.";
const wstring TestSecurityManager::ECLIPSE_TEST_RUNNER_PACKAGE =
    L"org.eclipse.jdt.internal.junit.runner.";
const wstring TestSecurityManager::IDEA_TEST_RUNNER_PACKAGE =
    L"com.intellij.rt.execution.junit.";

TestSecurityManager::TestSecurityManager() : SecurityManager() {}

void TestSecurityManager::checkExit(int const status)
{
  AccessController::doPrivileged(
      make_shared<PrivilegedActionAnonymousInnerClass>(shared_from_this(),
                                                       status));

  // we passed the stack check, delegate to super, so default policy can still
  // deny permission:
  SecurityManager::checkExit(status);
}

TestSecurityManager::PrivilegedActionAnonymousInnerClass::
    PrivilegedActionAnonymousInnerClass(
        shared_ptr<TestSecurityManager> outerInstance, int status)
{
  this->outerInstance = outerInstance;
  this->status = status;
}

shared_ptr<Void> TestSecurityManager::PrivilegedActionAnonymousInnerClass::run()
{
  const wstring systemClassName = System::typeid->getName(),
                runtimeClassName = Runtime::typeid->getName();
  wstring exitMethodHit = L"";
  for (shared_ptr<StackTraceElement> se :
       Thread::currentThread().getStackTrace()) {
    const wstring className = se->getClassName(),
                  methodName = se->getMethodName();
    if ((L"exit" == methodName || L"halt" == methodName) &&
        (systemClassName == className || runtimeClassName == className)) {
      exitMethodHit = className + StringHelper::toString(L'#') + methodName +
                      StringHelper::toString(L'(') + to_wstring(status) +
                      StringHelper::toString(L')');
      continue;
    }

    if (exitMethodHit != L"") {
      if (StringHelper::startsWith(className, JUNIT4_TEST_RUNNER_PACKAGE) ||
          StringHelper::startsWith(className, ECLIPSE_TEST_RUNNER_PACKAGE) ||
          StringHelper::startsWith(className, IDEA_TEST_RUNNER_PACKAGE)) {
        // this exit point is allowed, we return normally from closure:
        return nullptr;
      } else {
        // anything else in stack trace is not allowed, break and throw
        // SecurityException below:
        break;
      }
    }
  }

  if (exitMethodHit == L"") {
    // should never happen, only if JVM hides stack trace - replace by generic:
    exitMethodHit = L"JVM exit method";
  }
  throw make_shared<SecurityException>(
      exitMethodHit +
      L" calls are not allowed because they terminate the test runner's JVM.");
}
} // namespace org::apache::lucene::util