using namespace std;

#include "ReutersContentSource.h"
#include "../utils/Config.h"
#include "DocData.h"
#include "NoMoreDataException.h"

namespace org::apache::lucene::benchmark::byTask::feeds
{
using Config = org::apache::lucene::benchmark::byTask::utils::Config;

void ReutersContentSource::setConfig(shared_ptr<Config> config)
{
  ContentSource::setConfig(config);
  shared_ptr<Path> workDir = Paths->get(config->get(L"work.dir", L"work"));
  wstring d = config->get(L"docs.dir", L"reuters-out");
  dataDir = Paths->get(d);
  if (!dataDir->isAbsolute()) {
    dataDir = workDir->resolve(d);
  }
  inputFiles.clear();
  try {
    collectFiles(dataDir, inputFiles);
  } catch (const IOException &e) {
    throw runtime_error(e);
  }
  if (inputFiles.empty()) {
    throw runtime_error(L"No txt files in dataDir: " +
                        dataDir->toAbsolutePath());
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<DateFormatInfo> ReutersContentSource::getDateFormatInfo()
{
  shared_ptr<DateFormatInfo> dfi = dateFormat->get();
  if (dfi == nullptr) {
    dfi = make_shared<DateFormatInfo>();
    // date format: 30-MAR-1987 14:22:36.87
    dfi->df = make_shared<SimpleDateFormat>(L"dd-MMM-yyyy kk:mm:ss.SSS",
                                            Locale::ENGLISH);
    dfi->df->setLenient(true);
    dfi->pos = make_shared<ParsePosition>(0);
    dateFormat->set(dfi);
  }
  return dfi;
}

Date ReutersContentSource::parseDate(const wstring &dateStr)
{
  shared_ptr<DateFormatInfo> dfi = getDateFormatInfo();
  dfi->pos->setIndex(0);
  dfi->pos->setErrorIndex(-1);
  return dfi->df->parse(StringHelper::trim(dateStr), dfi->pos);
}

ReutersContentSource::~ReutersContentSource()
{
  // TODO implement?
}

shared_ptr<DocData> ReutersContentSource::getNextDocData(
    shared_ptr<DocData> docData) 
{
  shared_ptr<Path> f = nullptr;
  wstring name = L"";
  // C++ TODO: Multithread locking on 'this' is not converted to native C++:
  synchronized(shared_from_this())
  {
    if (nextFile >= inputFiles.size()) {
      // exhausted files, start a new round, unless forever set to false.
      if (!forever) {
        throw make_shared<NoMoreDataException>();
      }
      nextFile = 0;
      iteration++;
    }
    f = inputFiles[nextFile++];
    name = f->toRealPath() + L"_" + to_wstring(iteration);
  }

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.BufferedReader reader =
  // java.nio.file.Files.newBufferedReader(f,
  // java.nio.charset.StandardCharsets.UTF_8))
  {
    java::io::BufferedReader reader = java::nio::file::Files::newBufferedReader(
        f, java::nio::charset::StandardCharsets::UTF_8);
    // First line is the date, 3rd is the title, rest is body
    wstring dateStr = reader.readLine();
    reader.readLine(); // skip an empty line
    wstring title = reader.readLine();
    reader.readLine(); // skip an empty line
    shared_ptr<StringBuilder> bodyBuf = make_shared<StringBuilder>(1024);
    wstring line = L"";
    while ((line = reader.readLine()) != L"") {
      bodyBuf->append(line)->append(L' ');
    }

    addBytes(Files->size(f));

    Date date = parseDate(StringHelper::trim(dateStr));

    docData->clear();
    docData->setName(name);
    docData->setBody(bodyBuf->toString());
    docData->setTitle(title);
    docData->setDate(date);
    return docData;
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
void ReutersContentSource::resetInputs() 
{
  ContentSource::resetInputs();
  nextFile = 0;
  iteration = 0;
}
} // namespace org::apache::lucene::benchmark::byTask::feeds