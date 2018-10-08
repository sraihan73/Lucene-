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
 * A {@link SecurityManager} that prevents tests calling {@link
 * System#exit(int)}. Only the test runner itself is allowed to exit the JVM.
 * All other security checks are handled by the default security policy.
 * <p>
 * Use this with {@code
 * -Djava.security.manager=org.apache.lucene.util.TestSecurityManager}.
 */
class TestSecurityManager final : public SecurityManager
{
  GET_CLASS_NAME(TestSecurityManager)

public:
  static const std::wstring JUNIT4_TEST_RUNNER_PACKAGE;
  static const std::wstring ECLIPSE_TEST_RUNNER_PACKAGE;
  static const std::wstring IDEA_TEST_RUNNER_PACKAGE;

  /**
   * Creates a new TestSecurityManager. This ctor is called on JVM startup,
   * when {@code
   * -Djava.security.manager=org.apache.lucene.util.TestSecurityManager} is
   * passed to JVM.
   */
  TestSecurityManager();

  /**
   * {@inheritDoc}
   * <p>This method inspects the stack trace and checks who is calling
   * {@link System#exit(int)} and similar methods
   * @throws SecurityException if the caller of this method is not the test
   * runner itself.
   */
  void checkExit(int const status) override;

private:
  class PrivilegedActionAnonymousInnerClass
      : public PrivilegedAction<std::shared_ptr<Void>>
  {
    GET_CLASS_NAME(PrivilegedActionAnonymousInnerClass)
  private:
    std::shared_ptr<TestSecurityManager> outerInstance;

    int status = 0;

  public:
    PrivilegedActionAnonymousInnerClass(
        std::shared_ptr<TestSecurityManager> outerInstance, int status);

    std::shared_ptr<Void> run() override;

  protected:
    std::shared_ptr<PrivilegedActionAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<PrivilegedActionAnonymousInnerClass>(
          java.security.PrivilegedAction<Void>::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestSecurityManager> shared_from_this()
  {
    return std::static_pointer_cast<TestSecurityManager>(
        SecurityManager::shared_from_this());
  }
};

} // namespace org::apache::lucene::util
