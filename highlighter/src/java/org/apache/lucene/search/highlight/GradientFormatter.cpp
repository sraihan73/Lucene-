using namespace std;

#include "GradientFormatter.h"

namespace org::apache::lucene::search::highlight
{

GradientFormatter::GradientFormatter(float maxScore,
                                     const wstring &minForegroundColor,
                                     const wstring &maxForegroundColor,
                                     const wstring &minBackgroundColor,
                                     const wstring &maxBackgroundColor)
{
  highlightForeground =
      (minForegroundColor != L"") && (maxForegroundColor != L"");
  if (highlightForeground) {
    if (minForegroundColor.length() != 7) {
      throw invalid_argument(
          wstring(L"minForegroundColor is not 7 bytes long eg a hex ") +
          L"RGB value such as #FFFFFF");
    }
    if (maxForegroundColor.length() != 7) {
      throw invalid_argument(
          wstring(L"minForegroundColor is not 7 bytes long eg a hex ") +
          L"RGB value such as #FFFFFF");
    }
    fgRMin = hexToInt(minForegroundColor.substr(1, 2));
    fgGMin = hexToInt(minForegroundColor.substr(3, 2));
    fgBMin = hexToInt(minForegroundColor.substr(5, 2));

    fgRMax = hexToInt(maxForegroundColor.substr(1, 2));
    fgGMax = hexToInt(maxForegroundColor.substr(3, 2));
    fgBMax = hexToInt(maxForegroundColor.substr(5, 2));
  }

  highlightBackground =
      (minBackgroundColor != L"") && (maxBackgroundColor != L"");
  if (highlightBackground) {
    if (minBackgroundColor.length() != 7) {
      throw invalid_argument(
          wstring(L"minBackgroundColor is not 7 bytes long eg a hex ") +
          L"RGB value such as #FFFFFF");
    }
    if (maxBackgroundColor.length() != 7) {
      throw invalid_argument(
          wstring(L"minBackgroundColor is not 7 bytes long eg a hex ") +
          L"RGB value such as #FFFFFF");
    }
    bgRMin = hexToInt(minBackgroundColor.substr(1, 2));
    bgGMin = hexToInt(minBackgroundColor.substr(3, 2));
    bgBMin = hexToInt(minBackgroundColor.substr(5, 2));

    bgRMax = hexToInt(maxBackgroundColor.substr(1, 2));
    bgGMax = hexToInt(maxBackgroundColor.substr(3, 2));
    bgBMax = hexToInt(maxBackgroundColor.substr(5, 2));
  }
  //        this.corpusReader = corpusReader;
  this->maxScore = maxScore;
  //        totalNumDocs = corpusReader.numDocs();
}

wstring GradientFormatter::highlightTerm(const wstring &originalText,
                                         shared_ptr<TokenGroup> tokenGroup)
{
  if (tokenGroup->getTotalScore() == 0) {
    return originalText;
  }
  float score = tokenGroup->getTotalScore();
  if (score == 0) {
    return originalText;
  }
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  sb->append(L"<font ");
  if (highlightForeground) {
    sb->append(L"color=\"");
    sb->append(getForegroundColorString(score));
    sb->append(L"\" ");
  }
  if (highlightBackground) {
    sb->append(L"bgcolor=\"");
    sb->append(getBackgroundColorString(score));
    sb->append(L"\" ");
  }
  sb->append(L">");
  sb->append(originalText);
  sb->append(L"</font>");
  return sb->toString();
}

wstring GradientFormatter::getForegroundColorString(float score)
{
  int rVal = getColorVal(fgRMin, fgRMax, score);
  int gVal = getColorVal(fgGMin, fgGMax, score);
  int bVal = getColorVal(fgBMin, fgBMax, score);
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  sb->append(L"#");
  sb->append(intToHex(rVal));
  sb->append(intToHex(gVal));
  sb->append(intToHex(bVal));
  return sb->toString();
}

wstring GradientFormatter::getBackgroundColorString(float score)
{
  int rVal = getColorVal(bgRMin, bgRMax, score);
  int gVal = getColorVal(bgGMin, bgGMax, score);
  int bVal = getColorVal(bgBMin, bgBMax, score);
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  sb->append(L"#");
  sb->append(intToHex(rVal));
  sb->append(intToHex(gVal));
  sb->append(intToHex(bVal));
  return sb->toString();
}

int GradientFormatter::getColorVal(int colorMin, int colorMax, float score)
{
  if (colorMin == colorMax) {
    return colorMin;
  }
  float scale = abs(colorMin - colorMax);
  float relScorePercent = min(maxScore, score) / maxScore;
  float colScore = scale * relScorePercent;
  return min(colorMin, colorMax) + static_cast<int>(colScore);
}

std::deque<wchar_t> GradientFormatter::hexDigits = {
    L'0', L'1', L'2', L'3', L'4', L'5', L'6', L'7',
    L'8', L'9', L'A', L'B', L'C', L'D', L'E', L'F'};

wstring GradientFormatter::intToHex(int i)
{
  return L"" + StringHelper::toString(hexDigits[(i & 0xF0) >> 4]) +
         StringHelper::toString(hexDigits[i & 0x0F]);
}

int GradientFormatter::hexToInt(const wstring &hex)
{
  int len = hex.length();
  if (len > 16) {
    throw make_shared<NumberFormatException>();
  }

  int l = 0;
  for (int i = 0; i < len; i++) {
    l <<= 4;
    int c = Character::digit(hex[i], 16);
    if (c < 0) {
      throw make_shared<NumberFormatException>();
    }
    l |= c;
  }
  return l;
}
} // namespace org::apache::lucene::search::highlight