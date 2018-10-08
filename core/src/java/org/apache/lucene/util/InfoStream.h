#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
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
 * Debugging API for Lucene classes such as {@link IndexWriter}
 * and {@link SegmentInfos}.
GET_CLASS_NAME(es)
 * <p>
 * NOTE: Enabling infostreams may cause performance degradation
 * in some components.
 *
 * @lucene.internal
 */
class InfoStream : public std::enable_shared_from_this<InfoStream>
{
  GET_CLASS_NAME(InfoStream)

  /** Instance of InfoStream that does no logging at all. */
public:
  static const std::shared_ptr<InfoStream> NO_OUTPUT;

private:
  class NoOutput;

  /** prints a message */
public:
  virtual void message(const std::wstring &component,
                       const std::wstring &message) = 0;

  /** returns true if messages are enabled and should be posted to {@link
   * #message}. */
  virtual bool isEnabled(const std::wstring &component) = 0;

private:
  static std::shared_ptr<InfoStream> defaultInfoStream;

  /** The default {@code InfoStream} used by a newly instantiated classes.
   * @see #setDefault */
public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  static std::shared_ptr<InfoStream> getDefault();

  /** Sets the default {@code InfoStream} used
   * by a newly instantiated classes. It cannot be {@code null},
   * to disable logging use {@link #NO_OUTPUT}.
GET_CLASS_NAME(es.)
   * @see #getDefault */
  // C++ WARNING: The following method was originally marked 'synchronized':
  static void setDefault(std::shared_ptr<InfoStream> infoStream);
};

} // namespace org::apache::lucene::util
class InfoStream::NoOutput final : public InfoStream
{
  GET_CLASS_NAME(InfoStream::NoOutput)
public:
  void message(const std::wstring &component,
               const std::wstring &message) override;

  bool isEnabled(const std::wstring &component) override;

  virtual ~NoOutput();

protected:
  std::shared_ptr<NoOutput> shared_from_this()
  {
    return std::static_pointer_cast<NoOutput>(InfoStream::shared_from_this());
  }
};
