
#pragma once

#include <systemc-hls>

#include <connections_fifo.h>

namespace Connections {
  template <typename Message, int N>
  using msg_fifo = Connections::Fifo<Message, N>;
}
