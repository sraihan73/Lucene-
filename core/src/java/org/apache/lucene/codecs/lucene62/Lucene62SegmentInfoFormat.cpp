using namespace std;

#include "Lucene62SegmentInfoFormat.h"
#include "../../index/CorruptIndexException.h"
#include "../../index/IndexFileNames.h"
#include "../../index/SegmentInfo.h"
#include "../../search/Sort.h"
#include "../../search/SortField.h"
#include "../../search/SortedNumericSelector.h"
#include "../../search/SortedNumericSortField.h"
#include "../../search/SortedSetSelector.h"
#include "../../search/SortedSetSortField.h"
#include "../../store/Directory.h"
#include "../../store/IOContext.h"
#include "../../util/Version.h"
#include "../CodecUtil.h"

namespace org::apache::lucene::codecs::lucene62
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using SegmentInfoFormat = org::apache::lucene::codecs::SegmentInfoFormat;
using CorruptIndexException = org::apache::lucene::index::CorruptIndexException;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using SegmentInfos = org::apache::lucene::index::SegmentInfos;
using Sort = org::apache::lucene::search::Sort;
using SortField = org::apache::lucene::search::SortField;
using SortedNumericSelector =
    org::apache::lucene::search::SortedNumericSelector;
using SortedNumericSortField =
    org::apache::lucene::search::SortedNumericSortField;
using SortedSetSelector = org::apache::lucene::search::SortedSetSelector;
using SortedSetSortField = org::apache::lucene::search::SortedSetSortField;
using ChecksumIndexInput = org::apache::lucene::store::ChecksumIndexInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using Version = org::apache::lucene::util::Version;

Lucene62SegmentInfoFormat::Lucene62SegmentInfoFormat() {}

