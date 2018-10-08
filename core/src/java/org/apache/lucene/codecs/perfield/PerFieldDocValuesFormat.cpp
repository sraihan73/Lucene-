using namespace std;

#include "PerFieldDocValuesFormat.h"

namespace org::apache::lucene::codecs::perfield
{
using DocValuesConsumer = org::apache::lucene::codecs::DocValuesConsumer;
using DocValuesFormat = org::apache::lucene::codecs::DocValuesFormat;
using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using BinaryDocValues = org::apache::lucene::index::BinaryDocValues;
using DocValuesType = org::apache::lucene::index::DocValuesType;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using MergeState = org::apache::lucene::index::MergeState;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using SortedNumericDocValues =
    org::apache::lucene::index::SortedNumericDocValues;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using Accountable = org::apache::lucene::util::Accountable;
using Accountables = org::apache::lucene::util::Accountables;
using IOUtils = org::apache::lucene::util::IOUtils;
const wstring PerFieldDocValuesFormat::PER_FIELD_NAME = L"PerFieldDV40";
const wstring PerFieldDocValuesFormat::PER_FIELD_FORMAT_KEY =
    PerFieldDocValuesFormat::typeid->getSimpleName() + L".format";
const wstring PerFieldDocValuesFormat::PER_FIELD_SUFFIX_KEY =
    PerFieldDocValuesFormat::typeid->getSimpleName() + L".suffix";

PerFieldDocValuesFormat::PerFieldDocValuesFormat()
    : org::apache::lucene::codecs::DocValuesFormat(PER_FIELD_NAME)
{
}

shared_ptr<DocValuesConsumer> PerFieldDocValuesFormat::fieldsConsumer(
    shared_ptr<SegmentWriteState> state) 
{
  return make_shared<FieldsWriter>(shared_from_this(), state);
}

PerFieldDocValuesFormat::ConsumerAndSuffix::~ConsumerAndSuffix()
{
  delete consumer;
}

PerFieldDocValuesFormat::FieldsWriter::FieldsWriter(
    shared_ptr<PerFieldDocValuesFormat> outerInstance,
    shared_ptr<SegmentWriteState> state)
    : segmentWriteState(state), outerInstance(outerInstance)
{
}

void PerFieldDocValuesFormat::FieldsWriter::addNumericField(
    shared_ptr<FieldInfo> field,
    shared_ptr<DocValuesProducer> valuesProducer) 
{
  getInstance(field)->addNumericField(field, valuesProducer);
}

void PerFieldDocValuesFormat::FieldsWriter::addBinaryField(
    shared_ptr<FieldInfo> field,
    shared_ptr<DocValuesProducer> valuesProducer) 
{
  getInstance(field)->addBinaryField(field, valuesProducer);
}

void PerFieldDocValuesFormat::FieldsWriter::addSortedField(
    shared_ptr<FieldInfo> field,
    shared_ptr<DocValuesProducer> valuesProducer) 
{
  getInstance(field)->addSortedField(field, valuesProducer);
}

void PerFieldDocValuesFormat::FieldsWriter::addSortedNumericField(
    shared_ptr<FieldInfo> field,
    shared_ptr<DocValuesProducer> valuesProducer) 
{
  getInstance(field)->addSortedNumericField(field, valuesProducer);
}

void PerFieldDocValuesFormat::FieldsWriter::addSortedSetField(
    shared_ptr<FieldInfo> field,
    shared_ptr<DocValuesProducer> valuesProducer) 
{
  getInstance(field)->addSortedSetField(field, valuesProducer);
}

void PerFieldDocValuesFormat::FieldsWriter::merge(
    shared_ptr<MergeState> mergeState) 
{
  unordered_map<std::shared_ptr<DocValuesConsumer>, deque<wstring>>
      consumersToField =
          make_shared<IdentityHashMap<std::shared_ptr<DocValuesConsumer>,
                                      deque<wstring>>>();

  // Group each consumer by the fields it handles
  for (auto fi : mergeState->mergeFieldInfos) {
    shared_ptr<DocValuesConsumer> consumer = getInstance(fi);
    shared_ptr<deque<wstring>> fieldsForConsumer =
        consumersToField[consumer];
    if (fieldsForConsumer == nullptr) {
      fieldsForConsumer = deque<>();
      consumersToField.emplace(consumer, fieldsForConsumer);
    }
    fieldsForConsumer->add(fi->name);
  }

  // Delegate the merge to the appropriate consumer
  shared_ptr<PerFieldMergeState> pfMergeState =
      make_shared<PerFieldMergeState>(mergeState);
  try {
    for (auto e : consumersToField) {
      e.first::merge(pfMergeState->apply(e.second));
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    pfMergeState->reset();
  }
}

shared_ptr<DocValuesConsumer>
PerFieldDocValuesFormat::FieldsWriter::getInstance(
    shared_ptr<FieldInfo> field) 
{
  shared_ptr<DocValuesFormat> format = nullptr;
  if (field->getDocValuesGen() != -1) {
    const wstring formatName = field->getAttribute(PER_FIELD_FORMAT_KEY);
    // this means the field never existed in that segment, yet is applied
    // updates
    if (formatName != L"") {
      format = DocValuesFormat::forName(formatName);
    }
  }
  if (format == nullptr) {
    format = outerInstance->getDocValuesFormatForField(field->name);
  }
  if (format == nullptr) {
    throw make_shared<IllegalStateException>(
        L"invalid null DocValuesFormat for field=\"" + field->name + L"\"");
  }
  const wstring formatName = format->getName();

  wstring previousValue = field->putAttribute(PER_FIELD_FORMAT_KEY, formatName);
  if (field->getDocValuesGen() == -1 && previousValue != L"") {
    throw make_shared<IllegalStateException>(
        L"found existing value for " + PER_FIELD_FORMAT_KEY + L", field=" +
        field->name + L", old=" + previousValue + L", new=" + formatName);
  }

  optional<int> suffix = nullopt;

  shared_ptr<ConsumerAndSuffix> consumer = formats[format];
  if (consumer == nullptr) {
    // First time we are seeing this format; create a new instance

    if (field->getDocValuesGen() != -1) {
      const wstring suffixAtt = field->getAttribute(PER_FIELD_SUFFIX_KEY);
      // even when dvGen is != -1, it can still be a new field, that never
      // existed in the segment, and therefore doesn't have the recorded
      // attributes yet.
      if (suffixAtt != L"") {
        suffix = stoi(suffixAtt);
      }
    }

    if (!suffix) {
      // bump the suffix
      suffix = suffixes[formatName];
      if (!suffix) {
        suffix = 0;
      } else {
        suffix = suffix + 1;
      }
    }
    suffixes.emplace(formatName, suffix);

    // C++ TODO: There is no native C++ equivalent to 'toString':
    const wstring segmentSuffix =
        getFullSegmentSuffix(segmentWriteState->segmentSuffix,
                             getSuffix(formatName, Integer::toString(suffix)));
    consumer = make_shared<ConsumerAndSuffix>();
    consumer->consumer = format->fieldsConsumer(
        make_shared<SegmentWriteState>(segmentWriteState, segmentSuffix));
    consumer->suffix = suffix;
    formats.emplace(format, consumer);
  } else {
    // we've already seen this format, so just grab its suffix
    assert(suffixes.find(formatName) != suffixes.end());
    suffix = consumer->suffix;
  }

  // C++ TODO: There is no native C++ equivalent to 'toString':
  previousValue =
      field->putAttribute(PER_FIELD_SUFFIX_KEY, Integer::toString(suffix));
  if (field->getDocValuesGen() == -1 && previousValue != L"") {
    throw make_shared<IllegalStateException>(
        L"found existing value for " + PER_FIELD_SUFFIX_KEY + L", field=" +
        field->name + L", old=" + previousValue + L", new=" + suffix);
  }

  // TODO: we should only provide the "slice" of FIS
  // that this DVF actually sees ...
  return consumer->consumer;
}

PerFieldDocValuesFormat::FieldsWriter::~FieldsWriter()
{
  // Close all subs
  IOUtils::close({formats.values()});
}

wstring PerFieldDocValuesFormat::getSuffix(const wstring &formatName,
                                           const wstring &suffix)
{
  return formatName + L"_" + suffix;
}

wstring
PerFieldDocValuesFormat::getFullSegmentSuffix(const wstring &outerSegmentSuffix,
                                              const wstring &segmentSuffix)
{
  if (outerSegmentSuffix.length() == 0) {
    return segmentSuffix;
  } else {
    return outerSegmentSuffix + L"_" + segmentSuffix;
  }
}

PerFieldDocValuesFormat::FieldsReader::FieldsReader(
    shared_ptr<PerFieldDocValuesFormat> outerInstance,
    shared_ptr<FieldsReader> other) 
    : outerInstance(outerInstance)
{
  unordered_map<std::shared_ptr<DocValuesProducer>,
                std::shared_ptr<DocValuesProducer>>
      oldToNew =
          make_shared<IdentityHashMap<std::shared_ptr<DocValuesProducer>,
                                      std::shared_ptr<DocValuesProducer>>>();
  // First clone all formats
  for (auto ent : other->formats) {
    shared_ptr<DocValuesProducer> values = ent.second::getMergeInstance();
    formats.emplace(ent.first, values);
    oldToNew.emplace(ent.second, values);
  }

  // Then rebuild fields:
  for (auto ent : other->fields) {
    shared_ptr<DocValuesProducer> producer = oldToNew[ent.second];
    assert(producer != nullptr);
    fields.emplace(ent.first, producer);
  }
}

PerFieldDocValuesFormat::FieldsReader::FieldsReader(
    shared_ptr<PerFieldDocValuesFormat> outerInstance,
    shared_ptr<SegmentReadState> readState) 
    : outerInstance(outerInstance)
{

  // Init each unique format:
  bool success = false;
  try {
    // Read field name -> format name
    for (auto fi : readState->fieldInfos) {
      if (fi->getDocValuesType() != DocValuesType::NONE) {
        const wstring fieldName = fi->name;
        const wstring formatName = fi->getAttribute(PER_FIELD_FORMAT_KEY);
        if (formatName != L"") {
          // null formatName means the field is in fieldInfos, but has no
          // docvalues!
          const wstring suffix = fi->getAttribute(PER_FIELD_SUFFIX_KEY);
          if (suffix == L"") {
            throw make_shared<IllegalStateException>(
                L"missing attribute: " + PER_FIELD_SUFFIX_KEY +
                L" for field: " + fieldName);
          }
          shared_ptr<DocValuesFormat> format =
              DocValuesFormat::forName(formatName);
          wstring segmentSuffix = getFullSegmentSuffix(
              readState->segmentSuffix, getSuffix(formatName, suffix));
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

shared_ptr<NumericDocValues> PerFieldDocValuesFormat::FieldsReader::getNumeric(
    shared_ptr<FieldInfo> field) 
{
  shared_ptr<DocValuesProducer> producer = fields[field->name];
  return producer == nullptr ? nullptr : producer->getNumeric(field);
}

shared_ptr<BinaryDocValues> PerFieldDocValuesFormat::FieldsReader::getBinary(
    shared_ptr<FieldInfo> field) 
{
  shared_ptr<DocValuesProducer> producer = fields[field->name];
  return producer == nullptr ? nullptr : producer->getBinary(field);
}

shared_ptr<SortedDocValues> PerFieldDocValuesFormat::FieldsReader::getSorted(
    shared_ptr<FieldInfo> field) 
{
  shared_ptr<DocValuesProducer> producer = fields[field->name];
  return producer == nullptr ? nullptr : producer->getSorted(field);
}

shared_ptr<SortedNumericDocValues>
PerFieldDocValuesFormat::FieldsReader::getSortedNumeric(
    shared_ptr<FieldInfo> field) 
{
  shared_ptr<DocValuesProducer> producer = fields[field->name];
  return producer == nullptr ? nullptr : producer->getSortedNumeric(field);
}

shared_ptr<SortedSetDocValues>
PerFieldDocValuesFormat::FieldsReader::getSortedSet(
    shared_ptr<FieldInfo> field) 
{
  shared_ptr<DocValuesProducer> producer = fields[field->name];
  return producer == nullptr ? nullptr : producer->getSortedSet(field);
}

PerFieldDocValuesFormat::FieldsReader::~FieldsReader()
{
  IOUtils::close({formats.values()});
}

int64_t PerFieldDocValuesFormat::FieldsReader::ramBytesUsed()
{
  int64_t size = 0;
  for (auto entry : formats) {
    size += (entry.first->length() * Character::BYTES) +
            entry.second::ramBytesUsed();
  }
  return size;
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
PerFieldDocValuesFormat::FieldsReader::getChildResources()
{
  return Accountables::namedAccountables(L"format", formats);
}

void PerFieldDocValuesFormat::FieldsReader::checkIntegrity() 
{
  for (auto format : formats) {
    format->second.checkIntegrity();
  }
}

shared_ptr<DocValuesProducer>
PerFieldDocValuesFormat::FieldsReader::getMergeInstance() 
{
  return make_shared<FieldsReader>(outerInstance, shared_from_this());
}

wstring PerFieldDocValuesFormat::FieldsReader::toString()
{
  return L"PerFieldDocValues(formats=" + formats.size() + L")";
}

shared_ptr<DocValuesProducer> PerFieldDocValuesFormat::fieldsProducer(
    shared_ptr<SegmentReadState> state) 
{
  return make_shared<FieldsReader>(shared_from_this(), state);
}
} // namespace org::apache::lucene::codecs::perfield