#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/DelegateStream.h"

#include  "core/src/java/org/apache/lucene/util/TestRuleMarkFailure.h"

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

using com::carrotsearch::randomizedtesting::rules::TestRuleAdapter;

/**
 * Fails the suite if it prints over the given limit of bytes to either
 * {@link System#out} or {@link System#err},
 * unless the condition is not enforced (see {@link #isEnforced()}).
 */
class TestRuleLimitSysouts : public TestRuleAdapter
{
  GET_CLASS_NAME(TestRuleLimitSysouts)
  /**
   * Max limit of bytes printed to either {@link System#out} or {@link
   * System#err}. This limit is enforced per-class (suite).
   */
public:
  static constexpr int DEFAULT_SYSOUT_BYTES_THRESHOLD = 8 * 1024;

  /**
   * An annotation specifying the limit of bytes per class.
   */
  // C++ TODO: Annotation types are not converted:
  //      @Documented @Inherited @Retention(RetentionPolicy.RUNTIME)
  //      @Target(ElementType.TYPE) public static @interface Limit
  //  {
  //    public int bytes();
  //  }

private:
  static const std::shared_ptr<AtomicInteger> bytesWritten;

  static const std::shared_ptr<DelegateStream> capturedSystemOut;
  static const std::shared_ptr<DelegateStream> capturedSystemErr;

  /**
   * We capture system output and error streams as early as possible because
   * certain components (like the Java logging system) steal these references
   * and never refresh them.
   *
   * Also, for this exact reason, we cannot change delegate streams for every
   * suite. This isn't as elegant as it should be, but there's no workaround for
   * this.
   */
private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static TestRuleLimitSysouts::StaticConstructor staticConstructor;

  /**
   * Test failures from any tests or rules before.
   */
  const std::shared_ptr<TestRuleMarkFailure> failureMarker;

  /**
   * Tracks the number of bytes written to an underlying stream by
   * incrementing an {@link AtomicInteger}.
   */
public:
  class DelegateStream : public FilterOutputStream
  {
    GET_CLASS_NAME(DelegateStream)
  public:
    const std::shared_ptr<PrintStream> printStream;
    const std::shared_ptr<AtomicInteger> bytesCounter;

    DelegateStream(std::shared_ptr<OutputStream> delegate_,
                   const std::wstring &charset,
                   std::shared_ptr<AtomicInteger> bytesCounter);

    // Do override all three write() methods to make sure nothing slips through.

    void write(std::deque<char> &b)  override;

    void write(std::deque<char> &b, int off,
               int len)  override;

    void write(int b)  override;

  protected:
    std::shared_ptr<DelegateStream> shared_from_this()
    {
      return std::static_pointer_cast<DelegateStream>(
          java.io.FilterOutputStream::shared_from_this());
    }
  };

public:
  TestRuleLimitSysouts(std::shared_ptr<TestRuleMarkFailure> failureMarker);

  /** */
protected:
  void before()  override;

private:
  void validateClassAnnotations();

  /**
   * Ensures {@link System#out} and {@link System#err} point to delegate
   * streams.
   */
public:
  static void checkCaptureStreams();

protected:
  virtual bool isEnforced();

  /**
   * We're only interested in failing the suite if it was successful (otherwise
   * just propagate the original problem and don't bother doing anything else).
   */
  void afterIfSuccessful()  override;

  void afterAlways(std::deque<std::runtime_error> &errors) throw(
      std::runtime_error) override;

private:
  void resetCaptureState();

protected:
  std::shared_ptr<TestRuleLimitSysouts> shared_from_this()
  {
    return std::static_pointer_cast<TestRuleLimitSysouts>(
        com.carrotsearch.randomizedtesting.rules
            .TestRuleAdapter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/