shared_ptr<SegmentInfo> Lucene62SegmentInfoFormat::read(
    shared_ptr<Directory> dir, const wstring &segment,
    std::deque<char> &segmentID,
    shared_ptr<IOContext> context) 
{
  const wstring fileName = IndexFileNames::segmentFileName(
      segment, L"", Lucene62SegmentInfoFormat::SI_EXTENSION);
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.ChecksumIndexInput
  // input = dir.openChecksumInput(fileName, context))
  {
    org::apache::lucene::store::ChecksumIndexInput input =
        dir->openChecksumInput(fileName, context);
    runtime_error priorE = nullptr;
    shared_ptr<SegmentInfo> si = nullptr;
    try {
      int format = CodecUtil::checkIndexHeader(
          input, Lucene62SegmentInfoFormat::CODEC_NAME,
          Lucene62SegmentInfoFormat::VERSION_START,
          Lucene62SegmentInfoFormat::VERSION_CURRENT, segmentID, L"");
      shared_ptr<Version> *const version = Version::fromBits(
          input->readInt(), input->readInt(), input->readInt());

      constexpr int docCount = input->readInt();
      if (docCount < 0) {
        throw make_shared<CorruptIndexException>(
            L"invalid docCount: " + to_wstring(docCount), input);
      }
      constexpr bool isCompoundFile = input->readByte() == SegmentInfo::YES;

      const unordered_map<wstring, wstring> diagnostics =
          input->readMapOfStrings();
      shared_ptr<Set<wstring>> *const files = input->readSetOfStrings();
      const unordered_map<wstring, wstring> attributes =
          input->readMapOfStrings();

      int numSortFields = input->readVInt();
      shared_ptr<Sort> indexSort;
      if (numSortFields > 0) {
        std::deque<std::shared_ptr<SortField>> sortFields(numSortFields);
        for (int i = 0; i < numSortFields; i++) {
          wstring fieldName = input->readString();
          int sortTypeID = input->readVInt();
          SortField::Type sortType;
          SortedSetSelector::Type sortedSetSelector = nullptr;
          SortedNumericSelector::Type sortedNumericSelector = nullptr;
          switch (sortTypeID) {
          case 0:
            sortType = SortField::Type::STRING;
            break;
          case 1:
            sortType = SortField::Type::LONG;
            break;
          case 2:
            sortType = SortField::Type::INT;
            break;
          case 3:
            sortType = SortField::Type::DOUBLE;
            break;
          case 4:
            sortType = SortField::Type::FLOAT;
            break;
          case 5: {
            sortType = SortField::Type::STRING;
            char selector = input->readByte();
            if (selector == 0) {
              sortedSetSelector = SortedSetSelector::Type::MIN;
            } else if (selector == 1) {
              sortedSetSelector = SortedSetSelector::Type::MAX;
            } else if (selector == 2) {
              sortedSetSelector = SortedSetSelector::Type::MIDDLE_MIN;
            } else if (selector == 3) {
              sortedSetSelector = SortedSetSelector::Type::MIDDLE_MAX;
            } else {
              throw make_shared<CorruptIndexException>(
                  L"invalid index SortedSetSelector ID: " +
                      to_wstring(selector),
                  input);
            }
            break;
          }
          case 6: {
            char type = input->readByte();
            if (type == 0) {
              sortType = SortField::Type::LONG;
            } else if (type == 1) {
              sortType = SortField::Type::INT;
            } else if (type == 2) {
              sortType = SortField::Type::DOUBLE;
            } else if (type == 3) {
              sortType = SortField::Type::FLOAT;
            } else {
              throw make_shared<CorruptIndexException>(
                  L"invalid index SortedNumericSortField type ID: " +
                      to_wstring(type),
                  input);
            }
            char numericSelector = input->readByte();
            if (numericSelector == 0) {
              sortedNumericSelector = SortedNumericSelector::Type::MIN;
            } else if (numericSelector == 1) {
              sortedNumericSelector = SortedNumericSelector::Type::MAX;
            } else {
              throw make_shared<CorruptIndexException>(
                  L"invalid index SortedNumericSelector ID: " +
                      to_wstring(numericSelector),
                  input);
            }
            break;
          }
          default:
            throw make_shared<CorruptIndexException>(
                L"invalid index sort field type ID: " + to_wstring(sortTypeID),
                input);
          }
          char b = input->readByte();
          bool reverse;
          if (b == 0) {
            reverse = true;
          } else if (b == 1) {
            reverse = false;
          } else {
            throw make_shared<CorruptIndexException>(
                L"invalid index sort reverse: " + to_wstring(b), input);
          }

          if (sortedSetSelector != nullptr) {
            sortFields[i] = make_shared<SortedSetSortField>(fieldName, reverse,
                                                            sortedSetSelector);
          } else if (sortedNumericSelector != nullptr) {
            sortFields[i] = make_shared<SortedNumericSortField>(
                fieldName, sortType, reverse, sortedNumericSelector);
          } else {
            sortFields[i] =
                make_shared<SortField>(fieldName, sortType, reverse);
          }

          any missingValue;
          b = input->readByte();
          if (b == 0) {
            missingValue = nullptr;
          } else {
            switch (sortType) {
            case SortField::Type::STRING:
              if (b == 1) {
                missingValue = SortField::STRING_LAST;
              } else if (b == 2) {
                missingValue = SortField::STRING_FIRST;
              } else {
                throw make_shared<CorruptIndexException>(
                    L"invalid missing value flag: " + to_wstring(b), input);
              }
              break;
            case SortField::Type::LONG:
              if (b != 1) {
                throw make_shared<CorruptIndexException>(
                    L"invalid missing value flag: " + to_wstring(b), input);
              }
              missingValue = input->readLong();
              break;
            case SortField::Type::INT:
              if (b != 1) {
                throw make_shared<CorruptIndexException>(
                    L"invalid missing value flag: " + to_wstring(b), input);
              }
              missingValue = input->readInt();
              break;
            case SortField::Type::DOUBLE:
              if (b != 1) {
                throw make_shared<CorruptIndexException>(
                    L"invalid missing value flag: " + to_wstring(b), input);
              }
              missingValue = Double::longBitsToDouble(input->readLong());
              break;
            case SortField::Type::FLOAT:
              if (b != 1) {
                throw make_shared<CorruptIndexException>(
                    L"invalid missing value flag: " + to_wstring(b), input);
              }
              missingValue = Float::intBitsToFloat(input->readInt());
              break;
            default:
              throw make_shared<AssertionError>(L"unhandled sortType=" +
                                                sortType);
            }
          }
          if (missingValue != nullptr) {
            sortFields[i]->setMissingValue(missingValue);
          }
        }
        indexSort = make_shared<Sort>(sortFields);
      } else if (numSortFields < 0) {
        throw make_shared<CorruptIndexException>(
            L"invalid index sort field count: " + to_wstring(numSortFields),
            input);
      } else {
        indexSort.reset();
      }

      si = make_shared<SegmentInfo>(dir, version, nullptr, segment, docCount,
                                    isCompoundFile, nullptr, diagnostics,
                                    segmentID, attributes, indexSort);
      si->setFiles(files);
    } catch (const runtime_error &exception) {
      priorE = exception;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      CodecUtil::checkFooter(input, priorE);
    }
    return si;
  }
}

void Lucene62SegmentInfoFormat::write(
    shared_ptr<Directory> dir, shared_ptr<SegmentInfo> info,
    shared_ptr<IOContext> ioContext) 
{
  throw make_shared<UnsupportedOperationException>(
      L"This format can only be used for reading");
}

const wstring Lucene62SegmentInfoFormat::SI_EXTENSION = L"si";
const wstring Lucene62SegmentInfoFormat::CODEC_NAME = L"Lucene62SegmentInfo";
} // namespace org::apache::lucene::codecs::lucene62