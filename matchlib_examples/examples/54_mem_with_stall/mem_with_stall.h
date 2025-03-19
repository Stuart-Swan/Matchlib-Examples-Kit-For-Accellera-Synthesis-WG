
#pragma once

#include <mc_connections.h>

#include "sc_named.h"

#include "auto_gen_fields.h"
#include "stable_random.h"

#ifdef __SYNTHESIS__
#define SYNTH_NAME(prefix, nm) ""
#else
#define SYNTH_NAME(prefix, nm) (std::string(prefix) + nm ).c_str()
#endif

////////////////////////////////////////////
// The mem_with_stall protocol is for a RAM that has a stall signal originating in the RAM.

template <class ADDR_T, class DATA_T>
struct mem_with_stall_chan : public sc_channel {
  mem_with_stall_chan(sc_module_name nm) : sc_channel(nm) {}

  sc_signal<bool> SC_NAMED(stall);
  sc_signal<bool> SC_NAMED(read_en);
  sc_signal<bool> SC_NAMED(write_en);
  sc_signal<ADDR_T> SC_NAMED(address);
  sc_signal<DATA_T> SC_NAMED(write_data);
  sc_signal<DATA_T> SC_NAMED(read_data);
};

template <class ADDR_T, class DATA_T>
struct mem_with_stall_in {
  mem_with_stall_in(const char* nm = "") :
    stall(SYNTH_NAME(nm, "_stall"))
  , read_en(SYNTH_NAME(nm, "_read_en"))
  , write_en(SYNTH_NAME(nm, "_write_en"))
  , address(SYNTH_NAME(nm, "_address"))
  , write_data(SYNTH_NAME(nm, "_write_data"))
  , read_data(SYNTH_NAME(nm, "_read_data"))
  {}

  sc_out<bool> stall;
  sc_in<bool> read_en;
  sc_in<bool> write_en;
  sc_in<ADDR_T> address;
  sc_in<DATA_T> write_data;
  sc_out<DATA_T> read_data;

  template <class C>
  void operator()(C& c) {
     stall(c.stall);
     read_en(c.read_en);
     write_en(c.write_en);
     address(c.address);
     write_data(c.write_data);
     read_data(c.read_data);
  }
};

template <class ADDR_T, class DATA_T>
struct mem_with_stall_out {
  mem_with_stall_out(const char* nm = "") :
    stall(SYNTH_NAME(nm, "_stall"))
  , read_en(SYNTH_NAME(nm, "_read_en"))
  , write_en(SYNTH_NAME(nm, "_write_en"))
  , address(SYNTH_NAME(nm, "_address"))
  , write_data(SYNTH_NAME(nm, "_write_data"))
  , read_data(SYNTH_NAME(nm, "_read_data"))
  {}

  sc_in<bool> stall;
  sc_out<bool> read_en;
  sc_out<bool> write_en;
  sc_out<ADDR_T> address;
  sc_out<DATA_T> write_data;
  sc_in<DATA_T> read_data;

  template <class C>
  void operator()(C& c) {
     stall(c.stall);
     read_en(c.read_en);
     write_en(c.write_en);
     address(c.address);
     write_data(c.write_data);
     read_data(c.read_data);
  }
};

// transaction payloads for use with Push/Pop
template <class ADDR_T, class DATA_T>
struct mem_with_stall_req {
  bool read_en{0};
  bool write_en{0};
  ADDR_T address{0};
  DATA_T write_data{0};

  AUTO_GEN_FIELD_METHODS(mem_with_stall_req, ( \
     read_en \
   , write_en \
   , address \
   , write_data \
  ) )
  //
};

// transaction payloads for use with Push/Pop
template <class DATA_T>
struct mem_with_stall_rsp {
  DATA_T read_data{0};

  AUTO_GEN_FIELD_METHODS(mem_with_stall_rsp, ( \
    read_data \
  ) )
  //
};


//////////////////////

// Transactor to convert mem_with_stall_in to Push/Pop
template <class ADDR_T, class DATA_T>
struct mem_with_stall_in_xactor : public sc_module {
  sc_in<bool> SC_NAMED(clk);
  sc_in<bool> SC_NAMED(rst_bar);

  mem_with_stall_in<ADDR_T, DATA_T> SC_NAMED(mem_with_stall_in1);
  Connections::In<mem_with_stall_rsp<DATA_T>> SC_NAMED(rsp_chan);
  Connections::Out<mem_with_stall_req<ADDR_T, DATA_T>> SC_NAMED(req_chan);

  SC_CTOR(mem_with_stall_in_xactor) {
    SC_THREAD(main);
    sensitive << clk.pos();
    async_reset_signal_is(rst_bar, false);

#ifdef CONNECTIONS_SIM_ONLY
    rsp_chan.disable_spawn();
    req_chan.disable_spawn();
#endif
  }

