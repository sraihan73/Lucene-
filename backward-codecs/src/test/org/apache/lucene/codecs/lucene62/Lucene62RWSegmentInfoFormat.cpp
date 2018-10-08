using namespace std;

#include "Lucene62RWSegmentInfoFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/codecs/CodecUtil.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexFileNames.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentInfo.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/Sort.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/SortField.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/SortedNumericSelector.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/SortedNumericSortField.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/SortedSetSelector.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/SortedSetSortField.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/Directory.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IOContext.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/Version.h"

namespace org::apache::lucene::codecs::lucene62
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using IndexFileNames = org::apache::lucene::index::IndexFileNames;
using SegmentInfo = org::apache::lucene::index::SegmentInfo;
using Sort = org::apache::lucene::search::Sort;
using SortField = org::apache::lucene::search::SortField;
using SortedNumericSelector =
    org::apache::lucene::search::SortedNumericSelector;
using SortedNumericSortField =
    org::apache::lucene::search::SortedNumericSortField;
using SortedSetSelector = org::apache::lucene::search::SortedSetSelector;
using SortedSetSortField = org::apache::lucene::search::SortedSetSortField;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using Version = org::apache::lucene::util::Version;

void Lucene62RWSegmentInfoFormat::write(
    shared_ptr<Directory> dir, shared_ptr<SegmentInfo> si,
    shared_ptr<IOContext> ioContext) 
{
  const wstring fileName = IndexFileNames::segmentFileName(
      si->name, L"", Lucene62SegmentInfoFormat::SI_EXTENSION);

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexOutput output =
  // dir.createOutput(fileName, ioContext))
  {
    org::apache::lucene::store::IndexOutput output =
        dir->createOutput(fileName, ioContext);
    // Only add the file once we've successfully created it, else IFD assert can
    // trip:
    si->addFile(fileName);
    CodecUtil::writeIndexHeader(output, Lucene62SegmentInfoFormat::CODEC_NAME,
                                Lucene62SegmentInfoFormat::VERSION_CURRENT,
                                si->getId(), L"");
    shared_ptr<Version> version = si->getVersion();
    if (version->major < 5) {
      throw invalid_argument(
          L"invalid major version: should be >= 5 but got: " +
          to_wstring(version->major) + L" segment=" + si);
    }
    // Write the Lucene version that created this segment, since 3.1
    output->writeInt(version->major);
    output->writeInt(version->minor);
    output->writeInt(version->bugfix);
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
} // namespace org::apache::lucene::codecs::lucene62