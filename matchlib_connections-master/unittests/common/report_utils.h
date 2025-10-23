#pragma once

#include <systemc.h>

#include <string>

namespace unittest {

struct ExpectedReport {
  std::string id;
  std::string substring;
  bool seen = false;
  bool unexpected_error = false;
  sc_report_handler_proc previous = nullptr;
};

inline ExpectedReport &state() {
  static ExpectedReport instance;
  return instance;
}

inline void report_handler(const sc_report &report, const sc_actions &actions) {
  auto &exp = state();
  if (!exp.id.empty() && exp.id == report.get_msg_type()) {
    const std::string msg = report.get_msg();
    if (exp.substring.empty() || msg.find(exp.substring) != std::string::npos) {
      exp.seen = true;
      sc_stop();
    }
  } else if (report.get_severity() == SC_ERROR) {
    exp.unexpected_error = true;
  }

  sc_report_handler::default_handler(report, actions);
}

inline void expect_error(const std::string &id, const std::string &substring = std::string()) {
  auto &exp = state();
  exp.id = id;
  exp.substring = substring;
  exp.seen = false;
  exp.unexpected_error = false;
  exp.previous = sc_report_handler::get_handler();
  sc_report_handler::set_handler(report_handler);
  sc_report_handler::set_actions(SC_ERROR, SC_DISPLAY);
}

inline bool finalize_expectation() {
  auto &exp = state();
  sc_report_handler::set_handler(exp.previous ? exp.previous : sc_report_handler::default_handler);
  return exp.seen && !exp.unexpected_error;
}

inline bool saw_expected_error() { return state().seen; }
inline bool saw_unexpected_error() { return state().unexpected_error; }

} // namespace unittest
