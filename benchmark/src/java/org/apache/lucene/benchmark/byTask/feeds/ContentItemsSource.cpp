using namespace std;

#include "ContentItemsSource.h"
#include "../utils/Config.h"
#include "../utils/Format.h"

namespace org::apache::lucene::benchmark::byTask::feeds
{
using Config = org::apache::lucene::benchmark::byTask::utils::Config;
using Format = org::apache::lucene::benchmark::byTask::utils::Format;

// C++ WARNING: The following method was originally marked 'synchronized':
void ContentItemsSource::addBytes(int64_t numBytes)
{
  bytesCount += numBytes;
  totalBytesCount += numBytes;
}

// C++ WARNING: The following method was originally marked 'synchronized':
void ContentItemsSource::addItem()
{
  ++itemCount;
  ++totalItemCount;
}

void ContentItemsSource::collectFiles(
    shared_ptr<Path> dir,
    deque<std::shared_ptr<Path>> &files) 
{
  Files::walkFileTree(dir, make_shared<SimpleFileVisitorAnonymousInnerClass>(
                               shared_from_this(), files));
}

ContentItemsSource::SimpleFileVisitorAnonymousInnerClass::
    SimpleFileVisitorAnonymousInnerClass(
        shared_ptr<ContentItemsSource> outerInstance,
        deque<std::shared_ptr<Path>> &files)
{
  this->outerInstance = outerInstance;
  this->files = files;
}

shared_ptr<FileVisitResult>
ContentItemsSource::SimpleFileVisitorAnonymousInnerClass::visitFile(
    shared_ptr<Path> file,
    shared_ptr<BasicFileAttributes> attrs) 
{
  if (Files::isReadable(file)) {
    files.push_back(file->toRealPath());
  }
  return FileVisitResult::CONTINUE;
}

bool ContentItemsSource::shouldLog()
{
  return verbose && logStep > 0 && itemCount % logStep == 0;
}

int64_t ContentItemsSource::getBytesCount() { return bytesCount; }

int ContentItemsSource::getItemsCount() { return itemCount; }

shared_ptr<Config> ContentItemsSource::getConfig() { return config; }

int64_t ContentItemsSource::getTotalBytesCount() { return totalBytesCount; }

int ContentItemsSource::getTotalItemsCount() { return totalItemCount; }

void ContentItemsSource::resetInputs() 
{
  bytesCount = 0;
  itemCount = 0;
}

void ContentItemsSource::setConfig(shared_ptr<Config> config)
{
  this->config = config;
  forever = config->get(L"content.source.forever", true);
  logStep = config->get(L"content.source.log.step", 0);
  verbose = config->get(L"content.source.verbose", false);
  encoding = config->get(L"content.source.encoding", L"");
}

void ContentItemsSource::printStatistics(const wstring &itemsName)
{
  if (!verbose) {
    return;
  }
  bool print = false;
  wstring col = L"                  ";
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  wstring newline = System::getProperty(L"line.separator");
  sb->append(L"------------> ")
      ->append(getClass().getSimpleName())
      ->append(L" statistics (")
      ->append(printNum)
      ->append(L"): ")
      ->append(newline);
  int nut = getTotalItemsCount();
  if (nut > lastPrintedNumUniqueTexts) {
    print = true;
    sb->append(L"total count of " + itemsName + L": ")
        ->append(Format::format(0, nut, col))
        ->append(newline);
    lastPrintedNumUniqueTexts = nut;
  }
  int64_t nub = getTotalBytesCount();
  if (nub > lastPrintedNumUniqueBytes) {
    print = true;
    sb->append(L"total bytes of " + itemsName + L": ")
        ->append(Format::format(0, nub, col))
        ->append(newline);
    lastPrintedNumUniqueBytes = nub;
  }
  if (getItemsCount() > 0) {
    print = true;
    sb->append(L"num " + itemsName + L" added since last inputs reset:   ")
        ->append(Format::format(0, getItemsCount(), col))
        ->append(newline);
    sb->append(L"total bytes added for " + itemsName +
               L" since last inputs reset: ")
        ->append(Format::format(0, getBytesCount(), col))
        ->append(newline);
  }
  if (print) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wcout << sb->append(newline)->toString() << endl;
    printNum++;
  }
}
} // namespace org::apache::lucene::benchmark::byTask::feeds