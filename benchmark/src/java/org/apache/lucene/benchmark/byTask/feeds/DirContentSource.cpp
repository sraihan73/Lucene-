using namespace std;

#include "DirContentSource.h"
#include "../utils/Config.h"
#include "DocData.h"
#include "NoMoreDataException.h"

namespace org::apache::lucene::benchmark::byTask::feeds
{
using Config = org::apache::lucene::benchmark::byTask::utils::Config;

int DirContentSource::Iterator::Comparator::compare(shared_ptr<Path> _a,
                                                    shared_ptr<Path> _b)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring a = _a->toString();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring b = _b->toString();
  int diff = a.length() - b.length();

  if (diff > 0) {
    while (diff-- > 0) {
      b = L"0" + b;
    }
  } else if (diff < 0) {
    diff = -diff;
    while (diff-- > 0) {
      a = L"0" + a;
    }
  }

  /* note it's reversed because we're going to push,
     which reverses again */
  return b.compare(a);
}

DirContentSource::Iterator::Iterator(shared_ptr<Path> f) 
{
  push(f);
}

void DirContentSource::Iterator::find() 
{
  if (stack.empty()) {
    return;
  }
  if (!Files::isDirectory(stack.top())) {
    return;
  }
  shared_ptr<Path> f = stack.pop();
  push(f);
}

void DirContentSource::Iterator::push(shared_ptr<Path> f) 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try
  // (java.nio.file.DirectoryStream<java.nio.file.Path> stream =
  // java.nio.file.Files.newDirectoryStream(f))
  {
    shared_ptr<java::nio::file::DirectoryStream<
        std::shared_ptr<java::nio::file::Path>>>
        stream = java::nio::file::Files::newDirectoryStream(f);
    deque<std::shared_ptr<Path>> found = deque<std::shared_ptr<Path>>();
    for (auto p : stream) {
      if (Files::isDirectory(p)) {
        found.push_back(p);
      }
    }
    push(found.toArray(std::deque<std::shared_ptr<Path>>(found.size())));
  }
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try
  // (java.nio.file.DirectoryStream<java.nio.file.Path> stream =
  // java.nio.file.Files.newDirectoryStream(f, "*.txt"))
  {
    shared_ptr<java::nio::file::DirectoryStream<
        std::shared_ptr<java::nio::file::Path>>>
        stream = java::nio::file::Files::newDirectoryStream(f, L"*.txt");
    deque<std::shared_ptr<Path>> found = deque<std::shared_ptr<Path>>();
    for (auto p : stream) {
      found.push_back(p);
    }
    push(found.toArray(std::deque<std::shared_ptr<Path>>(found.size())));
  }
  find();
}

void DirContentSource::Iterator::push(std::deque<std::shared_ptr<Path>> &files)
{
  Arrays::sort(files, c);
  for (int i = 0; i < files.size(); i++) {
    // System.err.println("push " + files[i]);
    stack.push(files[i]);
  }
}

int DirContentSource::Iterator::getCount() { return count; }

bool DirContentSource::Iterator::hasNext() { return stack.size() > 0; }

shared_ptr<Path> DirContentSource::Iterator::next()
{
  assert(hasNext());
  count++;
  shared_ptr<Path> object = stack.pop();
  // System.err.println("pop " + object);
  try {
    find();
  } catch (const IOException &e) {
    throw runtime_error(e);
  }
  return object;
}

void DirContentSource::Iterator::remove() { throw runtime_error(L"cannot"); }

shared_ptr<DateFormatInfo> DirContentSource::getDateFormatInfo()
{
  shared_ptr<DateFormatInfo> dfi = dateFormat->get();
  if (dfi == nullptr) {
    dfi = make_shared<DateFormatInfo>();
    dfi->pos = make_shared<ParsePosition>(0);
    // date format: 30-MAR-1987 14:22:36.87
    dfi->df = make_shared<SimpleDateFormat>(L"dd-MMM-yyyy kk:mm:ss.SSS",
                                            Locale::ENGLISH);
    dfi->df->setLenient(true);
    dateFormat->set(dfi);
  }
  return dfi;
}

Date DirContentSource::parseDate(const wstring &dateStr)
{
  shared_ptr<DateFormatInfo> dfi = getDateFormatInfo();
  dfi->pos->setIndex(0);
  dfi->pos->setErrorIndex(-1);
  return dfi->df->parse(StringHelper::trim(dateStr), dfi->pos);
}

DirContentSource::~DirContentSource() { inputFiles.reset(); }

shared_ptr<DocData> DirContentSource::getNextDocData(
    shared_ptr<DocData> docData) 
{
  shared_ptr<Path> f = nullptr;
  wstring name = L"";
  // C++ TODO: Multithread locking on 'this' is not converted to native C++:
  synchronized(shared_from_this())
  {
    if (!inputFiles->hasNext()) {
      // exhausted files, start a new round, unless forever set to false.
      if (!forever) {
        throw make_shared<NoMoreDataException>();
      }
      inputFiles = make_shared<Iterator>(dataDir);
      iteration++;
    }
    f = inputFiles->next();
    // System.err.println(f);
    name = f->toRealPath() + L"_" + to_wstring(iteration);
  }

  shared_ptr<BufferedReader> reader =
      Files::newBufferedReader(f, StandardCharsets::UTF_8);
  wstring line = L"";
  // First line is the date, 3rd is the title, rest is body
  wstring dateStr = reader->readLine();
  reader->readLine(); // skip an empty line
  wstring title = reader->readLine();
  reader->readLine(); // skip an empty line
  shared_ptr<StringBuilder> bodyBuf = make_shared<StringBuilder>(1024);
  while ((line = reader->readLine()) != L"") {
    bodyBuf->append(line)->append(L' ');
  }
  reader->close();
  addBytes(Files->size(f));

  Date date = parseDate(dateStr);

  docData->clear();
  docData->setName(name);
  docData->setBody(bodyBuf->toString());
  docData->setTitle(title);
  docData->setDate(date);
  return docData;
}

// C++ WARNING: The following method was originally marked 'synchronized':
void DirContentSource::resetInputs() 
{
  ContentSource::resetInputs();
  inputFiles = make_shared<Iterator>(dataDir);
  iteration = 0;
}

void DirContentSource::setConfig(shared_ptr<Config> config)
{
  ContentSource::setConfig(config);

  shared_ptr<Path> workDir = Paths->get(config->get(L"work.dir", L"work"));
  wstring d = config->get(L"docs.dir", L"dir-out");
  dataDir = Paths->get(d);
  if (!dataDir->isAbsolute()) {
    dataDir = workDir->resolve(d);
  }

  try {
    inputFiles = make_shared<Iterator>(dataDir);
  } catch (const IOException &e) {
    throw runtime_error(e);
  }

  if (inputFiles == nullptr) {
    throw runtime_error(L"No txt files in dataDir: " +
                        dataDir->toAbsolutePath());
  }
}
} // namespace org::apache::lucene::benchmark::byTask::feeds