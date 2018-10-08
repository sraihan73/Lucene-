using namespace std;

#include "POS.h"

namespace org::apache::lucene::analysis::ko
{

Tag Tag::E(L"E", InnerEnum::E, 100, L"Verbal endings");
Tag Tag::IC(L"IC", InnerEnum::IC, 110, L"Interjection");
Tag Tag::J(L"J", InnerEnum::J, 120, L"Ending Particle");
Tag Tag::MAG(L"MAG", InnerEnum::MAG, 130, L"General Adverb");
Tag Tag::MAJ(L"MAJ", InnerEnum::MAJ, 131, L"Conjunctive adverb");
Tag Tag::MM(L"MM", InnerEnum::MM, 140, L"Modifier");
Tag Tag::NNG(L"NNG", InnerEnum::NNG, 150, L"General Noun");
Tag Tag::NNP(L"NNP", InnerEnum::NNP, 151, L"Proper Noun");
Tag Tag::NNB(L"NNB", InnerEnum::NNB, 152, L"Dependent noun");
Tag Tag::NNBC(L"NNBC", InnerEnum::NNBC, 153, L"Dependent noun");
Tag Tag::NP(L"NP", InnerEnum::NP, 154, L"Pronoun");
Tag Tag::NR(L"NR", InnerEnum::NR, 155, L"Numeral");
Tag Tag::SF(L"SF", InnerEnum::SF, 160, L"Terminal punctuation");
Tag Tag::SH(L"SH", InnerEnum::SH, 161, L"Chinese Characeter");
Tag Tag::SL(L"SL", InnerEnum::SL, 162, L"Foreign language");
Tag Tag::SN(L"SN", InnerEnum::SN, 163, L"Number");
Tag Tag::SP(L"SP", InnerEnum::SP, 164, L"Space");
Tag Tag::SSC(L"SSC", InnerEnum::SSC, 165, L"Closing brackets");
Tag Tag::SSO(L"SSO", InnerEnum::SSO, 166, L"Opening brackets");
Tag Tag::SC(L"SC", InnerEnum::SC, 167, L"Separator");
Tag Tag::SY(L"SY", InnerEnum::SY, 168, L"Other symbol");
Tag Tag::SE(L"SE", InnerEnum::SE, 169, L"Ellipsis");
Tag Tag::VA(L"VA", InnerEnum::VA, 170, L"Adjective");
Tag Tag::VCN(L"VCN", InnerEnum::VCN, 171, L"Negative designator");
Tag Tag::VCP(L"VCP", InnerEnum::VCP, 172, L"Positive designator");
Tag Tag::VV(L"VV", InnerEnum::VV, 173, L"Verb");
Tag Tag::VX(L"VX", InnerEnum::VX, 174, L"Auxiliary Verb or Adjective");
Tag Tag::XPN(L"XPN", InnerEnum::XPN, 181, L"Prefix");
Tag Tag::XR(L"XR", InnerEnum::XR, 182, L"Root");
Tag Tag::XSA(L"XSA", InnerEnum::XSA, 183, L"Adjective Suffix");
Tag Tag::XSN(L"XSN", InnerEnum::XSN, 184, L"Noun Suffix");
Tag Tag::XSV(L"XSV", InnerEnum::XSV, 185, L"Verb Suffix");
Tag Tag::UNKNOWN(L"UNKNOWN", InnerEnum::UNKNOWN, 999, L"Unknown");
Tag Tag::UNA(L"UNA", InnerEnum::UNA, -1, L"Unknown");
Tag Tag::NA(L"NA", InnerEnum::NA, -1, L"Unknown");
Tag Tag::VSV(L"VSV", InnerEnum::VSV, -1, L"Unknown");

deque<Tag> Tag::valueList;

Tag::StaticConstructor::StaticConstructor()
{
  valueList.push_back(E);
  valueList.push_back(IC);
  valueList.push_back(J);
  valueList.push_back(MAG);
  valueList.push_back(MAJ);
  valueList.push_back(MM);
  valueList.push_back(NNG);
  valueList.push_back(NNP);
  valueList.push_back(NNB);
  valueList.push_back(NNBC);
  valueList.push_back(NP);
  valueList.push_back(NR);
  valueList.push_back(SF);
  valueList.push_back(SH);
  valueList.push_back(SL);
  valueList.push_back(SN);
  valueList.push_back(SP);
  valueList.push_back(SSC);
  valueList.push_back(SSO);
  valueList.push_back(SC);
  valueList.push_back(SY);
  valueList.push_back(SE);
  valueList.push_back(VA);
  valueList.push_back(VCN);
  valueList.push_back(VCP);
  valueList.push_back(VV);
  valueList.push_back(VX);
  valueList.push_back(XPN);
  valueList.push_back(XR);
  valueList.push_back(XSA);
  valueList.push_back(XSN);
  valueList.push_back(XSV);
  valueList.push_back(UNKNOWN);
  valueList.push_back(UNA);
  valueList.push_back(NA);
  valueList.push_back(VSV);
}

Tag::StaticConstructor Tag::staticConstructor;
int Tag::nextOrdinal = 0;

int POS::Tag::code() { return outerInstance->code_; }

wstring POS::Tag::description() { return outerInstance->desc; }

POS::Tag::Tag(const wstring &name, InnerEnum innerEnum,
              shared_ptr<POS> outerInstance, int code, const wstring &desc)
    : nameValue(name), ordinalValue(nextOrdinal++), innerEnumValue(innerEnum)
{
  this->outerInstance = outerInstance;
  this->code = code;
  this->desc = desc;
}

bool Tag::operator==(const Tag &other)
{
  return this->ordinalValue == other.ordinalValue;
}

bool Tag::operator!=(const Tag &other)
{
  return this->ordinalValue != other.ordinalValue;
}

deque<Tag> Tag::values() { return valueList; }

int Tag::ordinal() { return ordinalValue; }

wstring Tag::toString() { return nameValue; }

Tag Tag::valueOf(const wstring &name)
{
  for (auto enumInstance : Tag::valueList) {
    if (enumInstance.nameValue == name) {
      return enumInstance;
    }
  }
}

POS::Tag POS::resolveTag(const wstring &name)
{
  wstring tagUpper = name.toUpperCase(Locale::ENGLISH);
  if (StringHelper::startsWith(tagUpper, L"J")) {
    return Tag::J;
  } else if (StringHelper::startsWith(tagUpper, L"E")) {
    return Tag::E;
  } else {
    return Tag::valueOf(tagUpper);
  }
}

POS::Tag POS::resolveTag(char tag)
{
  assert(tag < Tag::values()->length);
  return Tag::values()[tag];
}

POS::Type POS::resolveType(const wstring &name)
{
  if (L"*" == name) {
    return Type::MORPHEME;
  }
  return Type::valueOf(name.toUpperCase(Locale::ENGLISH));
}

POS::Type POS::resolveType(char type)
{
  assert(type < Type::values()->length);
  return Type::values()[type];
}
} // namespace org::apache::lucene::analysis::ko