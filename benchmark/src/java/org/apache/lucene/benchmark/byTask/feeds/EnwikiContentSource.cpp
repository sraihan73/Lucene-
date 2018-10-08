using namespace std;

#include "EnwikiContentSource.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/ThreadInterruptedException.h"
#include "../utils/Config.h"
#include "../utils/StreamUtils.h"
#include "DocData.h"
#include "NoMoreDataException.h"

namespace org::apache::lucene::benchmark::byTask::feeds
{
using Config = org::apache::lucene::benchmark::byTask::utils::Config;
using StreamUtils = org::apache::lucene::benchmark::byTask::utils::StreamUtils;
using IOUtils = org::apache::lucene::util::IOUtils;
using ThreadInterruptedException =
    org::apache::lucene::util::ThreadInterruptedException;
using org::xml::sax::Attributes;
using org::xml::sax::InputSource;
using org::xml::sax::SAXException;
using org::xml::sax::XMLReader;
using org::xml::sax::helpers::DefaultHandler;
using org::xml::sax::helpers::XMLReaderFactory;

EnwikiContentSource::Parser::Parser(
    shared_ptr<EnwikiContentSource> outerInstance)
    : outerInstance(outerInstance)
{
}

std::deque<wstring>
EnwikiContentSource::Parser::next() 
{
  if (t == nullptr) {
    threadDone = false;
    t = make_shared<Thread>(shared_from_this());
    t->setDaemon(true);
    t->start();
  }
  std::deque<wstring> result;
  // C++ TODO: Multithread locking on 'this' is not converted to native C++:
  synchronized(shared_from_this())
  {
    while (tuple.empty() && nmde == nullptr && !threadDone && !stopped) {
      try {
        wait();
      } catch (const InterruptedException &ie) {
        throw make_shared<ThreadInterruptedException>(ie);
      }
    }
    if (tuple.size() > 0) {
      result = tuple;
      tuple.clear();
      notify();
      return result;
    }
    if (nmde != nullptr) {
      // Set to null so we will re-start thread in case
      // we are re-used:
      t.reset();
      throw nmde;
    }
    // The thread has exited yet did not hit end of
    // data, so this means it hit an exception.  We
    // throw NoMorDataException here to force
    // benchmark to stop the current alg:
    throw make_shared<NoMoreDataException>();
  }
}

wstring EnwikiContentSource::Parser::time(const wstring &original)
{
  shared_ptr<StringBuilder> buffer = make_shared<StringBuilder>();

  buffer->append(original.substr(8, 2));
  buffer->append(L'-');
  buffer->append(months[stoi(original.substr(5, 2)) - 1]);
  buffer->append(L'-');
  buffer->append(original.substr(0, 4));
  buffer->append(L' ');
  buffer->append(original.substr(11, 8));
  buffer->append(L".000");

  return buffer->toString();
}

void EnwikiContentSource::Parser::characters(std::deque<wchar_t> &ch,
                                             int start, int length)
{
  contents->append(ch, start, length);
}

void EnwikiContentSource::Parser::endElement(
    const wstring &namespace_, const wstring &simple,
    const wstring &qualified) 
{
  int elemType = getElementType(qualified);
  switch (elemType) {
  case PAGE:
    // the body must be null and we either are keeping image docs or the
    // title does not start with Image:
    if (body != L"" && (outerInstance->keepImages ||
                        !StringHelper::startsWith(title, L"Image:"))) {
      std::deque<wstring> tmpTuple(LENGTH);
      tmpTuple[TITLE] = StringHelper::replace(title, L'\t', L' ');
      tmpTuple[DATE] = StringHelper::replace(time_, L'\t', L' ');
      tmpTuple[BODY] = body.replaceAll(L"[\t\n]", L" ");
      tmpTuple[ID] = id;
      // C++ TODO: Multithread locking on 'this' is not converted to native C++:
      synchronized(shared_from_this())
      {
        while (tuple.size() > 0 && !stopped) {
          try {
            wait();
          } catch (const InterruptedException &ie) {
            throw make_shared<ThreadInterruptedException>(ie);
          }
        }
        tuple = tmpTuple;
        notify();
      }
    }
    break;
  case BODY: {
    body = contents->toString();
    // workaround that startswith doesn't have an ignore case option, get at
    // least 20 chars.
    wstring startsWith =
        body.substr(0, min(10, contents->length()))->toLowerCase(Locale::ROOT);
    if (StringHelper::startsWith(startsWith, L"#redirect")) {
      body = L"";
    }
    break;
  }
  case DATE:
    time_ = time(contents->toString());
    break;
  case TITLE:
    title = contents->toString();
    break;
  case ID:
    // the doc id is the first one in the page.  All other ids after that one
    // can be ignored according to the schema
    if (id == L"") {
      id = contents->toString();
    }
    break;
  default:
    // this element should be discarded.
  }
}

void EnwikiContentSource::Parser::run()
{

  try {
    shared_ptr<XMLReader> reader = XMLReaderFactory::createXMLReader();
    reader->setContentHandler(shared_from_this());
    reader->setErrorHandler(shared_from_this());
    while (!stopped) {
      shared_ptr<InputStream> *const localFileIS = outerInstance->is;
      if (localFileIS != nullptr) { // null means fileIS was closed on us
        try {
          // To work around a bug in XERCES (XERCESJ-1257), we assume the XML is
          // always UTF8, so we simply provide reader.
          reader->parse(make_shared<InputSource>(IOUtils::getDecodingReader(
              localFileIS, StandardCharsets::UTF_8)));
        } catch (const IOException &ioe) {
          {
            lock_guard<mutex> lock(outerInstance);
            if (localFileIS != outerInstance->is) {
              // fileIS was closed on us, so, just fall through
            } else {
              // Exception is real
              throw ioe;
            }
          }
        }
      }
      // C++ TODO: Multithread locking on 'this' is not converted to native C++:
      synchronized(shared_from_this())
      {
        if (stopped || !outerInstance->forever) {
          nmde = make_shared<NoMoreDataException>();
          notify();
          return;
        } else if (localFileIS == outerInstance->is) {
          // If file is not already re-opened then re-open it now
          outerInstance->is = outerInstance->openInputStream();
        }
      }
    }
  }
  // C++ TODO: There is no equivalent in C++ to Java 'multi-catch' syntax:
  catch (SAXException | IOException sae) {
    throw runtime_error(sae);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    // C++ TODO: Multithread locking on 'this' is not converted to native C++:
    synchronized(shared_from_this())
    {
      threadDone = true;
      notify();
    }
  }
}

void EnwikiContentSource::Parser::startElement(
    const wstring &namespace_, const wstring &simple, const wstring &qualified,
    shared_ptr<Attributes> attributes)
{
  int elemType = getElementType(qualified);
  switch (elemType) {
  case PAGE:
    title = L"";
    body = L"";
    time_ = L"";
    id = L"";
    break;
  // intentional fall-through.
  case BODY:
  case DATE:
  case TITLE:
  case ID:
    contents->setLength(0);
    break;
  default:
    // this element should be discarded.
  }
}

void EnwikiContentSource::Parser::stop()
{
  // C++ TODO: Multithread locking on 'this' is not converted to native C++:
  synchronized(shared_from_this())
  {
    stopped = true;
    if (tuple.size() > 0) {
      tuple.clear();
      notify();
    }
  }
}

const unordered_map<wstring, int> EnwikiContentSource::ELEMENTS =
    unordered_map<wstring, int>();
std::deque<wstring> const EnwikiContentSource::months = {
    L"JAN", L"FEB", L"MAR", L"APR", L"MAY", L"JUN",
    L"JUL", L"AUG", L"SEP", L"OCT", L"NOV", L"DEC"};

EnwikiContentSource::StaticConstructor::StaticConstructor()
{
  ELEMENTS.emplace(L"page", static_cast<Integer>(PAGE));
  ELEMENTS.emplace(L"text", static_cast<Integer>(BODY));
  ELEMENTS.emplace(L"timestamp", static_cast<Integer>(DATE));
  ELEMENTS.emplace(L"title", static_cast<Integer>(TITLE));
  ELEMENTS.emplace(L"id", static_cast<Integer>(ID));
}

EnwikiContentSource::StaticConstructor EnwikiContentSource::staticConstructor;

int EnwikiContentSource::getElementType(const wstring &elem)
{
  optional<int> val = ELEMENTS[elem];
  return !val ? -1 : val.value();
}

EnwikiContentSource::~EnwikiContentSource()
{
  {
    lock_guard<mutex> lock(EnwikiContentSource::this);
    parser->stop();
    if (is != nullptr) {
      is->close();
      is.reset();
    }
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<DocData> EnwikiContentSource::getNextDocData(
    shared_ptr<DocData> docData) 
{
  std::deque<wstring> tuple = parser->next();
  docData->clear();
  docData->setName(tuple[ID]);
  docData->setBody(tuple[BODY]);
  docData->setDate(tuple[DATE]);
  docData->setTitle(tuple[TITLE]);
  return docData;
}

void EnwikiContentSource::resetInputs() 
{
  ContentSource::resetInputs();
  is = openInputStream();
}

shared_ptr<InputStream>
EnwikiContentSource::openInputStream() 
{
  return StreamUtils::inputStream(file);
}

void EnwikiContentSource::setConfig(shared_ptr<Config> config)
{
  ContentSource::setConfig(config);
  keepImages = config->get(L"keep.image.only.docs", true);
  wstring fileName = config->get(L"docs.file", L"");
  if (fileName != L"") {
    file = Paths->get(fileName).toAbsolutePath();
  }
}
} // namespace org::apache::lucene::benchmark::byTask::feeds