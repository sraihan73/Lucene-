#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <unordered_set>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/codecs/Codec.h"

#include  "core/src/java/org/apache/lucene/util/InfoStream.h"
#include  "core/src/java/org/apache/lucene/search/similarities/Similarity.h"
#include  "core/src/java/org/apache/lucene/codecs/PostingsFormat.h"
#include  "core/src/java/org/apache/lucene/codecs/DocValuesFormat.h"

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

using Codec = org::apache::lucene::codecs::Codec;
using Similarity = org::apache::lucene::search::similarities::Similarity;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.LuceneTestCase.INFOSTREAM;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.LuceneTestCase.LiveIWCFlushMode;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.LuceneTestCase.TEST_CODEC;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.LuceneTestCase.TEST_DOCVALUESFORMAT;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.LuceneTestCase.TEST_POSTINGSFORMAT;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.LuceneTestCase.VERBOSE;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.LuceneTestCase.assumeFalse;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.LuceneTestCase.localeForLanguageTag;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.LuceneTestCase.random;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.LuceneTestCase.randomLocale;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.LuceneTestCase.randomTimeZone;

/**
 * Setup and restore suite-level environment (fine grained junk that
 * doesn't fit anywhere else).
 */
class TestRuleSetupAndRestoreClassEnv final : public AbstractBeforeAfterRule
{
  GET_CLASS_NAME(TestRuleSetupAndRestoreClassEnv)
private:
  std::shared_ptr<Codec> savedCodec;
  std::shared_ptr<Locale> savedLocale;
  std::shared_ptr<TimeZone> savedTimeZone;
  std::shared_ptr<InfoStream> savedInfoStream;

public:
  std::shared_ptr<Locale> locale;
  std::shared_ptr<TimeZone> timeZone;
  std::shared_ptr<Similarity> similarity;
  std::shared_ptr<Codec> codec;

  /**
   * Indicates whether the rule has executed its {@link #before()} method fully.
   */
private:
  bool initialized = false;

  /**
   * @see SuppressCodecs
   */
public:
  std::unordered_set<std::wstring> avoidCodecs;

public:
  class ThreadNameFixingPrintStreamInfoStream : public PrintStreamInfoStream
  {
    GET_CLASS_NAME(ThreadNameFixingPrintStreamInfoStream)
  public:
    ThreadNameFixingPrintStreamInfoStream(std::shared_ptr<PrintStream> out);

    void message(const std::wstring &component,
                 const std::wstring &message) override;

  protected:
    std::shared_ptr<ThreadNameFixingPrintStreamInfoStream> shared_from_this()
    {
      return std::static_pointer_cast<ThreadNameFixingPrintStreamInfoStream>(
          PrintStreamInfoStream::shared_from_this());
    }
  };

public:
  bool isInitialized();

protected:
  void before()  override;

private:
  class AssertingCodecAnonymousInnerClass : public AssertingCodec
  {
    GET_CLASS_NAME(AssertingCodecAnonymousInnerClass)
  private:
    std::shared_ptr<TestRuleSetupAndRestoreClassEnv> outerInstance;

    std::shared_ptr<PostingsFormat> format;
    std::shared_ptr<DocValuesFormat> dvFormat;

  public:
    AssertingCodecAnonymousInnerClass(
        std::shared_ptr<TestRuleSetupAndRestoreClassEnv> outerInstance,
        std::shared_ptr<PostingsFormat> format,
        std::shared_ptr<DocValuesFormat> dvFormat);

    std::shared_ptr<PostingsFormat>
    getPostingsFormatForField(const std::wstring &field) override;

    std::shared_ptr<DocValuesFormat>
    getDocValuesFormatForField(const std::wstring &field) override;

    virtual std::wstring toString();

  protected:
    std::shared_ptr<AssertingCodecAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AssertingCodecAnonymousInnerClass>(
          org.apache.lucene.codecs.asserting
              .AssertingCodec::shared_from_this());
    }
  };

  /**
   * Check codec restrictions.
   *
   * @throws AssumptionViolatedException if the class does not work with a given
   * codec.
   */
private:
  void checkCodecRestrictions(std::shared_ptr<Codec> codec);

  /**
   * After suite cleanup (always invoked).
   */
protected:
  void after()  override;

  /**
   * Should a given codec be avoided for the currently executing suite?
   */
private:
  bool shouldAvoidCodec(const std::wstring &codec);

protected:
  std::shared_ptr<TestRuleSetupAndRestoreClassEnv> shared_from_this()
  {
    return std::static_pointer_cast<TestRuleSetupAndRestoreClassEnv>(
        AbstractBeforeAfterRule::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/
