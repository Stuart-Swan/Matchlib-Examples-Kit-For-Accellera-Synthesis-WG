
#pragma once

#include "new_connections.h"

// proposed new naming scheme:
namespace msg_lib = Connections;

namespace Connections {

  template <typename Message, connections_port_t port_marshall_type = AUTO_PORT>
  using msg_chan = Combinational<Message, port_marshall_type> ;

  template <typename Message, connections_port_t port_marshall_type = AUTO_PORT>
  using msg_in = In<Message, port_marshall_type> ;

  template <typename Message, connections_port_t port_marshall_type = AUTO_PORT>
  using msg_out = Out<Message, port_marshall_type> ;

}
