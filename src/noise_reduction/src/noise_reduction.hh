#pragma once

#include "../../common/src/speech_filter.hh"

namespace SpeechTools {

template <typename InType = std::vector<std::vector<float>>,
          typename OutType = std::vector<std::vector<float>>>
class NoiseFilter : public SpeechTools::SpeechFilter<InType, OutType> {
 public:
  template <typename QueueIn, typename QueueOut>
    requires QueueWithValueType<QueueIn, InType> &&
             QueueWithValueType<QueueOut, OutType>
  NoiseFilter(QueueIn& in, QueueOut& out)
      : SpeechTools::SpeechFilter<InType, OutType>(in, out) {}

 protected:
  virtual OutType process(const InType& in) override;
};
}  // namespace SpeechTools