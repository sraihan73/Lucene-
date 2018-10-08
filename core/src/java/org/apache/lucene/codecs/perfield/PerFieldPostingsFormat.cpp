using namespace std;

#include "PerFieldPostingsFormat.h"

namespace org::apache::lucene::codecs::perfield
{
using FieldsConsumer = org::apache::lucene::codecs::FieldsConsumer;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using Fields = org::apache::lucene::index::Fields;
using FilterFields = org::apache::lucene::index::FilterLeafReader::FilterFields;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using MergeState = org::apache::lucene::index::MergeState;
using MultiFields = org::apache::lucene::index::MultiFields;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using Terms = org::apache::lucene::index::Terms;
using Accountable = org::apache::lucene::util::Accountable;
using Accountables = org::apache::lucene::util::Accountables;
using IOUtils = org::apache::lucene::util::IOUtils;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
const wstring PerFieldPostingsFormat::PER_FIELD_NAME = L"PerField40";
const wstring PerFieldPostingsFormat::PER_FIELD_FORMAT_KEY =
    PerFieldPostingsFormat::typeid->getSimpleName() + L".format";
const wstring PerFieldPostingsFormat::PER_FIELD_SUFFIX_KEY =
    PerFieldPostingsFormat::typeid->getSimpleName() + L".suffix";

PerFieldPostingsFormat::PerFieldPostingsFormat()
    : org::apache::lucene::codecs::PostingsFormat(PER_FIELD_NAME)
{
}

wstring PerFieldPostingsFormat::getSuffix(const wstring &formatName,
                                          const wstring &suffix)
{
  return formatName + L"_" + suffix;
}

wstring
PerFieldPostingsFormat::getFullSegmentSuffix(const wstring &fieldName,
                                             const wstring &outerSegmentSuffix,
                                             const wstring &segmentSuffix)
{
  if (outerSegmentSuffix.length() == 0) {
    return segmentSuffix;
  } else {
    // TODO: support embedding; I think it should work but
    // we need a test confirm to confirm
    // return outerSegmentSuffix + "_" + segmentSuffix;
    throw make_shared<IllegalStateException>(
        L"cannot embed PerFieldPostingsFormat inside itself (field \"" +
        fieldName + L"\" returned PerFieldPostingsFormat)");
  }
}

PerFieldPostingsFormat::FieldsWriter::FieldsWriter(
    shared_ptr<PerFieldPostingsFormat> outerInstance,
    shared_ptr<SegmentWriteState> writeState)
    : writeState(writeState), outerInstance(outerInstance)
{
}

void PerFieldPostingsFormat::FieldsWriter::write(
    shared_ptr<Fields> fields) 
{
  unordered_map<std::shared_ptr<PostingsFormat>, std::shared_ptr<FieldsGroup>>
      formatToGroups = buildFieldsGroupMapping(fields);

  // Write postings
  bool success = false;
  try {
    for (auto ent : formatToGroups) {
      shared_ptr<PostingsFormat> format = ent.first;
      shared_ptr<FieldsGroup> *const group = ent.second;

      // Exposes only the fields from this group:
      shared_ptr<Fields> maskedFields =
          make_shared<FilterFieldsAnonymousInnerClass>(shared_from_this(),
                                                       fields, group);

      shared_ptr<FieldsConsumer> consumer =
          format->fieldsConsumer(group->state);
      toClose.push_back(consumer);
      consumer->write(maskedFields);
    }
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      IOUtils::closeWhileHandlingException(toClose);
    }
  }
}

PerFieldPostingsFormat::FieldsWriter::FilterFieldsAnonymousInnerClass::
    FilterFieldsAnonymousInnerClass(
        shared_ptr<FieldsWriter> outerInstance, shared_ptr<Fields> fields,
        shared_ptr<org::apache::lucene::codecs::perfield::
                       PerFieldPostingsFormat::FieldsGroup>
            group)
    : org::apache::lucene::index::FilterLeafReader::FilterFields(fields)
{
  this->outerInstance = outerInstance;
  this->group = group;
}

shared_ptr<Iterator<wstring>> PerFieldPostingsFormat::FieldsWriter::
    FilterFieldsAnonymousInnerClass::iterator()
{
  return group->fields->begin();
}

