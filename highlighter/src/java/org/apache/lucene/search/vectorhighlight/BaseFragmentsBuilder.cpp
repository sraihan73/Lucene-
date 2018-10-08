using namespace std;

#include "BaseFragmentsBuilder.h"

namespace org::apache::lucene::search::vectorhighlight
{
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using IndexReader = org::apache::lucene::index::IndexReader;
using StoredFieldVisitor = org::apache::lucene::index::StoredFieldVisitor;
using DefaultEncoder = org::apache::lucene::search::highlight::DefaultEncoder;
using Encoder = org::apache::lucene::search::highlight::Encoder;
using SubInfo = org::apache::lucene::search::vectorhighlight::FieldFragList::
    WeightedFragInfo::SubInfo;
using WeightedFragInfo = org::apache::lucene::search::vectorhighlight::
    FieldFragList::WeightedFragInfo;
using Toffs = org::apache::lucene::search::vectorhighlight::FieldPhraseList::
    WeightedPhraseInfo::Toffs;
std::deque<wstring> const BaseFragmentsBuilder::COLORED_PRE_TAGS = {
    L"<b style=\"background:yellow\">",
    L"<b style=\"background:lawngreen\">",
    L"<b style=\"background:aquamarine\">",
    L"<b style=\"background:magenta\">",
    L"<b style=\"background:palegreen\">",
    L"<b style=\"background:coral\">",
    L"<b style=\"background:wheat\">",
    L"<b style=\"background:khaki\">",
    L"<b style=\"background:lime\">",
    L"<b style=\"background:deepskyblue\">",
    L"<b style=\"background:deeppink\">",
    L"<b style=\"background:salmon\">",
    L"<b style=\"background:peachpuff\">",
    L"<b style=\"background:violet\">",
    L"<b style=\"background:mediumpurple\">",
    L"<b style=\"background:palegoldenrod\">",
    L"<b style=\"background:darkkhaki\">",
    L"<b style=\"background:springgreen\">",
    L"<b style=\"background:turquoise\">",
    L"<b style=\"background:powderblue\">"};
std::deque<wstring> const BaseFragmentsBuilder::COLORED_POST_TAGS = {L"</b>"};

BaseFragmentsBuilder::BaseFragmentsBuilder()
    : BaseFragmentsBuilder(new std::wstring[]{L"<b>"}, new std::wstring[]{L"</b>"})
{
}

BaseFragmentsBuilder::BaseFragmentsBuilder(std::deque<wstring> &preTags,
                                           std::deque<wstring> &postTags)
    : BaseFragmentsBuilder(preTags, postTags, new SimpleBoundaryScanner())
{
}

BaseFragmentsBuilder::BaseFragmentsBuilder(
    shared_ptr<BoundaryScanner> boundaryScanner)
    : BaseFragmentsBuilder(new std::wstring[]{L"<b>"}, new std::wstring[]{L"</b>"},
                           boundaryScanner)
{
}

BaseFragmentsBuilder::BaseFragmentsBuilder(
    std::deque<wstring> &preTags, std::deque<wstring> &postTags,
    shared_ptr<BoundaryScanner> boundaryScanner)
    : boundaryScanner(boundaryScanner)
{
  this->preTags = preTags;
  this->postTags = postTags;
}

any BaseFragmentsBuilder::checkTagsArgument(any tags)
{
  if (dynamic_cast<wstring>(tags) != nullptr) {
    return tags;
  } else if (dynamic_cast<std::deque<wstring>>(tags) != nullptr) {
    return tags;
  }
  throw invalid_argument(
      L"type of preTags/postTags must be a std::wstring or std::wstring[]");
}

const shared_ptr<org::apache::lucene::search::highlight::Encoder>
    BaseFragmentsBuilder::NULL_ENCODER =
        make_shared<org::apache::lucene::search::highlight::DefaultEncoder>();

wstring BaseFragmentsBuilder::createFragment(
    shared_ptr<IndexReader> reader, int docId, const wstring &fieldName,
    shared_ptr<FieldFragList> fieldFragList) 
{
  return createFragment(reader, docId, fieldName, fieldFragList, preTags,
                        postTags, NULL_ENCODER);
}

std::deque<wstring>
BaseFragmentsBuilder::createFragments(shared_ptr<IndexReader> reader, int docId,
                                      const wstring &fieldName,
                                      shared_ptr<FieldFragList> fieldFragList,
                                      int maxNumFragments) 
{
  return createFragments(reader, docId, fieldName, fieldFragList,
                         maxNumFragments, preTags, postTags, NULL_ENCODER);
}

wstring BaseFragmentsBuilder::createFragment(
    shared_ptr<IndexReader> reader, int docId, const wstring &fieldName,
    shared_ptr<FieldFragList> fieldFragList, std::deque<wstring> &preTags,
    std::deque<wstring> &postTags,
    shared_ptr<Encoder> encoder) 
{
  std::deque<wstring> fragments = createFragments(
      reader, docId, fieldName, fieldFragList, 1, preTags, postTags, encoder);
  if (fragments.empty() || fragments.empty()) {
    return L"";
  }
  return fragments[0];
}

std::deque<wstring> BaseFragmentsBuilder::createFragments(
    shared_ptr<IndexReader> reader, int docId, const wstring &fieldName,
    shared_ptr<FieldFragList> fieldFragList, int maxNumFragments,
    std::deque<wstring> &preTags, std::deque<wstring> &postTags,
    shared_ptr<Encoder> encoder) 
{

  if (maxNumFragments < 0) {
    throw invalid_argument(L"maxNumFragments(" + to_wstring(maxNumFragments) +
                           L") must be positive number.");
  }

  deque<std::shared_ptr<WeightedFragInfo>> fragInfos =
      fieldFragList->getFragInfos();
  std::deque<std::shared_ptr<Field>> values =
      getFields(reader, docId, fieldName);
  if (values.empty()) {
    return nullptr;
  }

  if (discreteMultiValueHighlighting_ && values.size() > 1) {
    fragInfos = discreteMultiValueHighlighting(fragInfos, values);
  }

  fragInfos = getWeightedFragInfoList(fragInfos);
  int limitFragments =
      maxNumFragments < fragInfos.size() ? maxNumFragments : fragInfos.size();
  deque<wstring> fragments = deque<wstring>(limitFragments);

  shared_ptr<StringBuilder> buffer = make_shared<StringBuilder>();
  std::deque<int> nextValueIndex = {0};
  for (int n = 0; n < limitFragments; n++) {
    shared_ptr<WeightedFragInfo> fragInfo = fragInfos[n];
    fragments.push_back(makeFragment(buffer, nextValueIndex, values, fragInfo,
                                     preTags, postTags, encoder));
  }
  return fragments.toArray(std::deque<wstring>(fragments.size()));
}

std::deque<std::shared_ptr<Field>>
BaseFragmentsBuilder::getFields(shared_ptr<IndexReader> reader, int docId,
                                const wstring &fieldName) 
{
  // according to javadoc, doc.getFields(fieldName) cannot be used with lazy
  // loaded field???
  const deque<std::shared_ptr<Field>> fields =
      deque<std::shared_ptr<Field>>();
  reader->document(docId, make_shared<StoredFieldVisitorAnonymousInnerClass>(
                              shared_from_this(), fieldName, fields));
  return fields.toArray(std::deque<std::shared_ptr<Field>>(fields.size()));
}

BaseFragmentsBuilder::StoredFieldVisitorAnonymousInnerClass::
    StoredFieldVisitorAnonymousInnerClass(
        shared_ptr<BaseFragmentsBuilder> outerInstance,
        const wstring &fieldName, deque<std::shared_ptr<Field>> &fields)
{
  this->outerInstance = outerInstance;
  this->fieldName = fieldName;
  this->fields = fields;
}

void BaseFragmentsBuilder::StoredFieldVisitorAnonymousInnerClass::stringField(
    shared_ptr<FieldInfo> fieldInfo, std::deque<char> &bytes)
{
  wstring value = wstring(bytes, StandardCharsets::UTF_8);
  shared_ptr<FieldType> ft = make_shared<FieldType>(TextField::TYPE_STORED);
  ft->setStoreTermVectors(fieldInfo->hasVectors());
  fields.push_back(make_shared<Field>(fieldInfo->name, value, ft));
}

Status BaseFragmentsBuilder::StoredFieldVisitorAnonymousInnerClass::needsField(
    shared_ptr<FieldInfo> fieldInfo)
{
  return fieldInfo->name == fieldName ? Status::YES : Status::NO;
}

wstring BaseFragmentsBuilder::makeFragment(
    shared_ptr<StringBuilder> buffer, std::deque<int> &index,
    std::deque<std::shared_ptr<Field>> &values,
    shared_ptr<WeightedFragInfo> fragInfo, std::deque<wstring> &preTags,
    std::deque<wstring> &postTags, shared_ptr<Encoder> encoder)
{
  shared_ptr<StringBuilder> fragment = make_shared<StringBuilder>();
  constexpr int s = fragInfo->getStartOffset();
  std::deque<int> modifiedStartOffset = {s};
  wstring src = getFragmentSourceMSO(
      buffer, index, values, s, fragInfo->getEndOffset(), modifiedStartOffset);
  int srcIndex = 0;
  for (auto subInfo : fragInfo->getSubInfos()) {
    for (auto to : subInfo->getTermsOffsets()) {
      fragment
          ->append(encoder->encodeText(src.substr(
              srcIndex,
              (to->getStartOffset() - modifiedStartOffset[0]) - srcIndex)))
          ->append(getPreTag(preTags, subInfo->getSeqnum()))
          ->append(encoder->encodeText(
              src.substr(to->getStartOffset() - modifiedStartOffset[0],
                         (to->getEndOffset() - modifiedStartOffset[0]) -
                             (to->getStartOffset() - modifiedStartOffset[0]))))
          ->append(getPostTag(postTags, subInfo->getSeqnum()));
      srcIndex = to->getEndOffset() - modifiedStartOffset[0];
    }
  }
  fragment->append(encoder->encodeText(src.substr(srcIndex)));
  return fragment->toString();
}

wstring BaseFragmentsBuilder::getFragmentSourceMSO(
    shared_ptr<StringBuilder> buffer, std::deque<int> &index,
    std::deque<std::shared_ptr<Field>> &values, int startOffset, int endOffset,
    std::deque<int> &modifiedStartOffset)
{
  while (buffer->length() < endOffset && index[0] < values.size()) {
    buffer->append(values[index[0]++]->stringValue());
    buffer->append(getMultiValuedSeparator());
  }
  int bufferLength = buffer->length();
  // we added the multi value char to the last buffer, ignore it
  if (values[index[0] - 1]->fieldType().tokenized()) {
    bufferLength--;
  }
  int eo = bufferLength < endOffset
               ? bufferLength
               : boundaryScanner->findEndOffset(buffer, endOffset);
  modifiedStartOffset[0] =
      boundaryScanner->findStartOffset(buffer, startOffset);
  return buffer->substr(modifiedStartOffset[0], eo - modifiedStartOffset[0]);
}

wstring BaseFragmentsBuilder::getFragmentSource(
    shared_ptr<StringBuilder> buffer, std::deque<int> &index,
    std::deque<std::shared_ptr<Field>> &values, int startOffset, int endOffset)
{
  while (buffer->length() < endOffset && index[0] < values.size()) {
    buffer->append(values[index[0]]->stringValue());
    buffer->append(multiValuedSeparator);
    index[0]++;
  }
  int eo = buffer->length() < endOffset ? buffer->length() : endOffset;
  return buffer->substr(startOffset, eo - startOffset);
}

deque<std::shared_ptr<WeightedFragInfo>>
BaseFragmentsBuilder::discreteMultiValueHighlighting(
    deque<std::shared_ptr<WeightedFragInfo>> &fragInfos,
    std::deque<std::shared_ptr<Field>> &fields)
{
  unordered_map<wstring, deque<std::shared_ptr<WeightedFragInfo>>>
      fieldNameToFragInfos =
          unordered_map<wstring, deque<std::shared_ptr<WeightedFragInfo>>>();
  for (auto field : fields) {
    fieldNameToFragInfos.emplace(field->name(),
                                 deque<std::shared_ptr<WeightedFragInfo>>());
  }

  for (auto fragInfo : fragInfos) {
    int fieldStart;
    int fieldEnd = 0;
    for (auto field : fields) {
      if (field->stringValue().isEmpty()) {
        fieldEnd++;
        continue;
      }
      fieldStart = fieldEnd;
      fieldEnd += field->stringValue().length() +
                  1; // + 1 for going to next field with same name.

      if (fragInfo->getStartOffset() >= fieldStart &&
          fragInfo->getEndOffset() >= fieldStart &&
          fragInfo->getStartOffset() <= fieldEnd &&
          fragInfo->getEndOffset() <= fieldEnd) {
        fieldNameToFragInfos[field->name()].push_back(fragInfo);
        goto fragInfosContinue;
      }

      if (fragInfo->getSubInfos().empty()) {
        goto fragInfosContinue;
      }

      shared_ptr<Toffs> firstToffs =
          fragInfo->getSubInfos()[0]->getTermsOffsets()[0];
      if (fragInfo->getStartOffset() >= fieldEnd ||
          firstToffs->getStartOffset() >= fieldEnd) {
        continue;
      }

      int fragStart = fieldStart;
      if (fragInfo->getStartOffset() > fieldStart &&
          fragInfo->getStartOffset() < fieldEnd) {
        fragStart = fragInfo->getStartOffset();
      }

      int fragEnd = fieldEnd;
      if (fragInfo->getEndOffset() > fieldStart &&
          fragInfo->getEndOffset() < fieldEnd) {
        fragEnd = fragInfo->getEndOffset();
      }

      deque<std::shared_ptr<SubInfo>> subInfos =
          deque<std::shared_ptr<SubInfo>>();
      Iterator<std::shared_ptr<SubInfo>> subInfoIterator =
          fragInfo->getSubInfos().begin();
      float boost = 0.0f; //  The boost of the new info will be the sum of the
                          //  boosts of its SubInfos
      while (subInfoIterator->hasNext()) {
        shared_ptr<SubInfo> subInfo = subInfoIterator->next();
        deque<std::shared_ptr<Toffs>> toffsList =
            deque<std::shared_ptr<Toffs>>();
        Iterator<std::shared_ptr<Toffs>> toffsIterator =
            subInfo->getTermsOffsets().begin();
        while (toffsIterator->hasNext()) {
          shared_ptr<Toffs> toffs = toffsIterator->next();
          if (toffs->getStartOffset() >= fieldEnd) {
            // We've gone past this value so its not worth iterating any more.
            break;
          }
          bool startsAfterField = toffs->getStartOffset() >= fieldStart;
          bool endsBeforeField = toffs->getEndOffset() < fieldEnd;
          if (startsAfterField && endsBeforeField) {
            // The Toff is entirely within this value.
            toffsList.push_back(toffs);
            toffsIterator->remove();
          } else if (startsAfterField) {
            /*
             * The Toffs starts within this value but ends after this value
             * so we clamp the returned Toffs to this value and leave the
             * Toffs in the iterator for the next value of this field.
             */
            toffsList.push_back(
                make_shared<Toffs>(toffs->getStartOffset(), fieldEnd - 1));
          } else if (endsBeforeField) {
            /*
             * The Toffs starts before this value but ends in this value
             * which means we're really continuing from where we left off
             * above. Since we use the remainder of the offset we can remove
             * it from the iterator.
             */
            toffsList.push_back(
                make_shared<Toffs>(fieldStart, toffs->getEndOffset()));
            toffsIterator->remove();
          } else {
            /*
             * The Toffs spans the whole value so we clamp on both sides.
             * This is basically a combination of both arms of the loop
             * above.
             */
            toffsList.push_back(make_shared<Toffs>(fieldStart, fieldEnd - 1));
          }
          toffsIterator++;
        }
        if (!toffsList.empty()) {
          subInfos.push_back(make_shared<SubInfo>(subInfo->getText(), toffsList,
                                                  subInfo->getSeqnum(),
                                                  subInfo->getBoost()));
          boost += subInfo->getBoost();
        }

        if (subInfo->getTermsOffsets().empty()) {
          subInfoIterator->remove();
        }
        subInfoIterator++;
      }
      shared_ptr<WeightedFragInfo> weightedFragInfo =
          make_shared<WeightedFragInfo>(fragStart, fragEnd, subInfos, boost);
      fieldNameToFragInfos[field->name()].push_back(weightedFragInfo);
    }
  fragInfosContinue:;
  }
fragInfosBreak:

  deque<std::shared_ptr<WeightedFragInfo>> result =
      deque<std::shared_ptr<WeightedFragInfo>>();
  for (auto weightedFragInfos : fieldNameToFragInfos) {
    result.insert(result.end(), weightedFragInfos.second::begin(),
                  weightedFragInfos.second::end());
  }
  // C++ TODO: The 'Compare' parameter of std::sort produces a bool value,
  // while the Java Comparator parameter produces a tri-state result: ORIGINAL
  // LINE: java.util.Collections.sort(result, new
  // java.util.Comparator<org.apache.lucene.search.vectorhighlight.FieldFragList.WeightedFragInfo>()
  sort(result.begin(), result.end(),
       make_shared<ComparatorAnonymousInnerClass>(shared_from_this()));

  return result;
}

BaseFragmentsBuilder::ComparatorAnonymousInnerClass::
    ComparatorAnonymousInnerClass(
        shared_ptr<BaseFragmentsBuilder> outerInstance)
{
  this->outerInstance = outerInstance;
}

int BaseFragmentsBuilder::ComparatorAnonymousInnerClass::compare(
    shared_ptr<FieldFragList::WeightedFragInfo> info1,
    shared_ptr<FieldFragList::WeightedFragInfo> info2)
{
  return info1->getStartOffset() - info2->getStartOffset();
}

void BaseFragmentsBuilder::setMultiValuedSeparator(wchar_t separator)
{
  multiValuedSeparator = separator;
}

wchar_t BaseFragmentsBuilder::getMultiValuedSeparator()
{
  return multiValuedSeparator;
}

bool BaseFragmentsBuilder::isDiscreteMultiValueHighlighting()
{
  return discreteMultiValueHighlighting_;
}

void BaseFragmentsBuilder::setDiscreteMultiValueHighlighting(
    bool discreteMultiValueHighlighting)
{
  this->discreteMultiValueHighlighting_ = discreteMultiValueHighlighting;
}

wstring BaseFragmentsBuilder::getPreTag(int num)
{
  return getPreTag(preTags, num);
}

wstring BaseFragmentsBuilder::getPostTag(int num)
{
  return getPostTag(postTags, num);
}

wstring BaseFragmentsBuilder::getPreTag(std::deque<wstring> &preTags, int num)
{
  int n = num % preTags.size();
  return preTags[n];
}

wstring BaseFragmentsBuilder::getPostTag(std::deque<wstring> &postTags,
                                         int num)
{
  int n = num % postTags.size();
  return postTags[n];
}
} // namespace org::apache::lucene::search::vectorhighlight