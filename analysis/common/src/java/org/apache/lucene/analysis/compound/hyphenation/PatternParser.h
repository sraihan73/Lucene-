#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <cctype>
#include <memory>
#include <regex>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/compound/hyphenation/PatternConsumer.h"

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
namespace org::apache::lucene::analysis::compound::hyphenation
{

// SAX
using org::xml::sax::Attributes;
using org::xml::sax::InputSource;
using org::xml::sax::SAXParseException;
using org::xml::sax::XMLReader;
using org::xml::sax::helpers::DefaultHandler;

// Java

/**
 * A SAX document handler to read and parse hyphenation patterns from a XML
 * file.
 *
 * This class has been taken from GPLv3 FOP project
 * (http://xmlgraphics.apache.org/fop/). They have been slightly modified.
 */
class PatternParser : public DefaultHandler
{
  GET_CLASS_NAME(PatternParser)

public:
  std::shared_ptr<XMLReader> parser;

  int currElement = 0;

  std::shared_ptr<PatternConsumer> consumer;

  std::shared_ptr<StringBuilder> token;

  std::deque<std::any> exception;

  wchar_t hyphenChar = L'\0';

  std::wstring errMsg;

  static constexpr int ELEM_CLASSES = 1;

  static constexpr int ELEM_EXCEPTIONS = 2;

  static constexpr int ELEM_PATTERNS = 3;

  static constexpr int ELEM_HYPHEN = 4;

  PatternParser();

  PatternParser(std::shared_ptr<PatternConsumer> consumer);

  virtual void setConsumer(std::shared_ptr<PatternConsumer> consumer);

  /**
   * Parses a hyphenation pattern file.
   *
   * @param filename the filename
   * @throws IOException In case of an exception while parsing
   */
  virtual void parse(const std::wstring &filename) ;

  /**
   * Parses a hyphenation pattern file.
   *
   * @param source the InputSource for the file
   * @throws IOException In case of an exception while parsing
   */
  virtual void parse(std::shared_ptr<InputSource> source) ;

  /**
   * Creates a SAX parser using JAXP
   *
   * @return the created SAX parser
   */
  static std::shared_ptr<XMLReader> createParser();

protected:
  virtual std::wstring readToken(std::shared_ptr<StringBuilder> chars);

  static std::wstring getPattern(const std::wstring &word);

  template <typename T1>
  std::deque<std::any> normalizeException(std::deque<T1> ex);

  template <typename T1>
  std::wstring getExceptionWord(std::deque<T1> ex);

  static std::wstring getInterletterValues(const std::wstring &pat);

  //
  // EntityResolver methods
  //
public:
  std::shared_ptr<InputSource>
  resolveEntity(const std::wstring &publicId,
                const std::wstring &systemId) override;

  //
  // ContentHandler methods
  //

  /**
   * @see org.xml.sax.ContentHandler#startElement(java.lang.std::wstring,
   *      java.lang.std::wstring, java.lang.std::wstring, org.xml.sax.Attributes)
   */
  void startElement(const std::wstring &uri, const std::wstring &local,
                    const std::wstring &raw,
                    std::shared_ptr<Attributes> attrs) override;

  /**
   * @see org.xml.sax.ContentHandler#endElement(java.lang.std::wstring,
   *      java.lang.std::wstring, java.lang.std::wstring)
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Override @SuppressWarnings({"unchecked", "rawtypes"})
  // public void endElement(std::wstring uri, std::wstring local, std::wstring raw)
  void endElement(const std::wstring &uri, const std::wstring &local,
                  const std::wstring &raw) override;

  /**
   * @see org.xml.sax.ContentHandler#characters(char[], int, int)
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings({"unchecked", "rawtypes"}) @Override
  // public void characters(char ch[], int start, int length)
  void characters(std::deque<wchar_t> &ch, int start, int length) override;

  /**
   * Returns a string of the location.
   */
private:
  std::wstring getLocationString(std::shared_ptr<SAXParseException> ex);

protected:
  std::shared_ptr<PatternParser> shared_from_this()
  {
    return std::static_pointer_cast<PatternParser>(
        org.xml.sax.helpers.DefaultHandler::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/compound/hyphenation/
