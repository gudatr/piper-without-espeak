#ifndef PIPER_H_
#define PIPER_H_

#include <fstream>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include <onnxruntime_cxx_api.h>

#include "json.hpp"

using json = nlohmann::json;

namespace piper
{
  typedef char32_t Phoneme;

  typedef int64_t PhonemeId;

  typedef int64_t SpeakerId;

  struct SynthesisConfig
  {
    std::string modelPath = "";

    // VITS inference settings
    float noiseScale = 0.667f;
    float lengthScale = 1.0f;
    float noiseW = 0.8f;

    bool useCuda = false;

    // Audio settings
    int sampleRate = 22050;
    int sampleWidth = 2; // 16-bit
    int channels = 1;    // mono

    // Speaker id from 0 to numSpeakers - 1
    SpeakerId speakerId = 0;

    // Extra silence
    float sentenceSilenceSeconds = 0.2f;
    std::map<piper::Phoneme, float> phonemeSilenceSeconds;
  };

  struct ModelSession
  {
    Ort::Session onnx;
    Ort::AllocatorWithDefaultOptions allocator;
    Ort::SessionOptions options;
    Ort::Env env;

    ModelSession() : onnx(nullptr){};
  };

  struct Voice
  {
    SynthesisConfig synthesisConfig;
    ModelSession session;
  };

  // Must be called before using textTo* functions
  void LoadIPAData(const char *ipaPath);

  void ApplySynthesisConfig(float lengthScale, float noiseScale, float noiseW, int speakerId, int sampleRate, float sentenceSilenceSeconds, bool useCuda);

  // Load Onnx model and JSON config file
  void LoadVoice(const char *modelPath);

  // Phonemize text and synthesize audio
  void TextToAudio(Voice &voice, const char *text, std::vector<int16_t> &audioBuffer);

  // Phonemize text and synthesize audio to WAV file
  char *TextToVoice(const char *text, uint32_t &dataSize);

} // namespace piper

#endif // PIPER_H_
