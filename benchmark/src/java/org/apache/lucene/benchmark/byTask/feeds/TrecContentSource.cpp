using namespace std;

#include "TrecContentSource.h"
#include "../utils/Config.h"
#include "../utils/StreamUtils.h"
#include "DocData.h"
#include "HTMLParser.h"
#include "NoMoreDataException.h"

namespace org::apache::lucene::benchmark::byTask::feeds
{
using ParsePathType =
    org::apache::lucene::benchmark::byTask::feeds::TrecDocParser::ParsePathType;
using Config = org::apache::lucene::benchmark::byTask::utils::Config;
using StreamUtils = org::apache::lucene::benchmark::byTask::utils::StreamUtils;
const wstring TrecContentSource::DOCNO = L"<DOCNO>";
const wstring TrecContentSource::TERMINATING_DOCNO = L"</DOCNO>";
const wstring TrecContentSource::DOC = L"<DOC>";
const wstring TrecContentSource::TERMINATING_DOC = L"</DOC>";
const wstring TrecContentSource::NEW_LINE =
    System::getProperty(L"line.separator");
std::deque<wstring> const TrecContentSource::DATE_FORMATS = {
    L"EEE, dd MMM yyyy kk:mm:ss z",
    L"EEE MMM dd kk:mm:ss yyyy z",
    L"EEE, dd-MMM-':'y kk:mm:ss z",
    L"EEE, dd-MMM-yyy kk:mm:ss z",
    L"EEE MMM dd kk:mm:ss yyyy",
    L"dd MMM yyyy",
    L"MMM dd, yyyy",
    L"yyMMdd",
    L"hhmm z.z.z. MMM dd, yyyy"};

shared_ptr<DateFormatInfo> TrecContentSource::getDateFormatInfo()
{
  shared_ptr<DateFormatInfo> dfi = dateFormats->get();
  if (dfi == nullptr) {
    dfi = make_shared<DateFormatInfo>();
    dfi->dfs =
        std::deque<std::shared_ptr<SimpleDateFormat>>(DATE_FORMATS.size());
    for (int i = 0; i < dfi->dfs.size(); i++) {
      dfi->dfs[i] =
          make_shared<SimpleDateFormat>(DATE_FORMATS[i], Locale::ENGLISH);
      dfi->dfs[i]->setLenient(true);
    }
    dfi->pos = make_shared<ParsePosition>(0);
    dateFormats->set(dfi);
  }
  return dfi;
}

shared_ptr<StringBuilder> TrecContentSource::getDocBuffer()
{
  shared_ptr<StringBuilder> sb = trecDocBuffer->get();
  if (sb == nullptr) {
    sb = make_shared<StringBuilder>();
    trecDocBuffer->set(sb);
  }
  return sb;
}

shared_ptr<HTMLParser> TrecContentSource::getHtmlParser() { return htmlParser; }

void TrecContentSource::read(shared_ptr<StringBuilder> buf,
                             const wstring &lineStart, bool collectMatchLine,
                             bool collectAll) throw(IOException,
                                                    NoMoreDataException)
{
  wstring sep = L"";
  while (true) {
    wstring line = reader->readLine();

    if (line == L"") {
      openNextFile();
      continue;
    }

    if (lineStart != L"" && StringHelper::startsWith(line, lineStart)) {
      if (collectMatchLine) {
        buf->append(sep)->append(line);
        sep = NEW_LINE;
      }
      return;
    }

    if (collectAll) {
      buf->append(sep)->append(line);
      sep = NEW_LINE;
    }
  }
}

void TrecContentSource::openNextFile() 
{
  close();
  currPathType = nullptr;
  while (true) {
    if (nextFile >= inputFiles.size()) {
      // exhausted files, start a new round, unless forever set to false.
      if (!forever) {
        throw make_shared<NoMoreDataException>();
      }
      nextFile = 0;
      iteration++;
    }
    shared_ptr<Path> f = inputFiles[nextFile++];
    if (verbose) {
      wcout << L"opening: " << f << L" length: " << Files->size(f) << endl;
    }
    try {
      shared_ptr<InputStream> inputStream = StreamUtils::inputStream(
          f); // support either gzip, bzip2, or regular text file, by extension
      reader = make_shared<BufferedReader>(
          make_shared<InputStreamReader>(inputStream, encoding),
          StreamUtils::BUFFER_SIZE);
      currPathType = TrecDocParser::pathType(f);
      return;
    } catch (const runtime_error &e) {
      if (verbose) {
        wcout << L"Skipping 'bad' file " << f->toAbsolutePath() << L" due to "
              << e.what() << endl;
        continue;
      }
      throw make_shared<NoMoreDataException>();
    }
  }
}

Date TrecContentSource::parseDate(const wstring &dateStr)
{
  dateStr = StringHelper::trim(dateStr);
  shared_ptr<DateFormatInfo> dfi = getDateFormatInfo();
  for (int i = 0; i < dfi->dfs.size(); i++) {
    shared_ptr<DateFormat> df = dfi->dfs[i];
    dfi->pos->setIndex(0);
    dfi->pos->setErrorIndex(-1);
    Date d = df->parse(dateStr, dfi->pos);
    if (d != nullptr) {
      // Parse succeeded.
      return d;
    }
  }
  // do not fail test just because a date could not be parsed
  if (verbose) {
    wcout << L"failed to parse date (assigning 'now') for: " << dateStr << endl;
  }
  return nullptr;
}

TrecContentSource::~TrecContentSource()
{
  if (reader == nullptr) {
    return;
  }

  try {
    reader->close();
  } catch (const IOException &e) {
    if (verbose) {
      wcout << L"failed to close reader !" << endl;
      e->printStackTrace(System::out);
    }
  }
  reader.reset();
}

shared_ptr<DocData> TrecContentSource::getNextDocData(
    shared_ptr<DocData> docData) 
{
  wstring name = L"";
  shared_ptr<StringBuilder> docBuf = getDocBuffer();
  ParsePathType parsedPathType;

  // protect reading from the TREC files by multiple threads. The rest of the
  // method, i.e., parsing the content and returning the DocData can run
  // unprotected.
  {
    lock_guard<mutex> lock(lock);
    if (reader == nullptr) {
      openNextFile();
    }

    // 1. skip until doc start - required for all TREC formats
    docBuf->setLength(0);
    read(docBuf, DOC, false, false);

    // save parsedFile for passing trecDataParser after the sync block, in
    // case another thread will open another file in between.
    parsedPathType = currPathType;

    // 2. name - required for all TREC formats
    docBuf->setLength(0);
    read(docBuf, DOCNO, true, false);
    name = docBuf
               ->substr(DOCNO.length(),
                        docBuf->find(TERMINATING_DOCNO, DOCNO.length()) -
                            DOCNO.length())
               ->trim();

    if (!excludeDocnameIteration) {
      name = name + L"_" + to_wstring(iteration);
    }

    // 3. read all until end of doc
    docBuf->setLength(0);
    read(docBuf, TERMINATING_DOC, false, true);
  }

  // count char length of text to be parsed (may be larger than the resulted
  // plain doc body text).
  addBytes(docBuf->length());

  // This code segment relies on HtmlParser being thread safe. When we get
  // here, everything else is already private to that thread, so we're safe.
  docData = trecDocParser->parse(docData, name, shared_from_this(), docBuf,
                                 parsedPathType);
  addItem();

  return docData;
}

void TrecContentSource::resetInputs() 
{
  {
    lock_guard<mutex> lock(lock);
    ContentSource::resetInputs();
    close();
    nextFile = 0;
    iteration = 0;
  }
}

void TrecContentSource::setConfig(shared_ptr<Config> config)
{
  ContentSource::setConfig(config);
  // dirs
  shared_ptr<Path> workDir = Paths->get(config->get(L"work.dir", L"work"));
  wstring d = config->get(L"docs.dir", L"trec");
  dataDir = Paths->get(d);
  if (!dataDir->isAbsolute()) {
    dataDir = workDir->resolve(d);
  }
  // files
  try {
    collectFiles(dataDir, inputFiles);
  } catch (const IOException &e) {
    throw runtime_error(e);
  }
  if (inputFiles.empty()) {
    throw invalid_argument(L"No files in dataDir: " + dataDir);
  }
  // trec doc parser
  try {
    wstring trecDocParserClassName =
        config->get(L"trec.doc.parser",
                    L"org.apache.lucene.benchmark.byTask.feeds.TrecGov2Parser");
    trecDocParser = type_info::forName(trecDocParserClassName)
                        .asSubclass(TrecDocParser::typeid)
                        .newInstance();
  } catch (const runtime_error &e) {
    // Should not get here. Throw runtime exception.
    throw runtime_error(e);
  }
  // html parser
  try {
    wstring htmlParserClassName =
        config->get(L"html.parser",
                    L"org.apache.lucene.benchmark.byTask.feeds.DemoHTMLParser");
    htmlParser = type_info::forName(htmlParserClassName)
                     .asSubclass(HTMLParser::typeid)
                     .newInstance();
  } catch (const runtime_error &e) {
    // Should not get here. Throw runtime exception.
    throw runtime_error(e);
  }
  // encoding
  if (encoding == L"") {
    encoding = StandardCharsets::ISO_8859_1::name();
  }
  // iteration exclusion in doc name
  excludeDocnameIteration =
      config->get(L"content.source.excludeIteration", false);
}
} // namespace org::apache::lucene::benchmark::byTask::feeds