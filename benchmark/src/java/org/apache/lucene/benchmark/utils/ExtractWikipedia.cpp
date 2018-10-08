using namespace std;

#include "ExtractWikipedia.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/document/Document.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../byTask/feeds/ContentSource.h"
#include "../byTask/feeds/DocMaker.h"
#include "../byTask/feeds/EnwikiContentSource.h"
#include "../byTask/feeds/NoMoreDataException.h"
#include "../byTask/utils/Config.h"

namespace org::apache::lucene::benchmark::utils
{
using ContentSource =
    org::apache::lucene::benchmark::byTask::feeds::ContentSource;
using DocMaker = org::apache::lucene::benchmark::byTask::feeds::DocMaker;
using EnwikiContentSource =
    org::apache::lucene::benchmark::byTask::feeds::EnwikiContentSource;
using NoMoreDataException =
    org::apache::lucene::benchmark::byTask::feeds::NoMoreDataException;
using Config = org::apache::lucene::benchmark::byTask::utils::Config;
using Document = org::apache::lucene::document::Document;
using IOUtils = org::apache::lucene::util::IOUtils;
int ExtractWikipedia::count = 0;

ExtractWikipedia::ExtractWikipedia(
    shared_ptr<DocMaker> docMaker,
    shared_ptr<Path> outputDir) 
{
  this->outputDir = outputDir;
  this->docMaker = docMaker;
  wcout << L"Deleting all files in " << outputDir << endl;
  IOUtils::rm({outputDir});
}

shared_ptr<Path> ExtractWikipedia::directory(int count,
                                             shared_ptr<Path> directory)
{
  if (directory == nullptr) {
    directory = outputDir;
  }
  int base = BASE;
  while (base <= count) {
    base *= BASE;
  }
  if (count < BASE) {
    return directory;
  }
  // C++ TODO: There is no native C++ equivalent to 'toString':
  directory = directory->resolve(Integer::toString(base / BASE));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  directory = directory->resolve(Integer::toString(count / (base / BASE)));
  return this->directory(count % (base / BASE), directory);
}

void ExtractWikipedia::create(const wstring &id, const wstring &title,
                              const wstring &time,
                              const wstring &body) 
{

  shared_ptr<Path> d = directory(count++, nullptr);
  Files::createDirectories(d);
  shared_ptr<Path> f = d->resolve(id + L".txt");

  shared_ptr<StringBuilder> contents = make_shared<StringBuilder>();

  contents->append(time);
  contents->append(L"\n\n");
  contents->append(title);
  contents->append(L"\n\n");
  contents->append(body);
  contents->append(L"\n");

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.Writer writer =
  // java.nio.file.Files.newBufferedWriter(f,
  // java.nio.charset.StandardCharsets.UTF_8))
  {
    java::io::Writer writer = java::nio::file::Files::newBufferedWriter(
        f, java::nio::charset::StandardCharsets::UTF_8);
    writer.write(contents->toString());
  }
}

void ExtractWikipedia::extract() 
{
  shared_ptr<Document> doc;
  wcout << L"Starting Extraction" << endl;
  int64_t start = System::currentTimeMillis();
  try {
    while ((doc = docMaker->makeDocument())->size() > 0) {
      create(doc[DocMaker::ID_FIELD], doc[DocMaker::TITLE_FIELD],
             doc[DocMaker::DATE_FIELD], doc[DocMaker::BODY_FIELD]);
    }
  } catch (const NoMoreDataException &e) {
    // continue
  }
  int64_t finish = System::currentTimeMillis();
  wcout << L"Extraction took " << (finish - start) << L" ms" << endl;
}

void ExtractWikipedia::main(std::deque<wstring> &args) 
{

  shared_ptr<Path> wikipedia = nullptr;
  shared_ptr<Path> outputDir = Paths->get(L"enwiki");
  bool keepImageOnlyDocs = true;
  for (int i = 0; i < args.size(); i++) {
    wstring arg = args[i];
    if (arg == L"--input" || arg == L"-i") {
      wikipedia = Paths->get(args[i + 1]);
      i++;
    } else if (arg == L"--output" || arg == L"-o") {
      outputDir = Paths->get(args[i + 1]);
      i++;
    } else if (arg == L"--discardImageOnlyDocs" || arg == L"-d") {
      keepImageOnlyDocs = false;
    }
  }

  shared_ptr<Properties> properties = make_shared<Properties>();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  properties->setProperty(L"docs.file",
                          wikipedia->toAbsolutePath()->toString());
  properties->setProperty(L"content.source.forever", L"false");
  properties->setProperty(L"keep.image.only.docs",
                          StringHelper::toString(keepImageOnlyDocs));
  shared_ptr<Config> config = make_shared<Config>(properties);

  shared_ptr<ContentSource> source = make_shared<EnwikiContentSource>();
  source->setConfig(config);

  shared_ptr<DocMaker> docMaker = make_shared<DocMaker>();
  docMaker->setConfig(config, source);
  docMaker->resetInputs();
  if (Files::exists(wikipedia)) {
    wcout << L"Extracting Wikipedia to: " << outputDir
          << L" using EnwikiContentSource" << endl;
    Files::createDirectories(outputDir);
    shared_ptr<ExtractWikipedia> extractor =
        make_shared<ExtractWikipedia>(docMaker, outputDir);
    extractor->extract();
  } else {
    printUsage();
  }
}

void ExtractWikipedia::printUsage()
{
  System::err::println(
      wstring(L"Usage: java -cp <...> "
              L"org.apache.lucene.benchmark.utils.ExtractWikipedia --input|-i "
              L"<Path to Wikipedia XML file> ") +
      L"[--output|-o <Output Path>] [--discardImageOnlyDocs|-d]");
  System::err::println(L"--discardImageOnlyDocs tells the extractor to skip "
                       L"Wiki docs that contain only images");
}
} // namespace org::apache::lucene::benchmark::utils