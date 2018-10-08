using namespace std;

#include "DOMUtils.h"

namespace org::apache::lucene::queryparser::xml
{
using org::w3c::dom::Document;
using org::w3c::dom::Element;
using org::w3c::dom::Node;
using org::xml::sax::InputSource;

shared_ptr<Element>
DOMUtils::getChildByTagOrFail(shared_ptr<Element> e,
                              const wstring &name) 
{
  shared_ptr<Element> kid = getChildByTagName(e, name);
  if (nullptr == kid) {
    throw make_shared<ParserException>(e->getTagName() + L" missing \"" + name +
                                       L"\" child element");
  }
  return kid;
}

shared_ptr<Element>
DOMUtils::getFirstChildOrFail(shared_ptr<Element> e) 
{
  shared_ptr<Element> kid = getFirstChildElement(e);
  if (nullptr == kid) {
    throw make_shared<ParserException>(e->getTagName() +
                                       L" does not contain a child element");
  }
  return kid;
}

wstring DOMUtils::getAttributeOrFail(shared_ptr<Element> e,
                                     const wstring &name) 
{
  wstring v = e->getAttribute(name);
  if (L"" == v) {
    throw make_shared<ParserException>(e->getTagName() + L" missing \"" + name +
                                       L"\" attribute");
  }
  return v;
}

wstring DOMUtils::getAttributeWithInheritanceOrFail(
    shared_ptr<Element> e, const wstring &name) 
{
  wstring v = getAttributeWithInheritance(e, name);
  if (L"" == v) {
    throw make_shared<ParserException>(e->getTagName() + L" missing \"" + name +
                                       L"\" attribute");
  }
  return v;
}

wstring
DOMUtils::getNonBlankTextOrFail(shared_ptr<Element> e) 
{
  wstring v = getText(e);
  if (L"" != v) {
    v = StringHelper::trim(v);
  }
  if (L"" == v || 0 == v.length()) {
    throw make_shared<ParserException>(e->getTagName() + L" has no text");
  }
  return v;
}

shared_ptr<Element> DOMUtils::getChildByTagName(shared_ptr<Element> e,
                                                const wstring &name)
{
  for (shared_ptr<Node> kid = e->getFirstChild(); kid != nullptr;
       kid = kid->getNextSibling()) {
    if ((kid->getNodeType() == Node::ELEMENT_NODE) &&
        (name == kid->getNodeName())) {
      return std::static_pointer_cast<Element>(kid);
    }
  }
  return nullptr;
}

wstring DOMUtils::getAttributeWithInheritance(shared_ptr<Element> element,
                                              const wstring &attributeName)
{
  wstring result = element->getAttribute(attributeName);
  if ((result == L"") || (L"" == result)) {
    shared_ptr<Node> n = element->getParentNode();
    if ((n == element) || (n == nullptr)) {
      return L"";
    }
    if (std::dynamic_pointer_cast<Element>(n) != nullptr) {
      shared_ptr<Element> parent = std::static_pointer_cast<Element>(n);
      return getAttributeWithInheritance(parent, attributeName);
    }
    return L""; // we reached the top level of the document without finding
                // attribute
  }
  return result;
}

wstring DOMUtils::getChildTextByTagName(shared_ptr<Element> e,
                                        const wstring &tagName)
{
  shared_ptr<Element> child = getChildByTagName(e, tagName);
  return child != nullptr ? getText(child) : L"";
}

shared_ptr<Element> DOMUtils::insertChild(shared_ptr<Element> parent,
                                          const wstring &tagName,
                                          const wstring &text)
{
  shared_ptr<Element> child = parent->getOwnerDocument().createElement(tagName);
  parent->appendChild(child);
  if (text != L"") {
    child->appendChild(child->getOwnerDocument().createTextNode(text));
  }
  return child;
}

wstring DOMUtils::getAttribute(shared_ptr<Element> element,
                               const wstring &attributeName,
                               const wstring &deflt)
{
  wstring result = element->getAttribute(attributeName);
  return (result == L"") || (L"" == result) ? deflt : result;
}

float DOMUtils::getAttribute(shared_ptr<Element> element,
                             const wstring &attributeName, float deflt)
{
  wstring result = element->getAttribute(attributeName);
  return (result == L"") || (L"" == result) ? deflt : stof(result);
}

int DOMUtils::getAttribute(shared_ptr<Element> element,
                           const wstring &attributeName, int deflt)
{
  wstring result = element->getAttribute(attributeName);
  return (result == L"") || (L"" == result) ? deflt : stoi(result);
}

bool DOMUtils::getAttribute(shared_ptr<Element> element,
                            const wstring &attributeName, bool deflt)
{
  wstring result = element->getAttribute(attributeName);
  return (result == L"") || (L"" == result)
             ? deflt
             : StringHelper::fromString<bool>(result);
}

wstring DOMUtils::getText(shared_ptr<Node> e)
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  getTextBuffer(e, sb);
  return sb->toString();
}

shared_ptr<Element> DOMUtils::getFirstChildElement(shared_ptr<Element> element)
{
  for (shared_ptr<Node> kid = element->getFirstChild(); kid != nullptr;
       kid = kid->getNextSibling()) {
    if (kid->getNodeType() == Node::ELEMENT_NODE) {
      return std::static_pointer_cast<Element>(kid);
    }
  }
  return nullptr;
}

void DOMUtils::getTextBuffer(shared_ptr<Node> e, shared_ptr<StringBuilder> sb)
{
  for (shared_ptr<Node> kid = e->getFirstChild(); kid != nullptr;
       kid = kid->getNextSibling()) {
    switch (kid->getNodeType()) {
    case Node::TEXT_NODE: {
      sb->append(kid->getNodeValue());
      break;
    }
    case Node::ELEMENT_NODE: {
      getTextBuffer(kid, sb);
      break;
    }
    case Node::ENTITY_REFERENCE_NODE: {
      getTextBuffer(kid, sb);
      break;
    }
    }
  }
}

shared_ptr<Document> DOMUtils::loadXML(shared_ptr<Reader> is)
{
  shared_ptr<DocumentBuilderFactory> dbf =
      DocumentBuilderFactory::newInstance();
  shared_ptr<DocumentBuilder> db = nullptr;

  try {
    db = dbf->newDocumentBuilder();
  } catch (const runtime_error &se) {
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new RuntimeException("Parser configuration error",
    // se);
    throw runtime_error(L"Parser configuration error");
  }

  // Step 3: parse the input file
  shared_ptr<Document> doc = nullptr;
  try {
    doc = db->parse(make_shared<InputSource>(is));
    // doc = db.parse(is);
  } catch (const runtime_error &se) {
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new RuntimeException("Error parsing file:" + se,
    // se);
    throw runtime_error(L"Error parsing file:" + se);
  }

  return doc;
}
} // namespace org::apache::lucene::queryparser::xml