#pragma once
#include "ContentSource.h"
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::benchmark::byTask::feeds
{
class NoMoreDataException;
}

namespace org::apache::lucene::benchmark::byTask::feeds
{
class DocData;
}
namespace org::apache::lucene::benchmark::byTask::utils
{
class Config;
}

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
namespace org::apache::lucene::benchmark::byTask::feeds
{

using Config = org::apache::lucene::benchmark::byTask::utils::Config;
using org::xml::sax::Attributes;
using org::xml::sax::SAXException;
using org::xml::sax::helpers::DefaultHandler;

/**
 * A {@link ContentSource} which reads the English Wikipedia dump. You can read
 * the .bz2 file directly (it will be decompressed on the fly). Config
 * properties:
 * <ul>
 * <li>keep.image.only.docs=false|true (default <b>true</b>).
 * <li>docs.file=&lt;path to the file&gt;
 * </ul>
 */
class EnwikiContentSource : public ContentSource
{
  GET_CLASS_NAME(EnwikiContentSource)

private:
  class Parser : public DefaultHandler, public Runnable
  {
    GET_CLASS_NAME(Parser)
  private:
    std::shared_ptr<EnwikiContentSource> outerInstance;

  public:
    Parser(std::shared_ptr<EnwikiContentSource> outerInstance);

  private:
    std::shared_ptr<Thread> t;
    bool threadDone = false;
    bool stopped = false;
    std::deque<std::wstring> tuple;
    std::shared_ptr<NoMoreDataException> nmde;
    std::shared_ptr<StringBuilder> contents = std::make_shared<StringBuilder>();
    std::wstring title;
    std::wstring body;
    // C++ NOTE: Fields cannot have the same name as methods:
    std::wstring time_;
    std::wstring id;

  public:
    virtual std::deque<std::wstring> next() ;

    virtual std::wstring time(const std::wstring &original);

    void characters(std::deque<wchar_t> &ch, int start, int length) override;

    void endElement(const std::wstring &namespace_, const std::wstring &simple,
                    const std::wstring &qualified)  override;

    void run() override;

    void startElement(const std::wstring &namespace_,
                      const std::wstring &simple, const std::wstring &qualified,
                      std::shared_ptr<Attributes> attributes) override;

  private:
    void stop();

  protected:
    std::shared_ptr<Parser> shared_from_this()
    {
      return std::static_pointer_cast<Parser>(
          org.xml.sax.helpers.DefaultHandler::shared_from_this());
    }
  };

private:
  static const std::unordered_map<std::wstring, int> ELEMENTS;
  static constexpr int TITLE = 0;
  static const int DATE = TITLE + 1;
  static const int BODY = DATE + 1;
  static const int ID = BODY + 1;
  static const int LENGTH = ID + 1;
  // LENGTH is used as the size of the tuple, so whatever constants we need that
  // should not be part of the tuple, we should define them after LENGTH.
  static const int PAGE = LENGTH + 1;

  static std::deque<std::wstring> const months;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static EnwikiContentSource::StaticConstructor staticConstructor;

  /**
   * Returns the type of the element if defined, otherwise returns -1. This
   * method is useful in startElement and endElement, by not needing to compare
   * the element qualified name over and over.
   */
  static int getElementType(const std::wstring &elem);

  std::shared_ptr<Path> file;
  bool keepImages = true;
  std::shared_ptr<InputStream> is;
  std::shared_ptr<Parser> parser = std::make_shared<Parser>(shared_from_this());

public:
  virtual ~EnwikiContentSource();

  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<DocData>
  getNextDocData(std::shared_ptr<DocData> docData) throw(NoMoreDataException,
                                                         IOException) override;

  void resetInputs()  override;

  /** Open the input stream. */
protected:
  virtual std::shared_ptr<InputStream> openInputStream() ;

public:
  void setConfig(std::shared_ptr<Config> config) override;

protected:
  std::shared_ptr<EnwikiContentSource> shared_from_this()
  {
    return std::static_pointer_cast<EnwikiContentSource>(
        ContentSource::shared_from_this());
  }
};

} // namespace org::apache::lucene::benchmark::byTask::feeds
