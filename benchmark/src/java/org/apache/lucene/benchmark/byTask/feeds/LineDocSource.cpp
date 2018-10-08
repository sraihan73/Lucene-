using namespace std;

#include "LineDocSource.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../tasks/WriteLineDocTask.h"
#include "../utils/Config.h"
#include "../utils/StreamUtils.h"
#include "DocData.h"
#include "DocMaker.h"
#include "NoMoreDataException.h"

namespace org::apache::lucene::benchmark::byTask::feeds
{
using WriteLineDocTask =
    org::apache::lucene::benchmark::byTask::tasks::WriteLineDocTask;
using Config = org::apache::lucene::benchmark::byTask::utils::Config;
using StreamUtils = org::apache::lucene::benchmark::byTask::utils::StreamUtils;
using IOUtils = org::apache::lucene::util::IOUtils;

LineDocSource::LineParser::LineParser(std::deque<wstring> &header)
    : header(header)
{
}

LineDocSource::SimpleLineParser::SimpleLineParser(std::deque<wstring> &header)
    : LineParser(header)
{
}

void LineDocSource::SimpleLineParser::parseLine(shared_ptr<DocData> docData,
                                                const wstring &line)
{
  int k1 = 0;
  int k2 = (int)line.find(WriteLineDocTask::SEP, k1);
  if (k2 < 0) {
    throw runtime_error(
        L"line: [" + line +
        L"] is in an invalid format (missing: separator title::date)!");
  }
  docData->setTitle(line.substr(k1, k2 - k1));
  k1 = k2 + 1;
  k2 = (int)line.find(WriteLineDocTask::SEP, k1);
  if (k2 < 0) {
    throw runtime_error(
        L"line: [" + line +
        L"] is in an invalid format (missing: separator date::body)!");
  }
  docData->setDate(line.substr(k1, k2 - k1));
  k1 = k2 + 1;
  k2 = (int)line.find(WriteLineDocTask::SEP, k1);
  if (k2 >= 0) {
    throw runtime_error(L"line: [" + line +
                        L"] is in an invalid format (too many separators)!");
  }
  // last one
  docData->setBody(line.substr(k1));
}

LineDocSource::HeaderLineParser::HeaderLineParser(std::deque<wstring> &header)
    : LineParser(header), posToF(std::deque<FieldName>(header.size()))
{
  for (int i = 0; i < header.size(); i++) {
    wstring f = header[i];
    if (DocMaker::NAME_FIELD == f) {
      posToF[i] = FieldName::NAME;
    } else if (DocMaker::TITLE_FIELD == f) {
      posToF[i] = FieldName::TITLE;
    } else if (DocMaker::DATE_FIELD == f) {
      posToF[i] = FieldName::DATE;
    } else if (DocMaker::BODY_FIELD == f) {
      posToF[i] = FieldName::BODY;
    } else {
      posToF[i] = FieldName::PROP;
    }
  }
}

void LineDocSource::HeaderLineParser::parseLine(shared_ptr<DocData> docData,
                                                const wstring &line)
{
  int n = 0;
  int k1 = 0;
  int k2;
  while ((k2 = (int)line.find(WriteLineDocTask::SEP, k1)) >= 0) {
    if (n >= header.size()) {
      throw runtime_error(L"input line has invalid format: " +
                          to_wstring(n + 1) + L" fields instead of " +
                          header.size() + L" :: [" + line + L"]");
    }
    setDocDataField(docData, n, line.substr(k1, k2 - k1));
    ++n;
    k1 = k2 + 1;
  }
  if (n != header.size() - 1) {
    throw runtime_error(L"input line has invalid format: " + to_wstring(n + 1) +
                        L" fields instead of " + header.size() + L" :: [" +
                        line + L"]");
  }
  // last one
  setDocDataField(docData, n, line.substr(k1));
}

void LineDocSource::HeaderLineParser::setDocDataField(
    shared_ptr<DocData> docData, int position, const wstring &text)
{
  switch (posToF[position]) {
  case NAME:
    docData->setName(text);
    break;
  case TITLE:
    docData->setTitle(text);
    break;
  case DATE:
    docData->setDate(text);
    break;
  case BODY:
    docData->setBody(text);
    break;
  case PROP: {
    shared_ptr<Properties> p = docData->getProps();
    if (p == nullptr) {
      p = make_shared<Properties>();
      docData->setProps(p);
    }
    p->setProperty(header[position], text);
    break;
  }
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
void LineDocSource::openFile()
{
  try {
    if (reader != nullptr) {
      reader->close();
    }
    shared_ptr<InputStream> is = StreamUtils::inputStream(file);
    reader = make_shared<BufferedReader>(
        make_shared<InputStreamReader>(is, encoding), StreamUtils::BUFFER_SIZE);
    if (skipHeaderLine) {
      reader->readLine(); // skip one line - the header line - already handled
                          // that info
    }
  } catch (const IOException &e) {
    throw runtime_error(e);
  }
}

LineDocSource::~LineDocSource()
{
  if (reader != nullptr) {
    reader->close();
    reader.reset();
  }
}

shared_ptr<DocData> LineDocSource::getNextDocData(
    shared_ptr<DocData> docData) 
{
  const wstring line;
  constexpr int myID;

  // C++ TODO: Multithread locking on 'this' is not converted to native C++:
  synchronized(shared_from_this())
  {
    line = reader->readLine();
    if (line == L"") {
      if (!forever) {
        throw make_shared<NoMoreDataException>();
      }
      // Reset the file
      openFile();
      return getNextDocData(docData);
    }
    if (docDataLineReader ==
        nullptr) { // first line ever, one time initialization,
      docDataLineReader = createDocDataLineReader(line);
      if (skipHeaderLine) {
        return getNextDocData(docData);
      }
    }
    // increment IDS only once...
    myID = readCount++;
  }

  // The date std::wstring was written in the format of DateTools.dateToString.
  docData->clear();
  docData->setID(myID);
  docDataLineReader->parseLine(docData, line);
  return docData;
}

shared_ptr<LineParser>
LineDocSource::createDocDataLineReader(const wstring &line)
{
  std::deque<wstring> header;
  wstring headIndicator = WriteLineDocTask::FIELDS_HEADER_INDICATOR +
                          StringHelper::toString(WriteLineDocTask::SEP);

  if (StringHelper::startsWith(line, headIndicator)) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    header = line.substr(headIndicator.length())
                 ->split(Character::toString(WriteLineDocTask::SEP));
    skipHeaderLine =
        true; // mark to skip the header line when input file is reopened
  } else {
    header = WriteLineDocTask::DEFAULT_FIELDS;
  }

  // if a specific DocDataLineReader was configured, must respect it
  wstring docDataLineReaderClassName = getConfig()->get(L"line.parser", L"");
  if (docDataLineReaderClassName != L"") {
    try {
      constexpr type_info clazz = type_info::forName(docDataLineReaderClassName)
                                      .asSubclass(LineParser::typeid);
      // C++ TODO: Java wildcard generics are not converted to C++:
      // ORIGINAL LINE: Constructor<? extends LineParser> cnstr =
      // clazz.getConstructor(std::wstring[].class);
      shared_ptr < Constructor <
          ? extends LineParser >> cnstr =
                clazz.getConstructor(std::deque<wstring>::typeid);
      return cnstr->newInstance(static_cast<any>(header));
    } catch (const runtime_error &e) {
      // C++ TODO: This exception's constructor requires only one argument:
      // ORIGINAL LINE: throw new RuntimeException("Failed to instantiate
      // "+docDataLineReaderClassName, e);
      throw runtime_error(L"Failed to instantiate " +
                          docDataLineReaderClassName);
    }
  }

  // if this the simple case,
  if (Arrays::deepEquals(header, WriteLineDocTask::DEFAULT_FIELDS)) {
    return make_shared<SimpleLineParser>(header);
  }
  return make_shared<HeaderLineParser>(header);
}

void LineDocSource::resetInputs() 
{
  ContentSource::resetInputs();
  openFile();
}

void LineDocSource::setConfig(shared_ptr<Config> config)
{
  ContentSource::setConfig(config);
  wstring fileName = config->get(L"docs.file", L"");
  if (fileName == L"") {
    throw invalid_argument(L"docs.file must be set");
  }
  file = Paths->get(fileName).toAbsolutePath();
  if (encoding == L"") {
    encoding = IOUtils::UTF_8;
  }
}
} // namespace org::apache::lucene::benchmark::byTask::feeds