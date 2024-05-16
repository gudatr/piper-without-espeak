#include <fstream>
#include <functional>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "json.hpp"
#include "piper.hpp"

using namespace std;
using json = nlohmann::json;

int main(int argc, char *argv[])
{
  piper::PiperConfig piperConfig;
  piper::Voice voice;

  if (argc < 2)
  {
    std::cerr << "Need voice model path" << std::endl;
    return 1;
  }

  if (argc < 4)
  {
    std::cerr << "Need output WAV path" << std::endl;
    return 1;
  }

  auto modelPath = std::string(argv[1]);
  auto outputPath = std::string(argv[3]);

  optional<piper::SpeakerId> speakerId;
  loadVoice(piperConfig, modelPath, modelPath + ".json", voice, speakerId,
            false);
  piper::initialize(piperConfig, "./ipa.data");

  // Output audio to WAV file
  ofstream audioFile(outputPath, ios::binary);

  piper::SynthesisResult result;
  uint32_t dataSize = 0;
  free(piper::textToVoice(piperConfig, voice, "This is a test.", result, dataSize));
  piper::terminate(piperConfig);

  // Verify that file has some data
  if (audioFile.tellp() < 10000)
  {
    std::cerr << "ERROR: Output file is smaller than expected!" << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "OK" << std::endl;

  return EXIT_SUCCESS;
}
