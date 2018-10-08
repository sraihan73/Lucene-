using namespace std;

#include "PatternParser.h"
#include "Hyphen.h"
#include "PatternConsumer.h"

namespace org::apache::lucene::analysis::compound::hyphenation
{
using org::xml::sax::Attributes;
using org::xml::sax::InputSource;
using org::xml::sax::SAXException;
using org::xml::sax::SAXParseException;
using org::xml::sax::XMLReader;
using org::xml::sax::helpers::DefaultHandler;

PatternParser::PatternParser()
{
  token = make_shared<StringBuilder>();
  parser = createParser();
  // C++ TODO: You cannot use 'shared_from_this' in a constructor:
  parser->setContentHandler(shared_from_this());
  // C++ TODO: You cannot use 'shared_from_this' in a constructor:
  parser->setErrorHandler(shared_from_this());
  // C++ TODO: You cannot use 'shared_from_this' in a constructor:
  parser->setEntityResolver(shared_from_this());
  hyphenChar = L'-'; // default
}

PatternParser::PatternParser(shared_ptr<PatternConsumer> consumer)
    : PatternParser()
{
  this->consumer = consumer;
}

void PatternParser::setConsumer(shared_ptr<PatternConsumer> consumer)
{
  this->consumer = consumer;
}

void PatternParser::parse(const wstring &filename) 
{
  parse(make_shared<InputSource>(filename));
}

void PatternParser::parse(shared_ptr<InputSource> source) 
{
  try {
    parser->parse(source);
  } catch (const SAXException &e) {
    throw make_shared<IOException>(e);
  }
}

shared_ptr<XMLReader> PatternParser::createParser()
{
  try {
    shared_ptr<SAXParserFactory> factory = SAXParserFactory::newInstance();
    factory->setNamespaceAware(true);
    return factory->newSAXParser().getXMLReader();
  } catch (const runtime_error &e) {
    throw runtime_error(L"Couldn't create XMLReader: " + e.what());
  }
}

wstring PatternParser::readToken(shared_ptr<StringBuilder> chars)
{
  wstring word;
  bool space = false;
  int i;
  for (i = 0; i < chars->length(); i++) {
    if (isspace(chars->charAt(i))) {
      space = true;
    } else {
      break;
    }
  }
  if (space) {
    // chars.delete(0,i);
    for (int countr = i; countr < chars->length(); countr++) {
      chars->setCharAt(countr - i, chars->charAt(countr));
    }
    chars->setLength(chars->length() - i);
    if (token->length() > 0) {
      word = token->toString();
      token->setLength(0);
      return word;
    }
  }
  space = false;
  for (i = 0; i < chars->length(); i++) {
    if (isspace(chars->charAt(i))) {
      space = true;
      break;
    }
  }
  token->append(chars->toString()->substr(0, i));
  // chars.delete(0,i);
  for (int countr = i; countr < chars->length(); countr++) {
    chars->setCharAt(countr - i, chars->charAt(countr));
  }
  chars->setLength(chars->length() - i);
  if (space) {
    word = token->toString();
    token->setLength(0);
    return word;
  }
  token->append(chars);
  return L"";
}

wstring PatternParser::getPattern(const wstring &word)
{
  shared_ptr<StringBuilder> pat = make_shared<StringBuilder>();
  int len = word.length();
  for (int i = 0; i < len; i++) {
    if (!isdigit(word[i])) {
      pat->append(word[i]);
    }
  }
  return pat->toString();
}

template <typename T1>
deque<any> PatternParser::normalizeException(deque<T1> ex)
{
  deque<any> res = deque<any>();
  for (int i = 0; i < ex.size(); i++) {
    any item = ex[i];
    if (dynamic_cast<wstring>(item) != nullptr) {
      wstring str = any_cast<wstring>(item);
      shared_ptr<StringBuilder> buf = make_shared<StringBuilder>();
      for (int j = 0; j < str.length(); j++) {
        wchar_t c = str[j];
        if (c != hyphenChar) {
          buf->append(c);
        } else {
          res.push_back(buf->toString());
          buf->setLength(0);
          std::deque<wchar_t> h(1);
          h[0] = hyphenChar;
          // we use here hyphenChar which is not necessarily
          // the one to be printed
          res.push_back(make_shared<Hyphen>(wstring(h), nullptr, nullptr));
        }
      }
      if (buf->length() > 0) {
        res.push_back(buf->toString());
      }
    } else {
      res.push_back(item);
    }
  }
  return res;
}

template <typename T1>
wstring PatternParser::getExceptionWord(deque<T1> ex)
{
  shared_ptr<StringBuilder> res = make_shared<StringBuilder>();
  for (int i = 0; i < ex.size(); i++) {
    any item = ex[i];
    if (dynamic_cast<wstring>(item) != nullptr) {
      res->append(any_cast<wstring>(item));
    } else {
      if ((any_cast<std::shared_ptr<Hyphen>>(item)).noBreak != L"") {
        res->append((any_cast<std::shared_ptr<Hyphen>>(item)).noBreak);
      }
    }
  }
  return res->toString();
}

wstring PatternParser::getInterletterValues(const wstring &pat)
{
  shared_ptr<StringBuilder> il = make_shared<StringBuilder>();
  wstring word = pat + L"a"; // add dummy letter to serve as sentinel
  int len = word.length();
  for (int i = 0; i < len; i++) {
    wchar_t c = word[i];
    if (isdigit(c)) {
      il->append(c);
      i++;
    } else {
      il->append(L'0');
    }
  }
  return il->toString();
}

shared_ptr<InputSource> PatternParser::resolveEntity(const wstring &publicId,
                                                     const wstring &systemId)
{
  // supply the internal hyphenation.dtd if possible
  if ((systemId != L"" &&
       regex_match(systemId, regex(L"(?i).*\\bhyphenation.dtd\\b.*"))) ||
      (L"hyphenation-info" == publicId)) {
    // System.out.println(this.getClass().getResource("hyphenation.dtd").toExternalForm());
    return make_shared<InputSource>(
        this->getClass().getResource(L"hyphenation.dtd").toExternalForm());
  }
  return nullptr;
}

void PatternParser::startElement(const wstring &uri, const wstring &local,
                                 const wstring &raw,
                                 shared_ptr<Attributes> attrs)
{
  if (local == L"hyphen-char") {
    wstring h = attrs->getValue(L"value");
    if (h != L"" && h.length() == 1) {
      hyphenChar = h[0];
    }
  } else if (local == L"classes") {
    currElement = ELEM_CLASSES;
  } else if (local == L"patterns") {
    currElement = ELEM_PATTERNS;
  } else if (local == L"exceptions") {
    currElement = ELEM_EXCEPTIONS;
    exception = deque<any>();
  } else if (local == L"hyphen") {
    if (token->length() > 0) {
      exception.push_back(token->toString());
    }
    exception.push_back(make_shared<Hyphen>(attrs->getValue(L"pre"),
                                            attrs->getValue(L"no"),
                                            attrs->getValue(L"post")));
    currElement = ELEM_HYPHEN;
  }
  token->setLength(0);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Override @SuppressWarnings({"unchecked", "rawtypes"}) public
// void endElement(std::wstring uri, std::wstring local, std::wstring raw)
void PatternParser::endElement(const wstring &uri, const wstring &local,
                               const wstring &raw)
{

  if (token->length() > 0) {
    wstring word = token->toString();
    switch (currElement) {
    case ELEM_CLASSES:
      consumer->addClass(word);
      break;
    case ELEM_EXCEPTIONS:
      exception.push_back(word);
      exception = normalizeException(exception);
      consumer->addException(getExceptionWord(exception),
                             static_cast<deque>(exception.clone()));
      break;
    case ELEM_PATTERNS:
      consumer->addPattern(getPattern(word), getInterletterValues(word));
      break;
    case ELEM_HYPHEN:
      // nothing to do
      break;
    }
    if (currElement != ELEM_HYPHEN) {
      token->setLength(0);
    }
  }
  if (currElement == ELEM_HYPHEN) {
    currElement = ELEM_EXCEPTIONS;
  } else {
    currElement = 0;
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings({"unchecked", "rawtypes"}) @Override public
// void characters(char ch[], int start, int length)
void PatternParser::characters(std::deque<wchar_t> &ch, int start, int length)
{
  shared_ptr<StringBuilder> chars = make_shared<StringBuilder>(length);
  chars->append(ch, start, length);
  wstring word = readToken(chars);
  while (word != L"") {
    // System.out.println("\"" + word + "\"");
    switch (currElement) {
    case ELEM_CLASSES:
      consumer->addClass(word);
      break;
    case ELEM_EXCEPTIONS:
      exception.push_back(word);
      exception = normalizeException(exception);
      consumer->addException(getExceptionWord(exception),
                             static_cast<deque>(exception.clone()));
      exception.clear();
      break;
    case ELEM_PATTERNS:
      consumer->addPattern(getPattern(word), getInterletterValues(word));
      break;
    }
    word = readToken(chars);
  }
}

wstring PatternParser::getLocationString(shared_ptr<SAXParseException> ex)
{
  shared_ptr<StringBuilder> str = make_shared<StringBuilder>();

  wstring systemId = ex->getSystemId();
  if (systemId != L"") {
    int index = (int)systemId.rfind(L'/');
    if (index != -1) {
      systemId = systemId.substr(index + 1);
    }
    str->append(systemId);
  }
  str->append(L':');
  str->append(ex->getLineNumber());
  str->append(L':');
  str->append(ex->getColumnNumber());

  return str->toString();

} // getLocationString(SAXParseException):std::wstring
} // namespace org::apache::lucene::analysis::compound::hyphenation