void PerFieldPostingsFormat::FieldsWriter::merge(
    shared_ptr<MergeState> mergeState) 
{
  unordered_map<std::shared_ptr<PostingsFormat>, std::shared_ptr<FieldsGroup>>
      formatToGroups = buildFieldsGroupMapping(
          make_shared<MultiFields>(mergeState->fieldsProducers, nullptr));

  // Merge postings
  shared_ptr<PerFieldMergeState> pfMergeState =
      make_shared<PerFieldMergeState>(mergeState);
  bool success = false;
  try {
    for (auto ent : formatToGroups) {
      shared_ptr<PostingsFormat> format = ent.first;
      shared_ptr<FieldsGroup> *const group = ent.second;

      shared_ptr<FieldsConsumer> consumer =
          format->fieldsConsumer(group->state);
      toClose.push_back(consumer);
      consumer->merge(pfMergeState->apply(group->fields));
    }
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    pfMergeState->reset();
    if (!success) {
      IOUtils::closeWhileHandlingException(toClose);
    }
  }
}

unordered_map<std::shared_ptr<PostingsFormat>, std::shared_ptr<FieldsGroup>>
PerFieldPostingsFormat::FieldsWriter::buildFieldsGroupMapping(
    shared_ptr<Fields> fields)
{
  // Maps a PostingsFormat instance to the suffix it
  // should use
  unordered_map<std::shared_ptr<PostingsFormat>, std::shared_ptr<FieldsGroup>>
      formatToGroups = unordered_map<std::shared_ptr<PostingsFormat>,
                                     std::shared_ptr<FieldsGroup>>();

  // Holds last suffix of each PostingFormat name
  unordered_map<wstring, int> suffixes = unordered_map<wstring, int>();

  // Assign field -> PostingsFormat
  for (auto field : fields) {
    shared_ptr<FieldInfo> fieldInfo = writeState->fieldInfos->fieldInfo(field);

    shared_ptr<PostingsFormat> *const format =
        outerInstance->getPostingsFormatForField(field);

    if (format == nullptr) {
      throw make_shared<IllegalStateException>(
          L"invalid null PostingsFormat for field=\"" + field + L"\"");
    }
    wstring formatName = format->getName();

    shared_ptr<FieldsGroup> group = formatToGroups[format];
    if (group == nullptr) {
      // First time we are seeing this format; create a
      // new instance

      // bump the suffix
      optional<int> suffix = suffixes[formatName];
      if (!suffix) {
        suffix = 0;
      } else {
        suffix = suffix + 1;
      }
      suffixes.emplace(formatName, suffix);

      // C++ TODO: There is no native C++ equivalent to 'toString':
      wstring segmentSuffix = getFullSegmentSuffix(
          field, writeState->segmentSuffix,
          getSuffix(formatName, Integer::toString(suffix)));
      group = make_shared<FieldsGroup>();
      group->state = make_shared<SegmentWriteState>(writeState, segmentSuffix);
      group->suffix = suffix;
      formatToGroups.emplace(format, group);
    } else {
      // we've already seen this format, so just grab its suffix
      if (suffixes.find(formatName) == suffixes.end()) {
        throw make_shared<IllegalStateException>(
            L"no suffix for format name: " + formatName + L", expected: " +
            to_wstring(group->suffix));
      }
    }

    group->fields->add(field);

    wstring previousValue =
        fieldInfo->putAttribute(PER_FIELD_FORMAT_KEY, formatName);
    if (previousValue != L"") {
      throw make_shared<IllegalStateException>(
          L"found existing value for " + PER_FIELD_FORMAT_KEY + L", field=" +
          fieldInfo->name + L", old=" + previousValue + L", new=" + formatName);
    }

    // C++ TODO: There is no native C++ equivalent to 'toString':
    previousValue = fieldInfo->putAttribute(PER_FIELD_SUFFIX_KEY,
                                            Integer::toString(group->suffix));
    if (previousValue != L"") {
      throw make_shared<IllegalStateException>(
          L"found existing value for " + PER_FIELD_SUFFIX_KEY + L", field=" +
          fieldInfo->name + L", old=" + previousValue + L", new=" +
          to_wstring(group->suffix));
    }
  }
  return formatToGroups;
}

PerFieldPostingsFormat::FieldsWriter::~FieldsWriter()
{
  IOUtils::close(toClose);
}

