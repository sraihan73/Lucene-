using namespace std;

#include "SegmentCommitInfo.h"

namespace org::apache::lucene::index
{

SegmentCommitInfo::SegmentCommitInfo(shared_ptr<SegmentInfo> info, int delCount,
                                     int softDelCount, int64_t delGen,
                                     int64_t fieldInfosGen,
                                     int64_t docValuesGen)
    : info(info)
{
  this->delCount = delCount;
  this->softDelCount = softDelCount;
  this->delGen = delGen;
  this->nextWriteDelGen = delGen == -1 ? 1 : delGen + 1;
  this->fieldInfosGen = fieldInfosGen;
  this->nextWriteFieldInfosGen = fieldInfosGen == -1 ? 1 : fieldInfosGen + 1;
  this->docValuesGen = docValuesGen;
  this->nextWriteDocValuesGen = docValuesGen == -1 ? 1 : docValuesGen + 1;
}

unordered_map<int, Set<wstring>> SegmentCommitInfo::getDocValuesUpdatesFiles()
{
  return dvUpdatesFiles;
}

void SegmentCommitInfo::setDocValuesUpdatesFiles(
    unordered_map<int, Set<wstring>> &dvUpdatesFiles)
{
  this->dvUpdatesFiles.clear();
  for (auto kv : dvUpdatesFiles) {
    // rename the set
    shared_ptr<Set<wstring>> set = unordered_set<wstring>();
    for (wstring file : kv.second) {
      set->add(info->namedForThisSegment(file));
    }
    this->dvUpdatesFiles.emplace(kv.first, set);
  }
}

shared_ptr<Set<wstring>> SegmentCommitInfo::getFieldInfosFiles()
{
  return Collections::unmodifiableSet(fieldInfosFiles);
}

void SegmentCommitInfo::setFieldInfosFiles(
    shared_ptr<Set<wstring>> fieldInfosFiles)
{
  this->fieldInfosFiles->clear();
  for (auto file : fieldInfosFiles) {
    this->fieldInfosFiles->add(info->namedForThisSegment(file));
  }
}

void SegmentCommitInfo::advanceDelGen()
{
  delGen = nextWriteDelGen;
  nextWriteDelGen = delGen + 1;
  sizeInBytes_ = -1;
}

void SegmentCommitInfo::advanceNextWriteDelGen() { nextWriteDelGen++; }

int64_t SegmentCommitInfo::getNextWriteDelGen() { return nextWriteDelGen; }

void SegmentCommitInfo::setNextWriteDelGen(int64_t v) { nextWriteDelGen = v; }

void SegmentCommitInfo::advanceFieldInfosGen()
{
  fieldInfosGen = nextWriteFieldInfosGen;
  nextWriteFieldInfosGen = fieldInfosGen + 1;
  sizeInBytes_ = -1;
}

void SegmentCommitInfo::advanceNextWriteFieldInfosGen()
{
  nextWriteFieldInfosGen++;
}

int64_t SegmentCommitInfo::getNextWriteFieldInfosGen()
{
  return nextWriteFieldInfosGen;
}

void SegmentCommitInfo::setNextWriteFieldInfosGen(int64_t v)
{
  nextWriteFieldInfosGen = v;
}

void SegmentCommitInfo::advanceDocValuesGen()
{
  docValuesGen = nextWriteDocValuesGen;
  nextWriteDocValuesGen = docValuesGen + 1;
  sizeInBytes_ = -1;
}

void SegmentCommitInfo::advanceNextWriteDocValuesGen()
{
  nextWriteDocValuesGen++;
}

int64_t SegmentCommitInfo::getNextWriteDocValuesGen()
{
  return nextWriteDocValuesGen;
}

void SegmentCommitInfo::setNextWriteDocValuesGen(int64_t v)
{
  nextWriteDocValuesGen = v;
}

int64_t SegmentCommitInfo::sizeInBytes() 
{
  if (sizeInBytes_ == -1) {
    int64_t sum = 0;
    for (auto fileName : files()) {
      sum += info->dir->fileLength(fileName);
    }
    sizeInBytes_ = sum;
  }

  return sizeInBytes_;
}

shared_ptr<deque<wstring>> SegmentCommitInfo::files() 
{
  // Start from the wrapped info's files:
  shared_ptr<deque<wstring>> files = unordered_set<wstring>(info->files());

  // TODO we could rely on TrackingDir.getCreatedFiles() (like we do for
  // updates) and then maybe even be able to remove LiveDocsFormat.files().

  // Must separately add any live docs files:
  info->getCodec()->liveDocsFormat()->files(shared_from_this(), files);

  // must separately add any field updates files
  for (auto updatefiles : dvUpdatesFiles) {
    files->addAll(updatefiles->second);
  }

  // must separately add fieldInfos files
  files->addAll(fieldInfosFiles);

  return files;
}

int64_t SegmentCommitInfo::getBufferedDeletesGen()
{
  return bufferedDeletesGen;
}

void SegmentCommitInfo::setBufferedDeletesGen(int64_t v)
{
  if (bufferedDeletesGen == -1) {
    bufferedDeletesGen = v;
    sizeInBytes_ = -1;
  } else {
    throw make_shared<IllegalStateException>(
        L"buffered deletes gen should only be set once");
  }
}

bool SegmentCommitInfo::hasDeletions() { return delGen != -1; }

bool SegmentCommitInfo::hasFieldUpdates() { return fieldInfosGen != -1; }

int64_t SegmentCommitInfo::getNextFieldInfosGen()
{
  return nextWriteFieldInfosGen;
}

int64_t SegmentCommitInfo::getFieldInfosGen() { return fieldInfosGen; }

int64_t SegmentCommitInfo::getNextDocValuesGen()
{
  return nextWriteDocValuesGen;
}

int64_t SegmentCommitInfo::getDocValuesGen() { return docValuesGen; }

int64_t SegmentCommitInfo::getNextDelGen() { return nextWriteDelGen; }

int64_t SegmentCommitInfo::getDelGen() { return delGen; }

int SegmentCommitInfo::getDelCount() { return delCount; }

int SegmentCommitInfo::getSoftDelCount() { return softDelCount; }

void SegmentCommitInfo::setDelCount(int delCount)
{
  if (delCount < 0 || delCount > info->maxDoc()) {
    throw invalid_argument(L"invalid delCount=" + to_wstring(delCount) +
                           L" (maxDoc=" + to_wstring(info->maxDoc()) + L")");
  }
  assert(softDelCount + delCount <= info->maxDoc());
  this->delCount = delCount;
}

void SegmentCommitInfo::setSoftDelCount(int softDelCount)
{
  if (softDelCount < 0 || softDelCount > info->maxDoc()) {
    throw invalid_argument(L"invalid softDelCount=" + to_wstring(softDelCount) +
                           L" (maxDoc=" + to_wstring(info->maxDoc()) + L")");
  }
  assert(softDelCount + delCount <= info->maxDoc());
  this->softDelCount = softDelCount;
}

wstring SegmentCommitInfo::toString(int pendingDelCount)
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring s = info->toString(delCount + pendingDelCount);
  if (delGen != -1) {
    s += L":delGen=" + to_wstring(delGen);
  }
  if (fieldInfosGen != -1) {
    s += L":fieldInfosGen=" + to_wstring(fieldInfosGen);
  }
  if (docValuesGen != -1) {
    s += L":dvGen=" + to_wstring(docValuesGen);
  }
  if (softDelCount > 0) {
    s += L" :softDel=" + to_wstring(softDelCount);
  }

  return s;
}

wstring SegmentCommitInfo::toString() { return toString(0); }

shared_ptr<SegmentCommitInfo> SegmentCommitInfo::clone()
{
  shared_ptr<SegmentCommitInfo> other = make_shared<SegmentCommitInfo>(
      info, delCount, softDelCount, delGen, fieldInfosGen, docValuesGen);
  // Not clear that we need to carry over nextWriteDelGen
  // (i.e. do we ever clone after a failed write and
  // before the next successful write?), but just do it to
  // be safe:
  other->nextWriteDelGen = nextWriteDelGen;
  other->nextWriteFieldInfosGen = nextWriteFieldInfosGen;
  other->nextWriteDocValuesGen = nextWriteDocValuesGen;

  // deep clone
  for (auto e : dvUpdatesFiles) {
    other->dvUpdatesFiles.emplace(e.first, unordered_set<>(e.second));
  }

  other->fieldInfosFiles->addAll(fieldInfosFiles);

  return other;
}

int SegmentCommitInfo::getDelCount(bool includeSoftDeletes)
{
  return includeSoftDeletes ? getDelCount() + getSoftDelCount() : getDelCount();
}
} // namespace org::apache::lucene::index