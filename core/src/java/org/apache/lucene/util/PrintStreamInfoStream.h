#pragma once
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
 * InfoStream implementation over a {@link PrintStream}
 * such as <code>System.out</code>.
 *
 * @lucene.internal
 */
class PrintStreamInfoStream : public InfoStream
{
  GET_CLASS_NAME(PrintStreamInfoStream)
  // Used for printing messages
private:
  static const std::shared_ptr<AtomicInteger> MESSAGE_ID;

protected:
  const int messageID;

  const std::shared_ptr<PrintStream> stream;

public:
  PrintStreamInfoStream(std::shared_ptr<PrintStream> stream);

  PrintStreamInfoStream(std::shared_ptr<PrintStream> stream, int messageID);

  void message(const std::wstring &component,
               const std::wstring &message) override;

  bool isEnabled(const std::wstring &component) override;

  virtual ~PrintStreamInfoStream();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressForbidden(reason = "System.out/err detection")
  // public bool isSystemStream()
  virtual bool isSystemStream();

  /** Returns the current time as string for insertion into log messages. */
protected:
  virtual std::wstring getTimestamp();

protected:
  std::shared_ptr<PrintStreamInfoStream> shared_from_this()
  {
    return std::static_pointer_cast<PrintStreamInfoStream>(
        InfoStream::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/
