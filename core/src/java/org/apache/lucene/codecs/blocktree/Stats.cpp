using namespace std;

#include "Stats.h"
#include "../../util/ArrayUtil.h"
#include "../../util/BytesRef.h"
#include "../../util/IOUtils.h"
#include "SegmentTermsEnumFrame.h"

namespace org::apache::lucene::codecs::blocktree
{
using PostingsReaderBase = org::apache::lucene::codecs::PostingsReaderBase;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using IOUtils = org::apache::lucene::util::IOUtils;

Stats::Stats(const wstring &segment, const wstring &field)
    : segment(segment), field(field)
{
}

void Stats::startBlock(shared_ptr<SegmentTermsEnumFrame> frame, bool isFloor)
{
  totalBlockCount++;
  if (isFloor) {
    if (frame->fp == frame->fpOrig) {
      floorBlockCount++;
    }
    floorSubBlockCount++;
  } else {
    nonFloorBlockCount++;
  }

  if (blockCountByPrefixLen.size() <= frame->prefix) {
    blockCountByPrefixLen =
        ArrayUtil::grow(blockCountByPrefixLen, 1 + frame->prefix);
  }
  blockCountByPrefixLen[frame->prefix]++;
  startBlockCount++;
  totalBlockSuffixBytes += frame->suffixesReader->length();
  totalBlockStatsBytes += frame->statsReader->length();
}

void Stats::endBlock(shared_ptr<SegmentTermsEnumFrame> frame)
{
  constexpr int termCount =
      frame->isLeafBlock ? frame->entCount : frame->state->termBlockOrd;
  constexpr int subBlockCount = frame->entCount - termCount;
  totalTermCount += termCount;
  if (termCount != 0 && subBlockCount != 0) {
    mixedBlockCount++;
  } else if (termCount != 0) {
    termsOnlyBlockCount++;
  } else if (subBlockCount != 0) {
    subBlocksOnlyBlockCount++;
  } else {
    throw make_shared<IllegalStateException>();
  }
  endBlockCount++;
  constexpr int64_t otherBytes = frame->fpEnd - frame->fp -
                                   frame->suffixesReader->length() -
                                   frame->statsReader->length();
  assert((otherBytes > 0, L"otherBytes=" + to_wstring(otherBytes) +
                              L" frame.fp=" + to_wstring(frame->fp) +
                              L" frame.fpEnd=" + to_wstring(frame->fpEnd)));
  totalBlockOtherBytes += otherBytes;
}

void Stats::term(shared_ptr<BytesRef> term) { totalTermBytes += term->length; }

void Stats::finish()
{
  assert((startBlockCount == endBlockCount,
          L"startBlockCount=" + to_wstring(startBlockCount) +
              L" endBlockCount=" + to_wstring(endBlockCount)));
  assert((totalBlockCount == floorSubBlockCount + nonFloorBlockCount,
          L"floorSubBlockCount=" + to_wstring(floorSubBlockCount) +
              L" nonFloorBlockCount=" + to_wstring(nonFloorBlockCount) +
              L" totalBlockCount=" + to_wstring(totalBlockCount)));
  assert((totalBlockCount ==
              mixedBlockCount + termsOnlyBlockCount + subBlocksOnlyBlockCount,
          L"totalBlockCount=" + to_wstring(totalBlockCount) +
              L" mixedBlockCount=" + to_wstring(mixedBlockCount) +
              L" subBlocksOnlyBlockCount=" +
              to_wstring(subBlocksOnlyBlockCount) + L" termsOnlyBlockCount=" +
              to_wstring(termsOnlyBlockCount)));
}

wstring Stats::toString()
{
  shared_ptr<ByteArrayOutputStream> *const bos =
      make_shared<ByteArrayOutputStream>(1024);
  shared_ptr<PrintStream> out;
  try {
    out = make_shared<PrintStream>(bos, false, IOUtils::UTF_8);
  } catch (const UnsupportedEncodingException &bogus) {
    throw runtime_error(bogus);
  }

  out->println(L"  index FST:");
  out->println(L"    " + to_wstring(indexNumBytes) + L" bytes");
  out->println(L"  terms:");
  out->println(L"    " + to_wstring(totalTermCount) + L" terms");
  out->println(
      L"    " + to_wstring(totalTermBytes) + L" bytes" +
      StringHelper::toString(
          totalTermCount != 0
              ? L" (" +
                    wstring::format(Locale::ROOT, L"%.1f",
                                    (static_cast<double>(totalTermBytes)) /
                                        totalTermCount) +
                    L" bytes/term)"
              : L""));
  out->println(L"  blocks:");
  out->println(L"    " + to_wstring(totalBlockCount) + L" blocks");
  out->println(L"    " + to_wstring(termsOnlyBlockCount) +
               L" terms-only blocks");
  out->println(L"    " + to_wstring(subBlocksOnlyBlockCount) +
               L" sub-block-only blocks");
  out->println(L"    " + to_wstring(mixedBlockCount) + L" mixed blocks");
  out->println(L"    " + to_wstring(floorBlockCount) + L" floor blocks");
  out->println(L"    " + to_wstring(totalBlockCount - floorSubBlockCount) +
               L" non-floor blocks");
  out->println(L"    " + to_wstring(floorSubBlockCount) + L" floor sub-blocks");
  out->println(L"    " + to_wstring(totalBlockSuffixBytes) +
               L" term suffix bytes" +
               StringHelper::toString(
                   totalBlockCount != 0
                       ? L" (" +
                             wstring::format(
                                 Locale::ROOT, L"%.1f",
                                 (static_cast<double>(totalBlockSuffixBytes)) /
                                     totalBlockCount) +
                             L" suffix-bytes/block)"
                       : L""));
  out->println(L"    " + to_wstring(totalBlockStatsBytes) +
               L" term stats bytes" +
               StringHelper::toString(
                   totalBlockCount != 0
                       ? L" (" +
                             wstring::format(
                                 Locale::ROOT, L"%.1f",
                                 (static_cast<double>(totalBlockStatsBytes)) /
                                     totalBlockCount) +
                             L" stats-bytes/block)"
                       : L""));
  out->println(L"    " + to_wstring(totalBlockOtherBytes) + L" other bytes" +
               StringHelper::toString(
                   totalBlockCount != 0
                       ? L" (" +
                             wstring::format(
                                 Locale::ROOT, L"%.1f",
                                 (static_cast<double>(totalBlockOtherBytes)) /
                                     totalBlockCount) +
                             L" other-bytes/block)"
                       : L""));
  if (totalBlockCount != 0) {
    out->println(L"    by prefix length:");
    int total = 0;
    for (int prefix = 0; prefix < blockCountByPrefixLen.size(); prefix++) {
      constexpr int blockCount = blockCountByPrefixLen[prefix];
      total += blockCount;
      if (blockCount != 0) {
        out->println(L"      " + wstring::format(Locale::ROOT, L"%2d", prefix) +
                     L": " + to_wstring(blockCount));
      }
    }
    assert(totalBlockCount == total);
  }

  try {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    return bos->toString(IOUtils::UTF_8);
  } catch (const UnsupportedEncodingException &bogus) {
    throw runtime_error(bogus);
  }
}
} // namespace org::apache::lucene::codecs::blocktree