using namespace std;

#include "SpanGradientFormatter.h"

namespace org::apache::lucene::search::highlight
{

SpanGradientFormatter::SpanGradientFormatter(float maxScore,
                                             const wstring &minForegroundColor,
                                             const wstring &maxForegroundColor,
                                             const wstring &minBackgroundColor,
                                             const wstring &maxBackgroundColor)
    : GradientFormatter(maxScore, minForegroundColor, maxForegroundColor,
                        minBackgroundColor, maxBackgroundColor)
{
}

wstring SpanGradientFormatter::highlightTerm(const wstring &originalText,
                                             shared_ptr<TokenGroup> tokenGroup)
{
  if (tokenGroup->getTotalScore() == 0) {
    return originalText;
  }
  float score = tokenGroup->getTotalScore();
  if (score == 0) {
    return originalText;
  }

  // try to size sb correctly
  shared_ptr<StringBuilder> sb =
      make_shared<StringBuilder>(originalText.length() + EXTRA);

  sb->append(L"<span style=\"");
  if (highlightForeground) {
    sb->append(L"color: ");
    sb->append(getForegroundColorString(score));
    sb->append(L"; ");
  }
  if (highlightBackground) {
    sb->append(L"background: ");
    sb->append(getBackgroundColorString(score));
    sb->append(L"; ");
  }
  sb->append(L"\">");
  sb->append(originalText);
  sb->append(L"</span>");
  return sb->toString();
}

const wstring SpanGradientFormatter::TEMPLATE =
    L"<span style=\"background: #EEEEEE; color: #000000;\">...</span>";
} // namespace org::apache::lucene::search::highlight