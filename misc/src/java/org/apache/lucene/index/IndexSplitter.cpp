using namespace std;

#include "IndexSplitter.h"

namespace org::apache::lucene::index
{
using FSDirectory = org::apache::lucene::store::FSDirectory;
using org::apache::lucene::util::SuppressForbidden;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressForbidden(reason = "System.out required: command line
// tool") public static void main(std::wstring[] args) throws Exception
void IndexSplitter::main(std::deque<wstring> &args) 
{
  if (args.size() < 2) {
    System::err::println(L"Usage: IndexSplitter <srcDir> -l (deque the segments "
                         L"and their sizes)");
    System::err::println(L"IndexSplitter <srcDir> <destDir> <segments>+");
    System::err::println(
        L"IndexSplitter <srcDir> -d (delete the following segments)");
    return;
  }
  shared_ptr<Path> srcDir = Paths->get(args[0]);
  shared_ptr<IndexSplitter> is = make_shared<IndexSplitter>(srcDir);
  if (!Files::exists(srcDir)) {
    throw runtime_error(L"srcdir:" + srcDir->toAbsolutePath() +
                        L" doesn't exist");
  }
  if (args[1] == L"-l") {
    is->listSegments();
  } else if (args[1] == L"-d") {
    deque<wstring> segs = deque<wstring>();
    for (int x = 2; x < args.size(); x++) {
      segs.push_back(args[x]);
    }
    is->remove(segs.toArray(std::deque<wstring>(0)));
  } else {
    shared_ptr<Path> targetDir = Paths->get(args[1]);
    deque<wstring> segs = deque<wstring>();
    for (int x = 2; x < args.size(); x++) {
      segs.push_back(args[x]);
    }
    is->split(targetDir, segs.toArray(std::deque<wstring>(0)));
  }
}

IndexSplitter::IndexSplitter(shared_ptr<Path> dir) 
    : infos(SegmentInfos::readLatestCommit(fsDir))
{
  this->dir = dir;
  fsDir = FSDirectory::open(dir);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressForbidden(reason = "System.out required: command line
// tool") public void listSegments() throws java.io.IOException
void IndexSplitter::listSegments() 
{
  shared_ptr<DecimalFormat> formatter = make_shared<DecimalFormat>(
      L"###,###.###", DecimalFormatSymbols::getInstance(Locale::ROOT));
  for (int x = 0; x < infos->size(); x++) {
    shared_ptr<SegmentCommitInfo> info = infos->info(x);
    wstring sizeStr = formatter->format(info->sizeInBytes());
    wcout << info->info->name << L" " << sizeStr << endl;
  }
}

int IndexSplitter::getIdx(const wstring &name)
{
  for (int x = 0; x < infos->size(); x++) {
    if (name == infos->info(x)->info->name) {
      return x;
    }
  }
  return -1;
}

shared_ptr<SegmentCommitInfo> IndexSplitter::getInfo(const wstring &name)
{
  for (int x = 0; x < infos->size(); x++) {
    if (name == infos->info(x)->info->name) {
      return infos->info(x);
    }
  }
  return nullptr;
}

void IndexSplitter::remove(std::deque<wstring> &segs) 
{
  for (auto n : segs) {
    int idx = getIdx(n);
    infos->erase(infos->begin() + idx);
  }
  infos->changed();
  infos->commit(fsDir);
}

void IndexSplitter::split(shared_ptr<Path> destDir,
                          std::deque<wstring> &segs) 
{
  Files::createDirectories(destDir);
  shared_ptr<FSDirectory> destFSDir = FSDirectory::open(destDir);
  shared_ptr<SegmentInfos> destInfos =
      make_shared<SegmentInfos>(infos->getIndexCreatedVersionMajor());
  destInfos->counter = infos->counter;
  for (auto n : segs) {
    shared_ptr<SegmentCommitInfo> infoPerCommit = getInfo(n);
    shared_ptr<SegmentInfo> info = infoPerCommit->info;
    // Same info just changing the dir:
    shared_ptr<SegmentInfo> newInfo = make_shared<SegmentInfo>(
        destFSDir, info->getVersion(), info->getMinVersion(), info->name,
        info->maxDoc(), info->getUseCompoundFile(), info->getCodec(),
        info->getDiagnostics(), info->getId(), unordered_map<>(), nullptr);
    destInfos->push_back(make_shared<SegmentCommitInfo>(
        newInfo, infoPerCommit->getDelCount(), infoPerCommit->getSoftDelCount(),
        infoPerCommit->getDelGen(), infoPerCommit->getFieldInfosGen(),
        infoPerCommit->getDocValuesGen()));
    // now copy files over
    shared_ptr<deque<wstring>> files = infoPerCommit->files();
    for (auto srcName : files) {
      shared_ptr<Path> srcFile = dir->resolve(srcName);
      shared_ptr<Path> destFile = destDir->resolve(srcName);
      Files::copy(srcFile, destFile);
    }
  }
  destInfos->changed();
  destInfos->commit(destFSDir);
  // System.out.println("destDir:"+destDir.getAbsolutePath());
}
} // namespace org::apache::lucene::index