#include "format.h"
#include <iomanip>
#include <sstream>
#include <string>

using std::string;
using std::to_string;

string Format::Format(int time) {
  string timeAsString = to_string(time);
  return string(2 - timeAsString.length(), '0') + timeAsString;
}

string Format::ElapsedTime(long seconds) {
  int hour = seconds / 3600;
  seconds %= 3600;
  int min = seconds / 60;
  seconds %= 60;
  int sec = seconds;

  return Format(hour) + ':' + Format(min) + ':' + Format(sec);
}

string Format::KBisMB(float kb) {
  float mb = kb / 1024;
  std::stringstream mb_stream;
  mb_stream << std::fixed << std::setprecision(1) << mb;
  return mb_stream.str();
}

// New ProgressBar function
string Format::ProgressBar(float percent) {
  string result{"["};
  int size = 50;
  int bars = percent * size;

  for (int i = 0; i < size; ++i) {
    if (i <= bars) {
      result += "|";
    } else {
      result += " ";
    }
  }

  result += "] ";
  result += to_string(percent * 100.0f).substr(0, 4);
  result += " /100%";
  return result;
}