PerFieldPostingsFormat::FieldsReader::FieldsReader(
    shared_ptr<FieldsReader> other) 
    : segment(other->segment)
{
  unordered_map<std::shared_ptr<FieldsProducer>,
                std::shared_ptr<FieldsProducer>>
      oldToNew =
          make_shared<IdentityHashMap<std::shared_ptr<FieldsProducer>,
                                      std::shared_ptr<FieldsProducer>>>();
  // First clone all formats
  for (auto ent : other->formats) {
    shared_ptr<FieldsProducer> values = ent.second::getMergeInstance();
    formats.emplace(ent.first, values);
    oldToNew.emplace(ent.second, values);
  }

  // Then rebuild fields:
  for (auto ent : other->fields) {
    shared_ptr<FieldsProducer> producer = oldToNew[ent.second];
    assert(producer->size() > 0);
    fields.emplace(ent.first, producer);
  }
}

PerFieldPostingsFormat::FieldsReader::FieldsReader(
    shared_ptr<SegmentReadState> readState) 
    : segment(readState->segmentInfo->name)
{

  // Read _X.per and init each format:
  bool success = false;
  try {
    // Read field name -> format name
    for (auto fi : readState->fieldInfos) {
      if (fi->getIndexOptions() != IndexOptions::NONE) {
        const wstring fieldName = fi->name;
        const wstring formatName = fi->getAttribute(PER_FIELD_FORMAT_KEY);
        if (formatName != L"") {
          // null formatName means the field is in fieldInfos, but has no
          // postings!
          const wstring suffix = fi->getAttribute(PER_FIELD_SUFFIX_KEY);
          if (suffix == L"") {
            throw make_shared<IllegalStateException>(
                L"missing attribute: " + PER_FIELD_SUFFIX_KEY +
                L" for field: " + fieldName);
          }
          shared_ptr<PostingsFormat> format =
              PostingsFormat::forName(formatName);
          wstring segmentSuffix = getSuffix(formatName, suffix);
          if (formats.find(segmentSuffix) == formats.end()) {
            formats.emplace(segmentSuffix, format->fieldsProducer(
                                               make_shared<SegmentReadState>(
                                                   readState, segmentSuffix)));
          }
          fields.emplace(fieldName, formats[segmentSuffix]);
        }
      }
    }
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (!success) {
      IOUtils::closeWhileHandlingException({formats.values()});
    }
  }
}

shared_ptr<Iterator<wstring>> PerFieldPostingsFormat::FieldsReader::iterator()
{
  return Collections::unmodifiableSet(fields.keySet()).begin();
}

shared_ptr<Terms> PerFieldPostingsFormat::FieldsReader::terms(
    const wstring &field) 
{
  shared_ptr<FieldsProducer> fieldsProducer = fields[field];
  return fieldsProducer->empty() ? nullptr : fieldsProducer->terms(field);
}

int PerFieldPostingsFormat::FieldsReader::size() { return fields.size(); }

PerFieldPostingsFormat::FieldsReader::~FieldsReader()
{
  IOUtils::close({formats.values()});
}

int64_t PerFieldPostingsFormat::FieldsReader::ramBytesUsed()
{
  int64_t ramBytesUsed = BASE_RAM_BYTES_USED;
  ramBytesUsed += fields.size() * 2LL * RamUsageEstimator::NUM_BYTES_OBJECT_REF;
  ramBytesUsed +=
      formats.size() * 2LL * RamUsageEstimator::NUM_BYTES_OBJECT_REF;
  for (auto entry : formats) {
    ramBytesUsed += entry.second::ramBytesUsed();
  }
  return ramBytesUsed;
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
PerFieldPostingsFormat::FieldsReader::getChildResources()
{
  return Accountables::namedAccountables(L"format", formats);
}

void PerFieldPostingsFormat::FieldsReader::checkIntegrity() 
{
  for (auto producer : formats) {
    producer->second.checkIntegrity();
  }
}

shared_ptr<FieldsProducer>
PerFieldPostingsFormat::FieldsReader::getMergeInstance() 
{
  return make_shared<FieldsReader>(shared_from_this());
}

wstring PerFieldPostingsFormat::FieldsReader::toString()
{
  return L"PerFieldPostings(segment=" + segment + L" formats=" +
         formats.size() + L")";
}

shared_ptr<FieldsConsumer> PerFieldPostingsFormat::fieldsConsumer(
    shared_ptr<SegmentWriteState> state) 
{
  return make_shared<FieldsWriter>(shared_from_this(), state);
}

shared_ptr<FieldsProducer> PerFieldPostingsFormat::fieldsProducer(
    shared_ptr<SegmentReadState> state) 
{
  return make_shared<FieldsReader>(state);
}
} // namespace org::apache::lucene::codecs::perfield