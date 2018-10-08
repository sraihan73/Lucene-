using namespace std;

#include "IndexFileNames.h"

namespace org::apache::lucene::index
{
using Codec = org::apache::lucene::codecs::Codec;

IndexFileNames::IndexFileNames() {}

const wstring IndexFileNames::SEGMENTS = L"segments";
const wstring IndexFileNames::PENDING_SEGMENTS = L"pending_segments";
const wstring IndexFileNames::OLD_SEGMENTS_GEN = L"segments.gen";

wstring IndexFileNames::fileNameFromGeneration(const wstring &base,
                                               const wstring &ext,
                                               int64_t gen)
{
  if (gen == -1) {
    return L"";
  } else if (gen == 0) {
    return segmentFileName(base, L"", ext);
  } else {
    assert(gen > 0);
    // The '6' part in the length is: 1 for '.', 1 for '_' and 4 as estimate
    // to the gen length as string (hopefully an upper limit so SB won't
    // expand in the middle.
    // C++ TODO: There is no native C++ equivalent to 'toString':
    shared_ptr<StringBuilder> res =
        (make_shared<StringBuilder>(base.length() + 6 + ext.length()))
            ->append(base)
            ->append(L'_')
            ->append(Long::toString(gen, Character::MAX_RADIX));
    if (ext.length() > 0) {
      res->append(L'.')->append(ext);
    }
    return res->toString();
  }
}

wstring IndexFileNames::segmentFileName(const wstring &segmentName,
                                        const wstring &segmentSuffix,
                                        const wstring &ext)
{
  if (ext.length() > 0 || segmentSuffix.length() > 0) {
    assert((!StringHelper::startsWith(ext, L".")));
    shared_ptr<StringBuilder> sb = make_shared<StringBuilder>(
        segmentName.length() + 2 + segmentSuffix.length() + ext.length());
    sb->append(segmentName);
    if (segmentSuffix.length() > 0) {
      sb->append(L'_')->append(segmentSuffix);
    }
    if (ext.length() > 0) {
      sb->append(L'.')->append(ext);
    }
    return sb->toString();
  } else {
    return segmentName;
  }
}

bool IndexFileNames::matchesExtension(const wstring &filename,
                                      const wstring &ext)
{
  // It doesn't make a difference whether we allocate a StringBuilder ourself
  // or not, since there's only 1 '+' operator.
  return StringHelper::endsWith(filename, L"." + ext);
}

int IndexFileNames::indexOfSegmentName(const wstring &filename)
{
  // If it is a .del file, there's an '_' after the first character
  int idx = (int)filename.find(L'_', 1);
  if (idx == -1) {
    // If it's not, strip everything that's before the '.'
    idx = (int)filename.find(L'.');
  }
  return idx;
}

wstring IndexFileNames::stripSegmentName(const wstring &filename)
{
  int idx = indexOfSegmentName(filename);
  if (idx != -1) {
    filename = filename.substr(idx);
  }
  return filename;
}

int64_t IndexFileNames::parseGeneration(const wstring &filename)
{
  assert((StringHelper::startsWith(filename, L"_")));
  std::deque<wstring> parts = stripExtension(filename).substr(1)->split(L"_");
  // 4 cases:
  // segment.ext
  // segment_gen.ext
  // segment_codec_suffix.ext
  // segment_gen_codec_suffix.ext
  if (parts.size() == 2 || parts.size() == 4) {
    // C++ TODO: Only single-argument parse and valueOf methods are converted:
    // ORIGINAL LINE: return Long.parseLong(parts[1], Character.MAX_RADIX);
    return int64_t ::valueOf(parts[1], Character::MAX_RADIX);
  } else {
    return 0;
  }
}

wstring IndexFileNames::parseSegmentName(const wstring &filename)
{
  int idx = indexOfSegmentName(filename);
  if (idx != -1) {
    filename = filename.substr(0, idx);
  }
  return filename;
}

wstring IndexFileNames::stripExtension(const wstring &filename)
{
  int idx = (int)filename.find(L'.');
  if (idx != -1) {
    filename = filename.substr(0, idx);
  }
  return filename;
}

wstring IndexFileNames::getExtension(const wstring &filename)
{
  constexpr int idx = (int)filename.find(L'.');
  if (idx == -1) {
    return L"";
  } else {
    return filename.substr(idx + 1, filename.length() - (idx + 1));
  }
}

const shared_ptr<java::util::regex::Pattern>
    IndexFileNames::CODEC_FILE_PATTERN =
        java::util::regex::Pattern::compile(L"_[a-z0-9]+(_.*)?\\..*");
} // namespace org::apache::lucene::index