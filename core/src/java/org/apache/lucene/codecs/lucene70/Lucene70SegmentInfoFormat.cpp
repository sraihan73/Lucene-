using namespace std;

#include "Lucene70SegmentInfoFormat.h"

namespace org::apache::lucene::codecs::lucene70
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
using IndexOutput = org::apache::lucene::store::IndexOutput;
using Version = org::apache::lucene::util::Version;

Lucene70SegmentInfoFormat::Lucene70SegmentInfoFormat() {}

shared_ptr<SegmentInfo> Lucene70SegmentInfoFormat::read(
    shared_ptr<Directory> dir, const wstring &segment,
    std::deque<char> &segmentID,
    shared_ptr<IOContext> context) 
{
  const wstring fileName = IndexFileNames::segmentFileName(
      segment, L"", Lucene70SegmentInfoFormat::SI_EXTENSION);
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
          input, Lucene70SegmentInfoFormat::CODEC_NAME,
          Lucene70SegmentInfoFormat::VERSION_START,
          Lucene70SegmentInfoFormat::VERSION_CURRENT, segmentID, L"");
      shared_ptr<Version> *const version = Version::fromBits(
          input->readInt(), input->readInt(), input->readInt());
      char hasMinVersion = input->readByte();
      shared_ptr<Version> *const minVersion;
      switch (hasMinVersion) {
      case 0:
        minVersion.reset();
        break;
      case 1:
        minVersion = Version::fromBits(input->readInt(), input->readInt(),
                                       input->readInt());
        break;
      default:
        throw make_shared<CorruptIndexException>(
            L"Illegal bool value " + to_wstring(hasMinVersion), input);
      }

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

      si = make_shared<SegmentInfo>(dir, version, minVersion, segment, docCount,
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

void Lucene70SegmentInfoFormat::write(
    shared_ptr<Directory> dir, shared_ptr<SegmentInfo> si,
    shared_ptr<IOContext> ioContext) 
{
  const wstring fileName = IndexFileNames::segmentFileName(
      si->name, L"", Lucene70SegmentInfoFormat::SI_EXTENSION);

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexOutput output =
  // dir.createOutput(fileName, ioContext))
  {
    org::apache::lucene::store::IndexOutput output =
        dir->createOutput(fileName, ioContext);
    // Only add the file once we've successfully created it, else IFD assert can
    // trip:
    si->addFile(fileName);
    CodecUtil::writeIndexHeader(output, Lucene70SegmentInfoFormat::CODEC_NAME,
                                Lucene70SegmentInfoFormat::VERSION_CURRENT,
                                si->getId(), L"");
    shared_ptr<Version> version = si->getVersion();
    if (version->major < 7) {
      throw invalid_argument(
          L"invalid major version: should be >= 7 but got: " +
          to_wstring(version->major) + L" segment=" + si);
    }
    // Write the Lucene version that created this segment, since 3.1
    output->writeInt(version->major);
    output->writeInt(version->minor);
    output->writeInt(version->bugfix);

    // Write the min Lucene version that contributed docs to the segment,
    // since 7.0
    if (si->getMinVersion() != nullptr) {
      output->writeByte(static_cast<char>(1));
      shared_ptr<Version> minVersion = si->getMinVersion();
      output->writeInt(minVersion->major);
      output->writeInt(minVersion->minor);
      output->writeInt(minVersion->bugfix);
    } else {
      output->writeByte(static_cast<char>(0));
    }

    assert(version->prerelease == 0);
    output->writeInt(si->maxDoc());

    output->writeByte(static_cast<char>(
        si->getUseCompoundFile() ? SegmentInfo::YES : SegmentInfo::NO));
    output->writeMapOfStrings(si->getDiagnostics());
    shared_ptr<Set<wstring>> files = si->files();
    for (auto file : files) {
      if (IndexFileNames::parseSegmentName(file) != si->name) {
        throw invalid_argument(L"invalid files: expected segment=" + si->name +
                               L", got=" + files);
      }
    }
    output->writeSetOfStrings(files);
    output->writeMapOfStrings(si->getAttributes());

    shared_ptr<Sort> indexSort = si->getIndexSort();
    int numSortFields = indexSort == nullptr ? 0 : indexSort->getSort().size();
    output->writeVInt(numSortFields);
    for (int i = 0; i < numSortFields; ++i) {
      shared_ptr<SortField> sortField = indexSort->getSort()[i];
      SortField::Type sortType = sortField->getType();
      output->writeString(sortField->getField());
      int sortTypeID;
      switch (sortField->getType()) {
      case STRING:
        sortTypeID = 0;
        break;
      case LONG:
        sortTypeID = 1;
        break;
      case INT:
        sortTypeID = 2;
        break;
      case DOUBLE:
        sortTypeID = 3;
        break;
      case FLOAT:
        sortTypeID = 4;
        break;
      case CUSTOM:
        if (std::dynamic_pointer_cast<SortedSetSortField>(sortField) !=
            nullptr) {
          sortTypeID = 5;
          sortType = SortField::Type::STRING;
        } else if (std::dynamic_pointer_cast<SortedNumericSortField>(
                       sortField) != nullptr) {
          sortTypeID = 6;
          sortType =
              (std::static_pointer_cast<SortedNumericSortField>(sortField))
                  ->getNumericType();
        } else {
          throw make_shared<IllegalStateException>(
              L"Unexpected SortedNumericSortField " + sortField);
        }
        break;
      default:
        throw make_shared<IllegalStateException>(L"Unexpected sort type: " +
                                                 sortField->getType());
      }
      output->writeVInt(sortTypeID);
      if (sortTypeID == 5) {
        shared_ptr<SortedSetSortField> ssf =
            std::static_pointer_cast<SortedSetSortField>(sortField);
        if (ssf->getSelector() == SortedSetSelector::Type::MIN) {
          output->writeByte(static_cast<char>(0));
        } else if (ssf->getSelector() == SortedSetSelector::Type::MAX) {
          output->writeByte(static_cast<char>(1));
        } else if (ssf->getSelector() == SortedSetSelector::Type::MIDDLE_MIN) {
          output->writeByte(static_cast<char>(2));
        } else if (ssf->getSelector() == SortedSetSelector::Type::MIDDLE_MAX) {
          output->writeByte(static_cast<char>(3));
        } else {
          throw make_shared<IllegalStateException>(
              L"Unexpected SortedSetSelector type: " + ssf->getSelector());
        }
      } else if (sortTypeID == 6) {
        shared_ptr<SortedNumericSortField> snsf =
            std::static_pointer_cast<SortedNumericSortField>(sortField);
        if (snsf->getNumericType() == SortField::Type::LONG) {
          output->writeByte(static_cast<char>(0));
        } else if (snsf->getNumericType() == SortField::Type::INT) {
          output->writeByte(static_cast<char>(1));
        } else if (snsf->getNumericType() == SortField::Type::DOUBLE) {
          output->writeByte(static_cast<char>(2));
        } else if (snsf->getNumericType() == SortField::Type::FLOAT) {
          output->writeByte(static_cast<char>(3));
        } else {
          throw make_shared<IllegalStateException>(
              L"Unexpected SortedNumericSelector type: " +
              snsf->getNumericType());
        }
        if (snsf->getSelector() == SortedNumericSelector::Type::MIN) {
          output->writeByte(static_cast<char>(0));
        } else if (snsf->getSelector() == SortedNumericSelector::Type::MAX) {
          output->writeByte(static_cast<char>(1));
        } else {
          throw make_shared<IllegalStateException>(
              L"Unexpected sorted numeric selector type: " +
              snsf->getSelector());
        }
      }
      output->writeByte(static_cast<char>(sortField->getReverse() ? 0 : 1));

      // write missing value
      any missingValue = sortField->getMissingValue();
      if (missingValue == nullptr) {
        output->writeByte(static_cast<char>(0));
      } else {
        switch (sortType) {
        case SortField::Type::STRING:
          if (missingValue == SortField::STRING_LAST) {
            output->writeByte(static_cast<char>(1));
          } else if (missingValue == SortField::STRING_FIRST) {
            output->writeByte(static_cast<char>(2));
          } else {
            throw make_shared<AssertionError>(
                L"unrecognized missing value for STRING field \"" +
                sortField->getField() + L"\": " + missingValue);
          }
          break;
        case SortField::Type::LONG:
          output->writeByte(static_cast<char>(1));
          output->writeLong(
              (any_cast<optional<int64_t>>(missingValue)).value());
          break;
        case SortField::Type::INT:
          output->writeByte(static_cast<char>(1));
          output->writeInt((any_cast<optional<int>>(missingValue)).value());
          break;
        case SortField::Type::DOUBLE:
          output->writeByte(static_cast<char>(1));
          output->writeLong(Double::doubleToLongBits(
              (any_cast<optional<double>>(missingValue)).value()));
          break;
        case SortField::Type::FLOAT:
          output->writeByte(static_cast<char>(1));
          output->writeInt(Float::floatToIntBits(
              (any_cast<optional<float>>(missingValue)).value()));
          break;
        default:
          throw make_shared<IllegalStateException>(L"Unexpected sort type: " +
                                                   sortField->getType());
        }
      }
    }

    CodecUtil::writeFooter(output);
  }
}

const wstring Lucene70SegmentInfoFormat::SI_EXTENSION = L"si";
const wstring Lucene70SegmentInfoFormat::CODEC_NAME = L"Lucene70SegmentInfo";
} // namespace org::apache::lucene::codecs::lucene70