
// compatibility stuff from new_connections.h to legacy connections.h

#pragma once

namespace Connections {

// Forward declarations below needed for other Matchlib headers currently

template <typename Message, connections_port_t port_marshall_type = AUTO_PORT>
class InBlocking;
template <typename Message, connections_port_t port_marshall_type = AUTO_PORT>
class OutBlocking;

template <typename Message, connections_port_t port_marshall_type = AUTO_PORT>
class Bypass;
template <typename Message, connections_port_t port_marshall_type = AUTO_PORT>
class Pipeline;
template <typename Message, unsigned int NumEntries, connections_port_t port_marshall_type = AUTO_PORT>
class Buffer;


} // namespace Connections

#ifndef __SYNTHESIS__
#define CONNECTIONS_SIM_ONLY
#endif

#define _VLDNAME_ vld
#define _RDYNAME_ rdy
#define _DATNAME_ dat
#define _VLDNAMESTR_ "vld"
#define _RDYNAMESTR_ "rdy"
#define _DATNAMESTR_ "dat"


#include "marshaller.h"
#include "connections_utils.h"
#include "connections_trace.h"
#include "message.h"
