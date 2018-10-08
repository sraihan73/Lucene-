#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::queryparser::xml
{
class ParserException;
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
namespace org::apache::lucene::queryparser::xml
{

using org::w3c::dom::Document;
using org::w3c::dom::Element;
using org::w3c::dom::Node;

/**
 * Helper methods for parsing XML
 */
class DOMUtils : public std::enable_shared_from_this<DOMUtils>
{
  GET_CLASS_NAME(DOMUtils)

public:
  static std::shared_ptr<Element>
  getChildByTagOrFail(std::shared_ptr<Element> e,
                      const std::wstring &name) ;

  static std::shared_ptr<Element>
  getFirstChildOrFail(std::shared_ptr<Element> e) ;

  static std::wstring
  getAttributeOrFail(std::shared_ptr<Element> e,
                     const std::wstring &name) ;

  static std::wstring getAttributeWithInheritanceOrFail(
      std::shared_ptr<Element> e,
      const std::wstring &name) ;

  static std::wstring
  getNonBlankTextOrFail(std::shared_ptr<Element> e) ;

  /* Convenience method where there is only one child Element of a given name */
  static std::shared_ptr<Element> getChildByTagName(std::shared_ptr<Element> e,
                                                    const std::wstring &name);

  /**
   * Returns an attribute value from this node, or first parent node with this
   * attribute defined
   *
   * @return A non-zero-length value if defined, otherwise null
   */
  static std::wstring
  getAttributeWithInheritance(std::shared_ptr<Element> element,
                              const std::wstring &attributeName);

  /* Convenience method where there is only one child Element of a given name */
  static std::wstring getChildTextByTagName(std::shared_ptr<Element> e,
                                            const std::wstring &tagName);

  /* Convenience method to append a new child with text*/
  static std::shared_ptr<Element> insertChild(std::shared_ptr<Element> parent,
                                              const std::wstring &tagName,
                                              const std::wstring &text);

  static std::wstring getAttribute(std::shared_ptr<Element> element,
                                   const std::wstring &attributeName,
                                   const std::wstring &deflt);

  static float getAttribute(std::shared_ptr<Element> element,
                            const std::wstring &attributeName, float deflt);

  static int getAttribute(std::shared_ptr<Element> element,
                          const std::wstring &attributeName, int deflt);

  static bool getAttribute(std::shared_ptr<Element> element,
                           const std::wstring &attributeName, bool deflt);

  /* Returns text of node and all child nodes - without markup */
  // MH changed to Node from Element 25/11/2005

  static std::wstring getText(std::shared_ptr<Node> e);

  static std::shared_ptr<Element>
  getFirstChildElement(std::shared_ptr<Element> element);

private:
  static void getTextBuffer(std::shared_ptr<Node> e,
                            std::shared_ptr<StringBuilder> sb);

  /**
   * Helper method to parse an XML file into a DOM tree, given a reader.
   *
   * @param is reader of the XML file to be parsed
   * @return an org.w3c.dom.Document object
   */
public:
  static std::shared_ptr<Document> loadXML(std::shared_ptr<Reader> is);
};

} // namespace org::apache::lucene::queryparser::xml
