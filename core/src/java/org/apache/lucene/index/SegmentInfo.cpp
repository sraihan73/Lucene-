using namespace std;

#include "SegmentInfo.h"

namespace org::apache::lucene::index
{
using Codec = org::apache::lucene::codecs::Codec;
using Sort = org::apache::lucene::search::Sort;
using Directory = org::apache::lucene::store::Directory;
using TrackingDirectoryWrapper =
    org::apache::lucene::store::TrackingDirectoryWrapper;
using StringHelper = org::apache::lucene::util::StringHelper;
using Version = org::apache::lucene::util::Version;

void SegmentInfo::setDiagnostics(unordered_map<wstring, wstring> &diagnostics)
{
  this->diagnostics = Objects::requireNonNull(diagnostics);
}

unordered_map<wstring, wstring> SegmentInfo::getDiagnostics()
{
  return diagnostics;
}

SegmentInfo::SegmentInfo(shared_ptr<Directory> dir, shared_ptr<Version> version,
                         shared_ptr<Version> minVersion, const wstring &name,
                         int maxDoc, bool isCompoundFile,
                         shared_ptr<Codec> codec,
                         unordered_map<wstring, wstring> &diagnostics,
                         std::deque<char> &id,
                         unordered_map<wstring, wstring> &attributes,
                         shared_ptr<Sort> indexSort)
    : name(Objects::requireNonNull(name)), dir(Objects::requireNonNull(dir)),
      id(id), attributes(Objects::requireNonNull(attributes)),
      indexSort(indexSort), version(Objects::requireNonNull(version))
{
  assert(
      !(std::dynamic_pointer_cast<TrackingDirectoryWrapper>(dir) != nullptr));
  this->minVersion = minVersion;
  this->maxDoc_ = maxDoc;
  this->isCompoundFile = isCompoundFile;
  this->codec = codec;
  this->diagnostics = Objects::requireNonNull(diagnostics);
  if (id.size() != StringHelper::ID_LENGTH) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    throw invalid_argument(L"invalid id: " + Arrays->toString(id));
  }
}

void SegmentInfo::setUseCompoundFile(bool isCompoundFile)
{
  this->isCompoundFile = isCompoundFile;
}

bool SegmentInfo::getUseCompoundFile() { return isCompoundFile; }

void SegmentInfo::setCodec(shared_ptr<Codec> codec)
{
  assert(this->codec == nullptr);
  if (codec == nullptr) {
    throw invalid_argument(L"codec must be non-null");
  }
  this->codec = codec;
}

shared_ptr<Codec> SegmentInfo::getCodec() { return codec; }

int SegmentInfo::maxDoc()
{
  if (this->maxDoc_ == -1) {
    throw make_shared<IllegalStateException>(L"maxDoc isn't set yet");
  }
  return maxDoc_;
}

void SegmentInfo::setMaxDoc(int maxDoc)
{
  if (this->maxDoc_ != -1) {
    throw make_shared<IllegalStateException>(
        L"maxDoc was already set: this.maxDoc=" + to_wstring(this->maxDoc_) +
        L" vs maxDoc=" + to_wstring(maxDoc));
  }
  this->maxDoc_ = maxDoc;
}

shared_ptr<Set<wstring>> SegmentInfo::files()
{
  if (setFiles_ == nullptr) {
    throw make_shared<IllegalStateException>(
        L"files were not computed yet; segment=" + name + L" maxDoc=" +
        to_wstring(maxDoc_));
  }
  return Collections::unmodifiableSet(setFiles_);
}

wstring SegmentInfo::toString() { return toString(0); }

wstring SegmentInfo::toString(int delCount)
{
  shared_ptr<StringBuilder> s = make_shared<StringBuilder>();
  s->append(name)
      ->append(L'(')
      ->append(version == nullptr ? L"?" : version)
      ->append(L')')
      ->append(L':');
  wchar_t cfs = getUseCompoundFile() ? L'c' : L'C';
  s->append(cfs);

  s->append(maxDoc_);

  if (delCount != 0) {
    s->append(L'/')->append(delCount);
  }

  if (indexSort != nullptr) {
    s->append(L":[indexSort=");
    s->append(indexSort);
    s->append(L']');
  }

  // TODO: we could append toString of attributes() here?

  return s->toString();
}

bool SegmentInfo::equals(any obj)
{
  if (shared_from_this() == obj) {
    return true;
  }
  if (std::dynamic_pointer_cast<SegmentInfo>(obj) != nullptr) {
    shared_ptr<SegmentInfo> *const other =
        any_cast<std::shared_ptr<SegmentInfo>>(obj);
    return other->dir == dir && other->name == name;
  } else {
    return false;
  }
}

int SegmentInfo::hashCode() { return dir->hashCode() + name.hashCode(); }

shared_ptr<Version> SegmentInfo::getVersion() { return version; }

shared_ptr<Version> SegmentInfo::getMinVersion() { return minVersion; }

std::deque<char> SegmentInfo::getId() { return id.clone(); }

void SegmentInfo::setFiles(shared_ptr<deque<wstring>> files)
{
  setFiles_ = unordered_set<>();
  addFiles(files);
}

void SegmentInfo::addFiles(shared_ptr<deque<wstring>> files)
{
  checkFileNames(files);
  for (auto f : files) {
    setFiles_->add(namedForThisSegment(f));
  }
}

void SegmentInfo::addFile(const wstring &file)
{
  checkFileNames(Collections::singleton(file));
  setFiles_->add(namedForThisSegment(file));
}

void SegmentInfo::checkFileNames(shared_ptr<deque<wstring>> files)
{
  shared_ptr<Matcher> m = IndexFileNames::CODEC_FILE_PATTERN->matcher(L"");
  for (auto file : files) {
    m->reset(file);
    if (!m->matches()) {
      throw invalid_argument(L"invalid codec filename '" + file +
                             L"', must match: " +
                             IndexFileNames::CODEC_FILE_PATTERN->pattern());
    }
    if (file.toLowerCase(Locale::ROOT)->endsWith(L".tmp")) {
      throw invalid_argument(L"invalid codec filename '" + file +
                             L"', cannot end with .tmp extension");
    }
  }
}

wstring SegmentInfo::namedForThisSegment(const wstring &file)
{
  return name + IndexFileNames::stripSegmentName(file);
}

wstring SegmentInfo::getAttribute(const wstring &key)
{
  return attributes[key];
}

wstring SegmentInfo::putAttribute(const wstring &key, const wstring &value)
{
  return attributes.emplace(key, value);
}

unordered_map<wstring, wstring> SegmentInfo::getAttributes()
{
  return attributes;
}

shared_ptr<Sort> SegmentInfo::getIndexSort() { return indexSort; }
} // namespace org::apache::lucene::index