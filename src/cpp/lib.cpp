#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <vector>
#include <string>
#include <codecvt>
#include <locale>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define LIB_API extern "C" __declspec(dllexport) // Microsoft
#elif defined(__GNUC__)
#define LIB_API extern "C" __attribute__((visibility("default"))) // GCC
#else
#define LIB_API
#endif

#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "piper.hpp"
#include "wavfile.hpp"

using namespace std;

bool writeFile = false;
std::string outputPath = "./output/";

int currentSpeaker = 0;

LIB_API void LoadIPAData(const char *path)
{
  std::string str(path);
  piper::LoadIPAData(str);
}

LIB_API void ApplySynthesisConfig(float lengthScale, float noiseScale, float noiseW, int speakerId, float sentenceSilenceSeconds, float fadeTime, bool useCuda)
{
  currentSpeaker = speakerId;
  piper::ApplySynthesisConfig(lengthScale, noiseScale, noiseW, speakerId, sentenceSilenceSeconds, fadeTime, useCuda);
}

LIB_API void LoadVoice(int modelDataLength, const void *modelData)
{
  piper::LoadVoice(modelDataLength, modelData);
}

LIB_API void SetWriteToFile(bool enabled)
{
  writeFile = enabled;
}

LIB_API void SetOutputDirectory(const char *outputDirectory)
{
  std::string str(outputDirectory);
  outputPath = str;
}

LIB_API char *GetLastIPA(int *length)
{
  auto str = piper::GetLastIPA();

  *length = str.size();

  char *full_data = (char *)malloc(*length);
  memcpy(full_data, (const char *)str.data(), *length);

  return full_data;
}

LIB_API char *GenerateVoiceData(int *length, const char *text)
{
  uint32_t dataSize = 0;
  std::string str(text);
  auto data = piper::TextToVoice(str, dataSize);
  *length = dataSize;

  if (writeFile)
  {
    // Timestamp is used for path to output WAV file
    const auto now = chrono::system_clock::now();
    const auto timestamp =
        chrono::duration_cast<chrono::nanoseconds>(now.time_since_epoch())
            .count();
    // Generate path using timestamp
    stringstream outputName;
    outputName << outputPath << timestamp << "_" << currentSpeaker << ".wav";

    // Output audio to automatically-named WAV file in a directory
    ofstream audioFile(outputName.str(), ios::binary);
    auto header = createWavHeader(22050, 16, 1, dataSize / 2);
    audioFile.write(reinterpret_cast<const char *>(&header), sizeof(header));
    audioFile.write(data, dataSize);
    audioFile.close();
  }

  return data;
}

LIB_API void DiscardVoiceData(char *data)
{
  free(data);
}