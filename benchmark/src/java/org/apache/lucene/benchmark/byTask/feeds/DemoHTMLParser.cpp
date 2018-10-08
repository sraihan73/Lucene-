using namespace std;

#include "DemoHTMLParser.h"
#include "DocData.h"
#include "TrecContentSource.h"

namespace org::apache::lucene::benchmark::byTask::feeds
{
using org::cyberneko::html::parsers::SAXParser;
using org::xml::sax::Attributes;
using org::xml::sax::InputSource;
using org::xml::sax::SAXException;
using org::xml::sax::helpers::DefaultHandler;

DemoHTMLParser::Parser::Parser(shared_ptr<Reader> reader) throw(IOException,
                                                                SAXException)
    : Parser(new InputSource(reader))
{
}

// C++ TODO: There is no native C++ equivalent to 'toString':
DemoHTMLParser::Parser::Parser(shared_ptr<InputSource> source) throw(
    IOException, SAXException)
    : title(title.toString()->trim()), body(body.toString())
{
  shared_ptr<SAXParser> *const parser = make_shared<SAXParser>();
  parser->setFeature(L"http://xml.org/sax/features/namespaces", true);
  parser->setFeature(L"http://cyberneko.org/html/features/balance-tags", true);
  parser->setFeature(L"http://cyberneko.org/html/features/report-errors",
                     false);
  parser->setProperty(L"http://cyberneko.org/html/properties/names/elems",
                      L"lower");
  parser->setProperty(L"http://cyberneko.org/html/properties/names/attrs",
                      L"lower");

  shared_ptr<StringBuilder> *const title = make_shared<StringBuilder>(),
                                   *const body = make_shared<StringBuilder>();
  // C++ TODO: You cannot use 'shared_from_this' in a constructor:
  shared_ptr<DefaultHandler> *const handler =
      make_shared<DefaultHandlerAnonymousInnerClass>(shared_from_this(), title,
                                                     body);

  parser->setContentHandler(handler);
  parser->setErrorHandler(handler);
  parser->parse(source);

  // the javacc-based parser trimmed title (which should be done for HTML in all
  // cases):

  // assign body text
}

DemoHTMLParser::Parser::DefaultHandlerAnonymousInnerClass::
    DefaultHandlerAnonymousInnerClass(shared_ptr<Parser> outerInstance,
                                      shared_ptr<StringBuilder> title,
                                      shared_ptr<StringBuilder> body)
{
  this->outerInstance = outerInstance;
  this->title = title;
  this->body = body;
  inBODY = 0, inHEAD = 0, inTITLE = 0, suppressed = 0;
}

void DemoHTMLParser::Parser::DefaultHandlerAnonymousInnerClass::startElement(
    const wstring &namespaceURI, const wstring &localName, const wstring &qName,
    shared_ptr<Attributes> atts) 
{
  if (inHEAD > 0) {
    if (L"title" == localName) {
      inTITLE++;
    } else {
      if (L"meta" == localName) {
        wstring name = atts->getValue(L"name");
        if (name == L"") {
          name = atts->getValue(L"http-equiv");
        }
        const wstring val = atts->getValue(L"content");
        if (name != L"" && val != L"") {
          outerInstance->metaTags->setProperty(name.toLowerCase(Locale::ROOT),
                                               val);
        }
      }
    }
  } else if (inBODY > 0) {
    if (SUPPRESS_ELEMENTS->contains(localName)) {
      suppressed++;
    } else if (L"img" == localName) {
      // the original javacc-based parser preserved <IMG alt="..."/>
      // attribute as body text in [] parenthesis:
      const wstring alt = atts->getValue(L"alt");
      if (alt != L"") {
        body->append(L'[')->append(alt)->append(L']');
      }
    }
  } else if (L"body" == localName) {
    inBODY++;
  } else if (L"head" == localName) {
    inHEAD++;
  } else if (L"frameset" == localName) {
    // C++ TODO: The following line could not be converted:
    throw org.xml.sax.SAXException(
        L"This parser does not support HTML framesets.");
  }
}

void DemoHTMLParser::Parser::DefaultHandlerAnonymousInnerClass::endElement(
    const wstring &namespaceURI, const wstring &localName,
    const wstring &qName) 
{
  if (inBODY > 0) {
    if (L"body" == localName) {
      inBODY--;
    } else if (ENDLINE_ELEMENTS->contains(localName)) {
      body->append(L'\n');
    } else if (SUPPRESS_ELEMENTS->contains(localName)) {
      suppressed--;
    }
  } else if (inHEAD > 0) {
    if (L"head" == localName) {
      inHEAD--;
    } else if (inTITLE > 0 && L"title" == localName) {
      inTITLE--;
    }
  }
}

void DemoHTMLParser::Parser::DefaultHandlerAnonymousInnerClass::characters(
    std::deque<wchar_t> &ch, int start, int length) 
{
  if (inBODY > 0 && suppressed == 0) {
    body->append(ch, start, length);
  } else if (inTITLE > 0) {
    title->append(ch, start, length);
  }
}

shared_ptr<InputSource>
DemoHTMLParser::Parser::DefaultHandlerAnonymousInnerClass::resolveEntity(
    const wstring &publicId, const wstring &systemId)
{
  // disable network access caused by DTDs
  return make_shared<InputSource>(make_shared<StringReader>(L""));
}

shared_ptr<Set<wstring>>
DemoHTMLParser::Parser::createElementNameSet(deque<wstring> &names)
{
  return Collections::unmodifiableSet(unordered_set<>(Arrays::asList(names)));
}

const shared_ptr<java::util::Set<wstring>>
    DemoHTMLParser::Parser::ENDLINE_ELEMENTS = createElementNameSet(
        {L"p",        L"h1",    L"h2",       L"h3",         L"h4",
         L"h5",       L"h6",    L"div",      L"ul",         L"ol",
         L"dl",       L"pre",   L"hr",       L"blockquote", L"address",
         L"fieldset", L"table", L"form",     L"noscript",   L"li",
         L"dt",       L"dd",    L"noframes", L"br",         L"tr",
         L"select",   L"option"});
const shared_ptr<java::util::Set<wstring>>
    DemoHTMLParser::Parser::SUPPRESS_ELEMENTS =
        createElementNameSet({L"style", L"script"});

shared_ptr<DocData>
DemoHTMLParser::parse(shared_ptr<DocData> docData, const wstring &name,
                      Date date, shared_ptr<Reader> reader,
                      shared_ptr<TrecContentSource> trecSrc) 
{
  try {
    return parse(docData, name, date, make_shared<InputSource>(reader),
                 trecSrc);
  } catch (const SAXException &saxe) {
    throw make_shared<IOException>(
        L"SAX exception occurred while parsing HTML document.", saxe);
  }
}

shared_ptr<DocData>
DemoHTMLParser::parse(shared_ptr<DocData> docData, const wstring &name,
                      Date date, shared_ptr<InputSource> source,
                      shared_ptr<TrecContentSource> trecSrc) throw(IOException,
                                                                   SAXException)
{
  shared_ptr<Parser> *const p = make_shared<Parser>(source);

  // properties
  shared_ptr<Properties> *const props = p->metaTags;
  wstring dateStr = props->getProperty(L"date");
  if (dateStr != L"") {
    constexpr Date newDate = trecSrc->parseDate(dateStr);
    if (newDate != nullptr) {
      date = newDate;
    }
  }

  docData->clear();
  docData->setName(name);
  docData->setBody(p->body);
  docData->setTitle(p->title);
  docData->setProps(props);
  docData->setDate(date);
  return docData;
}
} // namespace org::apache::lucene::benchmark::byTask::feeds