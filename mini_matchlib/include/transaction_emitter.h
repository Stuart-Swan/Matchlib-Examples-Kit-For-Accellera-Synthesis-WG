
#pragma once

#include <systemc.h>

struct default_emitter_tag {};
struct dollar_emitter_tag {};

//#define TRANSACTION_EMITTER dollar_emitter_tag

#ifndef TRANSACTION_EMITTER
#define TRANSACTION_EMITTER default_emitter_tag
#endif


// base class for options, specialized emitter options derive from this and add options
class transaction_emitter_options {
public:
};

// default transaction emittor
template <typename Message, typename T>
class transaction_emitter {
public:
  static void emit(std::ofstream* log_stream, const Message& m, 
      int log_number, const std::string& full_name, transaction_emitter_options* options) {
    if (log_stream) { 
      *log_stream << std::dec << log_number << " | " << std::hex <<  m << " | " << sc_time_stamp() << "\n"; 
    }
  }
};


// example of a specialized emitter, uses $ for separator:
template <typename Message>
class transaction_emitter<Message, dollar_emitter_tag> {
public:
  static void emit(std::ofstream* log_stream, const Message& m, 
      int log_number, const std::string& full_name, transaction_emitter_options* options) {
    if (log_stream) { 
      *log_stream << std::dec << log_number << " $ " << std::hex <<  m << " $ " << sc_time_stamp() << "\n"; 
    }
  }
};

// Note that partial specialization on the Message type is possible also (in addition to emitter type)

