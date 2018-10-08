using namespace std;

#include "FileMetaData.h"

namespace org::apache::lucene::replicator::nrt
{

FileMetaData::FileMetaData(std::deque<char> &header, std::deque<char> &footer,
                           int64_t length, int64_t checksum)
    : header(header), footer(footer), length(length), checksum(checksum)
{
}

wstring FileMetaData::toString()
{
  return L"FileMetaData(length=" + to_wstring(length) + L")";
}
} // namespace org::apache::lucene::replicator::nrt