  /// This is "RTL in SystemC"

#pragma implicit_fsm true
  void main() {
    req_chan.vld = 0;
    rsp_chan.rdy = 0;
    mem_with_stall_in1.stall = 0;
    wait();
    mem_with_stall_req<ADDR_T, DATA_T> req;
    bool got_req{0};

    while (1) {

      if (got_req && req.read_en) {
        rsp_chan.rdy = 1;
      } else {
        rsp_chan.rdy = 0;
      }

      wait();

      req_chan.vld = 0;

      if (got_req && req.read_en) {
        if (!rsp_chan.vld.read()) {
          CCS_LOG("Error rsp_chan not valid");
        }

        auto rsp = rsp_chan.dat.read();
        mem_with_stall_in1.read_data = rsp.read_data;
        rsp_chan.rdy = 0;
      }

      if (got_req) {
       if (!req_chan.rdy.read()) {
         CCS_LOG("Error: req_chan backpressure");
       }
       got_req = 0;
      }

      req.read_en = mem_with_stall_in1.read_en;
      req.write_en = mem_with_stall_in1.write_en;
      req.address = mem_with_stall_in1.address;
      req.write_data = mem_with_stall_in1.write_data;

      if (req.read_en || req.write_en) {
       // CCS_LOG("in_xactor: " << (int)req.address);
       req_chan.vld = 1;
       req_chan.dat = req;
       got_req = 1;
      }
    }
  }
};

// Transactor to convert Push/Pop to mem_with_stall_out
template <class ADDR_T, class DATA_T>
struct mem_with_stall_out_xactor : public sc_module {
  sc_in<bool> SC_NAMED(clk);
  sc_in<bool> SC_NAMED(rst_bar);

  mem_with_stall_out<ADDR_T, DATA_T> SC_NAMED(mem_with_stall_out1);
  Connections::In<mem_with_stall_req<ADDR_T, DATA_T>> SC_NAMED(req_chan);
  Connections::Out<mem_with_stall_rsp<DATA_T>> SC_NAMED(rsp_chan);

  SC_CTOR(mem_with_stall_out_xactor) {
    SC_THREAD(main);
    sensitive << clk.pos();
    async_reset_signal_is(rst_bar, false);

#ifdef CONNECTIONS_SIM_ONLY
    req_chan.disable_spawn();
    rsp_chan.disable_spawn();
#endif
  }

  // This is "RTL in SystemC"

#pragma implicit_fsm true
  void main() {
    mem_with_stall_req<ADDR_T, DATA_T> req;
    mem_with_stall_rsp<DATA_T> rsp;
    //rsp_chan.dat = rsp;
    rsp_chan.vld = 0;
    req_chan.rdy = 0;
    mem_with_stall_out1.read_en = 0;
    mem_with_stall_out1.write_en = 0;
    mem_with_stall_out1.address = 0;
    mem_with_stall_out1.write_data = 0;
    wait();

    bool got_req{0};
    bool got_rsp{0};
    bool do_read1{0};
    bool do_read2{0};

    while (1) {
      if (got_req) {
        mem_with_stall_out1.read_en = req.read_en;
        mem_with_stall_out1.write_en = req.write_en;
        mem_with_stall_out1.address = req.address;
        mem_with_stall_out1.write_data = req.write_data;
        got_req = 0;
        do_read1 = req.read_en;
      } else {
        mem_with_stall_out1.read_en = 0;
        mem_with_stall_out1.write_en = 0;
      }

      if (got_req)
        req_chan.rdy = 0;
      else
        req_chan.rdy = 1;

      do {
       wait();
       if (rsp_chan.vld.read()) {
        if (!rsp_chan.rdy.read()) // Any backpressure is an error..
        {
          CCS_LOG("Error: read response backpressure");
        }
        rsp_chan.vld = 0;
       }

       rsp_chan.vld = 0;

       if (!got_req && req_chan.vld.read()) {
        got_req = 1;
        req = req_chan.dat.read();
        req_chan.rdy = 0;
       }
      } while (mem_with_stall_out1.stall);

      if (do_read2) {
          do_read2 = 0;
          rsp.read_data = mem_with_stall_out1.read_data.read();
          rsp_chan.dat = rsp;
          rsp_chan.vld = 1;
      }

      do_read2 = do_read1;
      do_read1 = 0;
    }
  }
};


// mem_with_stall_memory is intended for use in SystemC testbench, not for HLS.
// Assumption is that actual implementation model comes from elsewhere

template <class ADDR_T, class DATA_T, int ELEMENTS>
struct mem_with_stall_memory : public sc_module {
  sc_in<bool> SC_NAMED(clk);
  sc_in<bool> SC_NAMED(rst_bar);

  bool debug{0};
  bool do_stall{0};

  mem_with_stall_in<ADDR_T, DATA_T> SC_NAMED(mem_with_stall_in1);

