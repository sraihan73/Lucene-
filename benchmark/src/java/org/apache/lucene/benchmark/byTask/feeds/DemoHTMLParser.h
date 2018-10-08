#pragma once
#include "HTMLParser.h"
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/benchmark/byTask/feeds/DocData.h"

#include  "core/src/java/org/apache/lucene/benchmark/byTask/feeds/TrecContentSource.h"

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

using org::xml::sax::InputSource;
using org::xml::sax::SAXException;

/**
 * Simple HTML Parser extracting title, meta tags, and body text
 * that is based on <a href="http://nekohtml.sourceforge.net/">NekoHTML</a>.
 */
class DemoHTMLParser : public std::enable_shared_from_this<DemoHTMLParser>,
                       public HTMLParser
{
  GET_CLASS_NAME(DemoHTMLParser)

  /** The actual parser to read HTML documents */
public:
  class Parser final : public std::enable_shared_from_this<Parser>
  {
    GET_CLASS_NAME(Parser)

  public:
    const std::shared_ptr<Properties> metaTags = std::make_shared<Properties>();
    const std::wstring title, body;

    Parser(std::shared_ptr<Reader> reader) ;

    // C++ TODO: There is no native C++ equivalent to 'toString':
    Parser(std::shared_ptr<InputSource> source) throw(IOException,
                                                      SAXException);

  private:
    class DefaultHandlerAnonymousInnerClass : public DefaultHandler
    {
      GET_CLASS_NAME(DefaultHandlerAnonymousInnerClass)
    private:
      std::shared_ptr<Parser> outerInstance;

      std::shared_ptr<StringBuilder> title;
      std::shared_ptr<StringBuilder> body;

    public:
      DefaultHandlerAnonymousInnerClass(std::shared_ptr<Parser> outerInstance,
                                        std::shared_ptr<StringBuilder> title,
                                        std::shared_ptr<StringBuilder> body);

    private:
      int inBODY = 0;

    public:
      void startElement(
          const std::wstring &namespaceURI, const std::wstring &localName,
          const std::wstring &qName,
          std::shared_ptr<Attributes> atts)  override;

      void endElement(const std::wstring &namespaceURI,
                      const std::wstring &localName,
                      const std::wstring &qName)  override;

      void characters(std::deque<wchar_t> &ch, int start,
                      int length)  override;

      std::shared_ptr<InputSource>
      resolveEntity(const std::wstring &publicId,
                    const std::wstring &systemId) override;

    protected:
      std::shared_ptr<DefaultHandlerAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<DefaultHandlerAnonymousInnerClass>(
            org.xml.sax.helpers.DefaultHandler::shared_from_this());
      }
    };

  private:
    static std::shared_ptr<Set<std::wstring>>
    createElementNameSet(std::deque<std::wstring> &names);

    /** HTML elements that cause a line break (they are block-elements) */
  public:
    static const std::shared_ptr<Set<std::wstring>> ENDLINE_ELEMENTS;

    /** HTML elements with contents that are ignored */
    static const std::shared_ptr<Set<std::wstring>> SUPPRESS_ELEMENTS;
  };

public:
  std::shared_ptr<DocData>
  parse(std::shared_ptr<DocData> docData, const std::wstring &name, Date date,
        std::shared_ptr<Reader> reader,
        std::shared_ptr<TrecContentSource> trecSrc)  override;

  virtual std::shared_ptr<DocData>
  parse(std::shared_ptr<DocData> docData, const std::wstring &name, Date date,
        std::shared_ptr<InputSource> source,
        std::shared_ptr<TrecContentSource> trecSrc) throw(IOException,
                                                          SAXException);
};

} // #include  "core/src/java/org/apache/lucene/benchmark/byTask/feeds/
