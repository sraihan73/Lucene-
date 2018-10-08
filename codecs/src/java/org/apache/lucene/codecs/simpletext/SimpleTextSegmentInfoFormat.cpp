using namespace std;

#include "SimpleTextSegmentInfoFormat.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/CorruptIndexException.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexFileNames.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/SegmentInfo.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/Sort.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/SortField.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/SortedNumericSortField.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/SortedSetSortField.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/Directory.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IOContext.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/IndexInput.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/StringHelper.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/Version.h"
#include "SimpleTextUtil.h"

namespace org::apache::lucene::codecs::simpletext
{
using SegmentInfoFormat = org::apache::lucene::codecs::SegmentInfoFormat;
using CorruptIndexException = org::apache::lucene::index::CorruptIndexException;
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
using ChecksumIndexInput = org::apache::lucene::store::ChecksumIndexInput;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexInput = org::apache::lucene::store::IndexInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using StringHelper = org::apache::lucene::util::StringHelper;
using Version = org::apache::lucene::util::Version;
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextSegmentInfoFormat::SI_VERSION =
        make_shared<org::apache::lucene::util::BytesRef>(L"    version ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextSegmentInfoFormat::SI_MIN_VERSION =
        make_shared<org::apache::lucene::util::BytesRef>(L"    min version ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextSegmentInfoFormat::SI_DOCCOUNT =
        make_shared<org::apache::lucene::util::BytesRef>(
            L"    number of documents ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextSegmentInfoFormat::SI_USECOMPOUND =
        make_shared<org::apache::lucene::util::BytesRef>(
            L"    uses compound file ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextSegmentInfoFormat::SI_NUM_DIAG =
        make_shared<org::apache::lucene::util::BytesRef>(L"    diagnostics ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextSegmentInfoFormat::SI_DIAG_KEY =
        make_shared<org::apache::lucene::util::BytesRef>(L"      key ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextSegmentInfoFormat::SI_DIAG_VALUE =
        make_shared<org::apache::lucene::util::BytesRef>(L"      value ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextSegmentInfoFormat::SI_NUM_ATT =
        make_shared<org::apache::lucene::util::BytesRef>(L"    attributes ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextSegmentInfoFormat::SI_ATT_KEY =
        make_shared<org::apache::lucene::util::BytesRef>(L"      key ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextSegmentInfoFormat::SI_ATT_VALUE =
        make_shared<org::apache::lucene::util::BytesRef>(L"      value ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextSegmentInfoFormat::SI_NUM_FILES =
        make_shared<org::apache::lucene::util::BytesRef>(L"    files ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextSegmentInfoFormat::SI_FILE =
        make_shared<org::apache::lucene::util::BytesRef>(L"      file ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextSegmentInfoFormat::SI_ID =
        make_shared<org::apache::lucene::util::BytesRef>(L"    id ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextSegmentInfoFormat::SI_SORT =
        make_shared<org::apache::lucene::util::BytesRef>(L"    sort ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextSegmentInfoFormat::SI_SORT_FIELD =
        make_shared<org::apache::lucene::util::BytesRef>(L"      field ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextSegmentInfoFormat::SI_SORT_TYPE =
        make_shared<org::apache::lucene::util::BytesRef>(L"      type ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextSegmentInfoFormat::SI_SELECTOR_TYPE =
        make_shared<org::apache::lucene::util::BytesRef>(L"      selector ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextSegmentInfoFormat::SI_SORT_REVERSE =
        make_shared<org::apache::lucene::util::BytesRef>(L"      reverse ");
const shared_ptr<org::apache::lucene::util::BytesRef>
    SimpleTextSegmentInfoFormat::SI_SORT_MISSING =
        make_shared<org::apache::lucene::util::BytesRef>(L"      missing ");
const wstring SimpleTextSegmentInfoFormat::SI_EXTENSION = L"si";

shared_ptr<SegmentInfo> SimpleTextSegmentInfoFormat::read(
    shared_ptr<Directory> directory, const wstring &segmentName,
    std::deque<char> &segmentID,
    shared_ptr<IOContext> context) 
{
  shared_ptr<BytesRefBuilder> scratch = make_shared<BytesRefBuilder>();
  wstring segFileName = IndexFileNames::segmentFileName(
      segmentName, L"", SimpleTextSegmentInfoFormat::SI_EXTENSION);
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.ChecksumIndexInput
  // input = directory.openChecksumInput(segFileName, context))
  {
    org::apache::lucene::store::ChecksumIndexInput input =
        directory->openChecksumInput(segFileName, context);
    SimpleTextUtil::readLine(input, scratch);
    assert((StringHelper::startsWith(scratch->get(), SI_VERSION)));
    shared_ptr<Version> *const version;
    try {
      version = Version::parse(readString(SI_VERSION->length, scratch));
    } catch (const ParseException &pe) {
      throw make_shared<CorruptIndexException>(
          L"unable to parse version string: " + pe->getMessage(), input, pe);
    }

    SimpleTextUtil::readLine(input, scratch);
    assert((StringHelper::startsWith(scratch->get(), SI_MIN_VERSION)));
    shared_ptr<Version> minVersion;
    try {
      wstring versionString = readString(SI_MIN_VERSION->length, scratch);
      if (versionString == L"null") {
        minVersion.reset();
      } else {
        minVersion = Version::parse(versionString);
      }
    } catch (const ParseException &pe) {
      throw make_shared<CorruptIndexException>(
          L"unable to parse version string: " + pe->getMessage(), input, pe);
    }

    SimpleTextUtil::readLine(input, scratch);
    assert((StringHelper::startsWith(scratch->get(), SI_DOCCOUNT)));
    constexpr int docCount = stoi(readString(SI_DOCCOUNT->length, scratch));

    SimpleTextUtil::readLine(input, scratch);
    assert((StringHelper::startsWith(scratch->get(), SI_USECOMPOUND)));
    constexpr bool isCompoundFile = StringHelper::fromString<bool>(
        readString(SI_USECOMPOUND->length, scratch));

    SimpleTextUtil::readLine(input, scratch);
    assert((StringHelper::startsWith(scratch->get(), SI_NUM_DIAG)));
    int numDiag = stoi(readString(SI_NUM_DIAG->length, scratch));
    unordered_map<wstring, wstring> diagnostics =
        unordered_map<wstring, wstring>();

    for (int i = 0; i < numDiag; i++) {
      SimpleTextUtil::readLine(input, scratch);
      assert((StringHelper::startsWith(scratch->get(), SI_DIAG_KEY)));
      wstring key = readString(SI_DIAG_KEY->length, scratch);

      SimpleTextUtil::readLine(input, scratch);
      assert((StringHelper::startsWith(scratch->get(), SI_DIAG_VALUE)));
      wstring value = readString(SI_DIAG_VALUE->length, scratch);
      diagnostics.emplace(key, value);
    }

    SimpleTextUtil::readLine(input, scratch);
    assert((StringHelper::startsWith(scratch->get(), SI_NUM_ATT)));
    int numAtt = stoi(readString(SI_NUM_ATT->length, scratch));
    unordered_map<wstring, wstring> attributes =
        unordered_map<wstring, wstring>(numAtt);

    for (int i = 0; i < numAtt; i++) {
      SimpleTextUtil::readLine(input, scratch);
      assert((StringHelper::startsWith(scratch->get(), SI_ATT_KEY)));
      wstring key = readString(SI_ATT_KEY->length, scratch);

      SimpleTextUtil::readLine(input, scratch);
      assert((StringHelper::startsWith(scratch->get(), SI_ATT_VALUE)));
      wstring value = readString(SI_ATT_VALUE->length, scratch);
      attributes.emplace(key, value);
    }

    SimpleTextUtil::readLine(input, scratch);
    assert((StringHelper::startsWith(scratch->get(), SI_NUM_FILES)));
    int numFiles = stoi(readString(SI_NUM_FILES->length, scratch));
    shared_ptr<Set<wstring>> files = unordered_set<wstring>();

    for (int i = 0; i < numFiles; i++) {
      SimpleTextUtil::readLine(input, scratch);
      assert((StringHelper::startsWith(scratch->get(), SI_FILE)));
      wstring fileName = readString(SI_FILE->length, scratch);
      files->add(fileName);
    }

    SimpleTextUtil::readLine(input, scratch);
    assert((StringHelper::startsWith(scratch->get(), SI_ID)));
    const std::deque<char> id =
        Arrays::copyOfRange(scratch->bytes(), SI_ID->length, scratch->length());

    if (!Arrays::equals(segmentID, id)) {
      throw make_shared<CorruptIndexException>(
          L"file mismatch, expected: " + StringHelper::idToString(segmentID) +
              L", got: " + StringHelper::idToString(id),
          input);
    }

    SimpleTextUtil::readLine(input, scratch);
    assert((StringHelper::startsWith(scratch->get(), SI_SORT)));
    constexpr int numSortFields = stoi(readString(SI_SORT->length, scratch));
    std::deque<std::shared_ptr<SortField>> sortField(numSortFields);
    for (int i = 0; i < numSortFields; ++i) {
      SimpleTextUtil::readLine(input, scratch);
      assert((StringHelper::startsWith(scratch->get(), SI_SORT_FIELD)));
      const wstring field = readString(SI_SORT_FIELD->length, scratch);

      SimpleTextUtil::readLine(input, scratch);
      assert((StringHelper::startsWith(scratch->get(), SI_SORT_TYPE)));
      const wstring typeAsString = readString(SI_SORT_TYPE->length, scratch);

      constexpr SortField::Type type;
      SortedSetSelector::Type selectorSet = nullptr;
      SortedNumericSelector::Type selectorNumeric = nullptr;
      switch (typeAsString) {
      case L"string":
        type = SortField::Type::STRING;
        break;
      case L"long":
        type = SortField::Type::LONG;
        break;
      case L"int":
        type = SortField::Type::INT;
        break;
      case L"double":
        type = SortField::Type::DOUBLE;
        break;
      case L"float":
        type = SortField::Type::FLOAT;
        break;
      case L"multi_valued_string":
        type = SortField::Type::STRING;
        selectorSet = readSetSelector(input, scratch);
        break;
      case L"multi_valued_long":
        type = SortField::Type::LONG;
        selectorNumeric = readNumericSelector(input, scratch);
        break;
      case L"multi_valued_int":
        type = SortField::Type::INT;
        selectorNumeric = readNumericSelector(input, scratch);
        break;
      case L"multi_valued_double":
        type = SortField::Type::DOUBLE;
        selectorNumeric = readNumericSelector(input, scratch);
        break;
      case L"multi_valued_float":
        type = SortField::Type::FLOAT;
        selectorNumeric = readNumericSelector(input, scratch);
        break;
      default:
        throw make_shared<CorruptIndexException>(
            L"unable to parse sort type string: " + typeAsString, input);
      }

      SimpleTextUtil::readLine(input, scratch);
      assert((StringHelper::startsWith(scratch->get(), SI_SORT_REVERSE)));
      constexpr bool reverse = StringHelper::fromString<bool>(
          readString(SI_SORT_REVERSE->length, scratch));

      SimpleTextUtil::readLine(input, scratch);
      assert((StringHelper::startsWith(scratch->get(), SI_SORT_MISSING)));
      const wstring missingLastAsString =
          readString(SI_SORT_MISSING->length, scratch);
      constexpr any missingValue;
      switch (type) {
      case SortField::Type::STRING:
        switch (missingLastAsString) {
        case L"null":
          missingValue = nullptr;
          break;
        case L"first":
          missingValue = SortField::STRING_FIRST;
          break;
        case L"last":
          missingValue = SortField::STRING_LAST;
          break;
        default:
          throw make_shared<CorruptIndexException>(
              L"unable to parse missing string: " + typeAsString, input);
        }
        break;
      case SortField::Type::LONG:
        switch (missingLastAsString) {
        case L"null":
          missingValue = nullptr;
          break;
        default:
          missingValue =
              StringHelper::fromString<int64_t>(missingLastAsString);
          break;
        }
        break;
      case SortField::Type::INT:
        switch (missingLastAsString) {
        case L"null":
          missingValue = nullptr;
          break;
        default:
          missingValue = stoi(missingLastAsString);
          break;
        }
        break;
      case SortField::Type::DOUBLE:
        switch (missingLastAsString) {
        case L"null":
          missingValue = nullptr;
          break;
        default:
          missingValue = stod(missingLastAsString);
          break;
        }
        break;
      case SortField::Type::FLOAT:
        switch (missingLastAsString) {
        case L"null":
          missingValue = nullptr;
          break;
        default:
          missingValue = stof(missingLastAsString);
          break;
        }
        break;
      default:
        throw make_shared<AssertionError>();
      }
      if (selectorSet != nullptr) {
        sortField[i] = make_shared<SortedSetSortField>(field, reverse);
      } else if (selectorNumeric != nullptr) {
        sortField[i] =
            make_shared<SortedNumericSortField>(field, type, reverse);
      } else {
        sortField[i] = make_shared<SortField>(field, type, reverse);
      }
      if (missingValue != nullptr) {
        sortField[i]->setMissingValue(missingValue);
      }
    }
    shared_ptr<Sort> indexSort =
        sortField.empty() ? nullptr : make_shared<Sort>(sortField);

    SimpleTextUtil::checkFooter(input);

    shared_ptr<SegmentInfo> info = make_shared<SegmentInfo>(
        directory, version, minVersion, segmentName, docCount, isCompoundFile,
        nullptr, diagnostics, id,
        attributes, indexSort);
    info->setFiles(files);
    return info;
  }
}

wstring
SimpleTextSegmentInfoFormat::readString(int offset,
                                        shared_ptr<BytesRefBuilder> scratch)
{
  return wstring(scratch->bytes(), offset, scratch->length() - offset,
                 StandardCharsets::UTF_8);
}

SortedSetSelector::Type SimpleTextSegmentInfoFormat::readSetSelector(
    shared_ptr<IndexInput> input,
    shared_ptr<BytesRefBuilder> scratch) 
{
  SimpleTextUtil::readLine(input, scratch);
  assert((StringHelper::startsWith(scratch->get(), SI_SELECTOR_TYPE)));
  const wstring selectorAsString =
      readString(SI_SELECTOR_TYPE->length, scratch);
  switch (selectorAsString) {
  case L"min":
    return SortedSetSelector::Type::MIN;
  case L"middle_min":
    return SortedSetSelector::Type::MIDDLE_MIN;
  case L"middle_max":
    return SortedSetSelector::Type::MIDDLE_MAX;
  case L"max":
    return SortedSetSelector::Type::MAX;
  default:
    throw make_shared<CorruptIndexException>(
        L"unable to parse SortedSetSelector type: " + selectorAsString, input);
  }
}

SortedNumericSelector::Type SimpleTextSegmentInfoFormat::readNumericSelector(
    shared_ptr<IndexInput> input,
    shared_ptr<BytesRefBuilder> scratch) 
{
  SimpleTextUtil::readLine(input, scratch);
  assert((StringHelper::startsWith(scratch->get(), SI_SELECTOR_TYPE)));
  const wstring selectorAsString =
      readString(SI_SELECTOR_TYPE->length, scratch);
  switch (selectorAsString) {
  case L"min":
    return SortedNumericSelector::Type::MIN;
  case L"max":
    return SortedNumericSelector::Type::MAX;
  default:
    throw make_shared<CorruptIndexException>(
        L"unable to parse SortedNumericSelector type: " + selectorAsString,
        input);
  }
}

void SimpleTextSegmentInfoFormat::write(
    shared_ptr<Directory> dir, shared_ptr<SegmentInfo> si,
    shared_ptr<IOContext> ioContext) 
{

  wstring segFileName = IndexFileNames::segmentFileName(
      si->name, L"", SimpleTextSegmentInfoFormat::SI_EXTENSION);

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexOutput output =
  // dir.createOutput(segFileName, ioContext))
  {
    org::apache::lucene::store::IndexOutput output =
        dir->createOutput(segFileName, ioContext);
    // Only add the file once we've successfully created it, else IFD assert can
    // trip:
    si->addFile(segFileName);
    shared_ptr<BytesRefBuilder> scratch = make_shared<BytesRefBuilder>();

    SimpleTextUtil::write(output, SI_VERSION);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    SimpleTextUtil::write(output, si->getVersion()->toString(), scratch);
    SimpleTextUtil::writeNewline(output);

    SimpleTextUtil::write(output, SI_MIN_VERSION);
    if (si->getMinVersion() == nullptr) {
      SimpleTextUtil::write(output, L"null", scratch);
    } else {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      SimpleTextUtil::write(output, si->getMinVersion()->toString(), scratch);
    }
    SimpleTextUtil::writeNewline(output);

    SimpleTextUtil::write(output, SI_DOCCOUNT);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    SimpleTextUtil::write(output, Integer::toString(si->maxDoc()), scratch);
    SimpleTextUtil::writeNewline(output);

    SimpleTextUtil::write(output, SI_USECOMPOUND);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    SimpleTextUtil::write(output, Boolean::toString(si->getUseCompoundFile()),
                          scratch);
    SimpleTextUtil::writeNewline(output);

    unordered_map<wstring, wstring> diagnostics = si->getDiagnostics();
    int numDiagnostics = diagnostics.empty() ? 0 : diagnostics.size();
    SimpleTextUtil::write(output, SI_NUM_DIAG);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    SimpleTextUtil::write(output, Integer::toString(numDiagnostics), scratch);
    SimpleTextUtil::writeNewline(output);

    if (numDiagnostics > 0) {
      for (auto diagEntry : diagnostics) {
        SimpleTextUtil::write(output, SI_DIAG_KEY);
        SimpleTextUtil::write(output, diagEntry.first, scratch);
        SimpleTextUtil::writeNewline(output);

        SimpleTextUtil::write(output, SI_DIAG_VALUE);
        SimpleTextUtil::write(output, diagEntry.second, scratch);
        SimpleTextUtil::writeNewline(output);
      }
    }

    unordered_map<wstring, wstring> attributes = si->getAttributes();
    SimpleTextUtil::write(output, SI_NUM_ATT);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    SimpleTextUtil::write(output, Integer::toString(attributes.size()),
                          scratch);
    SimpleTextUtil::writeNewline(output);

    for (auto attEntry : attributes) {
      SimpleTextUtil::write(output, SI_ATT_KEY);
      SimpleTextUtil::write(output, attEntry.first, scratch);
      SimpleTextUtil::writeNewline(output);

      SimpleTextUtil::write(output, SI_ATT_VALUE);
      SimpleTextUtil::write(output, attEntry.second, scratch);
      SimpleTextUtil::writeNewline(output);
    }

    shared_ptr<Set<wstring>> files = si->files();
    int numFiles = files == nullptr ? 0 : files->size();
    SimpleTextUtil::write(output, SI_NUM_FILES);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    SimpleTextUtil::write(output, Integer::toString(numFiles), scratch);
    SimpleTextUtil::writeNewline(output);

    if (numFiles > 0) {
      for (auto fileName : files) {
        SimpleTextUtil::write(output, SI_FILE);
        SimpleTextUtil::write(output, fileName, scratch);
        SimpleTextUtil::writeNewline(output);
      }
    }

    SimpleTextUtil::write(output, SI_ID);
    SimpleTextUtil::write(output, make_shared<BytesRef>(si->getId()));
    SimpleTextUtil::writeNewline(output);

    shared_ptr<Sort> indexSort = si->getIndexSort();
    SimpleTextUtil::write(output, SI_SORT);
    constexpr int numSortFields =
        indexSort == nullptr ? 0 : indexSort->getSort().size();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    SimpleTextUtil::write(output, Integer::toString(numSortFields), scratch);
    SimpleTextUtil::writeNewline(output);
    for (int i = 0; i < numSortFields; ++i) {
      shared_ptr<SortField> *const sortField = indexSort->getSort()[i];

      SimpleTextUtil::write(output, SI_SORT_FIELD);
      SimpleTextUtil::write(output, sortField->getField(), scratch);
      SimpleTextUtil::writeNewline(output);

      SimpleTextUtil::write(output, SI_SORT_TYPE);
      const wstring sortTypeString;
      constexpr SortField::Type sortType;
      constexpr bool multiValued;
      if (std::dynamic_pointer_cast<SortedSetSortField>(sortField) != nullptr) {
        sortType = SortField::Type::STRING;
        multiValued = true;
      } else if (std::dynamic_pointer_cast<SortedNumericSortField>(sortField) !=
                 nullptr) {
        sortType = (std::static_pointer_cast<SortedNumericSortField>(sortField))
                       ->getNumericType();
        multiValued = true;
      } else {
        sortType = sortField->getType();
        multiValued = false;
      }
      switch (sortType) {
      case SortField::Type::STRING:
        if (multiValued) {
          sortTypeString = L"multi_valued_string";
        } else {
          sortTypeString = L"string";
        }
        break;
      case SortField::Type::LONG:
        if (multiValued) {
          sortTypeString = L"multi_valued_long";
        } else {
          sortTypeString = L"long";
        }
        break;
      case SortField::Type::INT:
        if (multiValued) {
          sortTypeString = L"multi_valued_int";
        } else {
          sortTypeString = L"int";
        }
        break;
      case SortField::Type::DOUBLE:
        if (multiValued) {
          sortTypeString = L"multi_valued_double";
        } else {
          sortTypeString = L"double";
        }
        break;
      case SortField::Type::FLOAT:
        if (multiValued) {
          sortTypeString = L"multi_valued_float";
        } else {
          sortTypeString = L"float";
        }
        break;
      default:
        throw make_shared<IllegalStateException>(L"Unexpected sort type: " +
                                                 sortField->getType());
      }
      SimpleTextUtil::write(output, sortTypeString, scratch);
      SimpleTextUtil::writeNewline(output);

      if (std::dynamic_pointer_cast<SortedSetSortField>(sortField) != nullptr) {
        SortedSetSelector::Type selector =
            (std::static_pointer_cast<SortedSetSortField>(sortField))
                ->getSelector();
        const wstring selectorString;
        if (selector == SortedSetSelector::Type::MIN) {
          selectorString = L"min";
        } else if (selector == SortedSetSelector::Type::MIDDLE_MIN) {
          selectorString = L"middle_min";
        } else if (selector == SortedSetSelector::Type::MIDDLE_MAX) {
          selectorString = L"middle_max";
        } else if (selector == SortedSetSelector::Type::MAX) {
          selectorString = L"max";
        } else {
          throw make_shared<IllegalStateException>(
              L"Unexpected SortedSetSelector type selector: " + selector);
        }
        SimpleTextUtil::write(output, SI_SELECTOR_TYPE);
        SimpleTextUtil::write(output, selectorString, scratch);
        SimpleTextUtil::writeNewline(output);
      } else if (std::dynamic_pointer_cast<SortedNumericSortField>(sortField) !=
                 nullptr) {
        SortedNumericSelector::Type selector =
            (std::static_pointer_cast<SortedNumericSortField>(sortField))
                ->getSelector();
        const wstring selectorString;
        if (selector == SortedNumericSelector::Type::MIN) {
          selectorString = L"min";
        } else if (selector == SortedNumericSelector::Type::MAX) {
          selectorString = L"max";
        } else {
          throw make_shared<IllegalStateException>(
              L"Unexpected SortedNumericSelector type selector: " + selector);
        }
        SimpleTextUtil::write(output, SI_SELECTOR_TYPE);
        SimpleTextUtil::write(output, selectorString, scratch);
        SimpleTextUtil::writeNewline(output);
      }

      SimpleTextUtil::write(output, SI_SORT_REVERSE);
      // C++ TODO: There is no native C++ equivalent to 'toString':
      SimpleTextUtil::write(output, Boolean::toString(sortField->getReverse()),
                            scratch);
      SimpleTextUtil::writeNewline(output);

      SimpleTextUtil::write(output, SI_SORT_MISSING);
      constexpr any missingValue = sortField->getMissingValue();
      const wstring missing;
      if (missingValue == nullptr) {
        missing = L"null";
      } else if (missingValue == SortField::STRING_FIRST) {
        missing = L"first";
      } else if (missingValue == SortField::STRING_LAST) {
        missing = L"last";
      } else {
        // C++ TODO: There is no native C++ equivalent to 'toString':
        missing = missingValue.toString();
      }
      SimpleTextUtil::write(output, missing, scratch);
      SimpleTextUtil::writeNewline(output);
    }

    SimpleTextUtil::writeChecksum(output, scratch);
  }
}
} // namespace org::apache::lucene::codecs::simpletext