  SC_CTOR(mem_with_stall_memory) {
    SC_THREAD(main);
    sensitive << clk.pos();
    async_reset_signal_is(rst_bar, false);
  }

  DATA_T data[ELEMENTS];

#pragma implicit_fsm true
  void main() {
    ADDR_T read_adr;
    mem_with_stall_in1.read_data = 0;
    mem_with_stall_in1.stall = 0;
    wait();

    stable_random gen;

    while (1) {

      wait();

      if (!mem_with_stall_in1.stall) {
       ADDR_T adr = mem_with_stall_in1.address;

       if (mem_with_stall_in1.write_en) {
        assert(adr < ELEMENTS);
        data[adr] = mem_with_stall_in1.write_data;
        if (debug)
         CCS_LOG("mem: write addr data " << (int)adr << " " << (int)data[adr]);
       }

       if (mem_with_stall_in1.read_en) {
        assert(adr < ELEMENTS);
        mem_with_stall_in1.read_data = data[adr];
        if (debug)
         CCS_LOG("mem: read addr data " << (int)adr << " " << (int)data[adr]);
       }
      }

      mem_with_stall_in1.stall = 0;

      if (do_stall) {
        if ((gen.get() & 3) == 3) {
          mem_with_stall_in1.stall = 1;
        }
      }
    }
  }
};


////////////////
// Convenience classes for instantiating transactor modules in user designs

template <class ADDR_T, class DATA_T>
struct mem_with_stall_in_xact {
  std::string prefix;
  mem_with_stall_in_xactor<ADDR_T, DATA_T> in_xactor1;
  Connections::Combinational<mem_with_stall_rsp<DATA_T>> SC_NAMED(rsp_chan);
  Connections::Combinational<mem_with_stall_req<ADDR_T, DATA_T>> SC_NAMED(req_chan);

  mem_with_stall_in_xact(const char* s) : 
    prefix(s)
  , in_xactor1(SYNTH_NAME(prefix, "_xactor1"))
  , rsp_chan(SYNTH_NAME(prefix, "rsq_chan"))
  , req_chan(SYNTH_NAME(prefix, "req_chan"))
  {
  }

  template <class C, class R, class P>
  void bind(C& clk, R& rst_bar, P& in1) {
    in_xactor1.clk(clk);
    in_xactor1.rst_bar(rst_bar);
    in_xactor1.req_chan(req_chan);
    in_xactor1.rsp_chan(rsp_chan);
    in_xactor1.mem_with_stall_in1(in1);
  }
};


template <class ADDR_T, class DATA_T>
struct mem_with_stall_out_xact {
  std::string prefix;
  mem_with_stall_out_xactor<ADDR_T, DATA_T> out_xactor1;
  Connections::Combinational<mem_with_stall_rsp<DATA_T>> SC_NAMED(rsp_chan);
  Connections::Combinational<mem_with_stall_req<ADDR_T, DATA_T>> SC_NAMED(req_chan);

  typedef DATA_T data_t;
  typedef ADDR_T addr_t;

  mem_with_stall_out_xact(const char* s) : 
    prefix(s)
  , out_xactor1(SYNTH_NAME(prefix, "_xactor1"))
  , rsp_chan(SYNTH_NAME(prefix, "rsq_chan"))
  , req_chan(SYNTH_NAME(prefix, "req_chan"))
  {
  }

  template <class C, class R, class P>
  void bind(C& clk, R& rst_bar, P& out1) {
    out_xactor1.clk(clk);
    out_xactor1.rst_bar(rst_bar);
    out_xactor1.req_chan(req_chan);
    out_xactor1.rsp_chan(rsp_chan);
    out_xactor1.mem_with_stall_out1(out1);
  }

  struct elem_proxy {
    typedef mem_with_stall_out_xact::addr_t addr_t;
    typedef mem_with_stall_out_xact::data_t data_t;
    mem_with_stall_out_xact& xact;
    data_t idx;

    elem_proxy(mem_with_stall_out_xact& _xact, unsigned _idx)
     : xact(_xact), idx(_idx) {}

    operator data_t () {
      mem_with_stall_req<addr_t, data_t> req;
      req.write_en = 0;
      req.read_en = 1;
      req.address = idx;
      xact.req_chan.Push(req);
      auto rsp = xact.rsp_chan.Pop();
      return rsp.read_data;
    }

    void operator=(const mem_with_stall_out_xact::data_t& val) {
      mem_with_stall_req<addr_t, data_t> req;
      req.write_en = 1;
      req.read_en = 0;
      req.address = idx;
      req.write_data = val;
      xact.req_chan.Push(req);
    }
  };

  elem_proxy operator[](unsigned idx) { return elem_proxy(*this, idx); }
  const elem_proxy operator[](unsigned idx) const { return elem_proxy(*this, idx); }
};

