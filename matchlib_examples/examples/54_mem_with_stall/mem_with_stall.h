
#pragma once

#include <mc_connections.h>

#include "auto_gen_fields.h"

#include "stable_random.h"
#include "sc_named.h"

#include "ac_assert.h"

#ifdef __SYNTHESIS__
#define SYNTH_NAME(prefix, nm) ""
#else
#define SYNTH_NAME(prefix, nm) (std::string(prefix) + nm).c_str()
#endif

template <typename T_data, typename T_addr>
class mem_with_stall_chan : public sc_channel {
 public:
  mem_with_stall_chan(sc_module_name name) : sc_channel(name) {}

  sc_signal<bool> SC_NAMED(stall);
  sc_signal<bool> SC_NAMED(read_en);
  sc_signal<bool> SC_NAMED(write_en);
  sc_signal<T_addr> SC_NAMED(address);
  sc_signal<T_data> SC_NAMED(write_data);
  sc_signal<T_data> SC_NAMED(read_data);
};

// Memory side of interface
template <typename T_data, typename T_addr>
class mem_with_stall_in {
 public:
  mem_with_stall_in(const char* name = "")
      : stall(SYNTH_NAME(name, "_stall"))
      , read_en(SYNTH_NAME(name, "_read_en"))
      , write_en(SYNTH_NAME(name, "_write_en"))
      , address(SYNTH_NAME(name, "_address"))
      , write_data(SYNTH_NAME(name, "_write_data"))
      , read_data(SYNTH_NAME(name, "_read_data")) {}

  sc_out<bool> stall;
  sc_in<bool> read_en;
  sc_in<bool> write_en;
  sc_in<T_addr> address;
  sc_in<T_data> write_data;
  sc_out<T_data> read_data;

  template <class T_class>
  void operator()(T_class& in_val) {
    stall(in_val.stall);
    read_en(in_val.read_en);
    write_en(in_val.write_en);
    address(in_val.address);
    write_data(in_val.write_data);
    read_data(in_val.read_data);
  }
};

// memory controller side of interface
template <typename T_data, typename T_addr>
class mem_with_stall_out {
 public:
  mem_with_stall_out(const char* name = "")
      : stall(SYNTH_NAME(name, "_stall"))
      , read_en(SYNTH_NAME(name, "_read_en"))
      , write_en(SYNTH_NAME(name, "_write_en"))
      , address(SYNTH_NAME(name, "_address"))
      , write_data(SYNTH_NAME(name, "_write_data"))
      , read_data(SYNTH_NAME(name, "_read_data")) {}

  sc_in<bool> stall;
  sc_out<bool> read_en;
  sc_out<bool> write_en;
  sc_out<T_addr> address;
  sc_out<T_data> write_data;
  sc_in<T_data> read_data;

  template <class T_class>
  void operator()(T_class& in_val) {
    stall(in_val.stall);
    read_en(in_val.read_en);
    write_en(in_val.write_en);
    address(in_val.address);
    write_data(in_val.write_data);
    read_data(in_val.read_data);
  }

  void Reset() {
    read_en.write(false);
    write_en.write(false);
    address.write(0);
    write_data.write(0);
  }
};

// transaction payloads for use with Push/Pop
template <typename T_data, typename T_addr>
struct mem_with_stall_req {
  bool read_en{0};
  bool write_en{0};
  T_addr address{0};
  T_data write_data{0};

  AUTO_GEN_FIELD_METHODS(mem_with_stall_req, (read_en, write_en, address, write_data))
};

// transaction payloads for use with Push/Pop
template <class T_data>
struct mem_with_stall_rsp {
  T_data read_data{0};

  AUTO_GEN_FIELD_METHODS(mem_with_stall_rsp, (read_data))
};

// Transactor to convert mem_with_stall_in to Push/Pop
template <typename T_data, typename T_addr>
class mem_with_stall_in_xactor : public sc_module {
 public:
  sc_in<bool> SC_NAMED(clk);
  sc_in<bool> SC_NAMED(rst_n);

  mem_with_stall_in<T_data, T_addr> SC_NAMED(stallable_mem_slave);
  Connections::In<mem_with_stall_rsp<T_data>> SC_NAMED(rsp_chan);
  Connections::Out<mem_with_stall_req<T_data, T_addr>> SC_NAMED(req_chan);

  SC_CTOR(mem_with_stall_in_xactor) {
    SC_CTHREAD(SlaveThread, clk.pos());
    async_reset_signal_is(rst_n, false);

#ifdef CONNECTIONS_SIM_ONLY
    rsp_chan.disable_spawn();
    req_chan.disable_spawn();
#endif
  }

#pragma implicit_fsm true

  void SlaveThread() {
    req_chan.vld = false;
    rsp_chan.rdy = false;
    stallable_mem_slave.stall = false;
    wait();
    mem_with_stall_req<T_data, T_addr> req;
    bool req_received = false;

    while (1) {
      if (req_received && req.read_en) {
        rsp_chan.rdy = true;
      } else {
        rsp_chan.rdy = false;
      }

      wait();

      req_chan.vld = false;

      if (req_received && req.read_en) {
        if (!rsp_chan.vld.read()) {
          CCS_LOG("Error rsp_chan not valid");
        }

        auto rsp = rsp_chan.dat.read();
        stallable_mem_slave.read_data = rsp.read_data;
        rsp_chan.rdy = false;
      }

      if (req_received) {
        if (!req_chan.rdy.read()) {
          CCS_LOG("Error req_chan backpressure");
        }
        req_received = false;
      }

      req.read_en = stallable_mem_slave.read_en;
      req.write_en = stallable_mem_slave.write_en;
      req.address = stallable_mem_slave.address;
      req.write_data = stallable_mem_slave.write_data;

      if (req.read_en || req.write_en) {
        req_chan.vld = true;
        req_chan.dat = req;
        req_received = true;
      }
    }
  }
};

// Transactor to convert Push/Pop to mem_with_stall_out
template <typename T_data, typename T_addr>
class mem_with_stall_out_xactor : public sc_module {
 public:
  sc_in<bool> SC_NAMED(clk);
  sc_in<bool> SC_NAMED(rst_n);
  sc_in<bool> SC_NAMED(other_mem_stall);

  mem_with_stall_out<T_data, T_addr> SC_NAMED(stallable_mem_master);
  Connections::In<mem_with_stall_req<T_data, T_addr>> SC_NAMED(rd_req_chan);
  Connections::In<mem_with_stall_req<T_data, T_addr>> SC_NAMED(wr_req_chan);
  Connections::Out<mem_with_stall_rsp<T_data>> SC_NAMED(rsp_chan);

  sc_signal<bool> SC_NAMED(rd_start);
  sc_signal<bool> SC_NAMED(wr_req_delayed);
  sc_signal<bool> SC_NAMED(stall);

  SC_CTOR(mem_with_stall_out_xactor) {
#ifdef CONNECTIONS_SIM_ONLY
    rd_req_chan.disable_spawn();
    wr_req_chan.disable_spawn();
    rsp_chan.disable_spawn();
#endif

    SC_CTHREAD(rd_start_thread, clk.pos());
    async_reset_signal_is(rst_n, false);

    SC_CTHREAD(wr_req_delayed_thread, clk.pos());
    async_reset_signal_is(rst_n, false);

    SC_METHOD(rsp_vld);
    sensitive << rd_start << stall;

    SC_METHOD(rsp_dat);
    sensitive << stallable_mem_master.read_data;

    SC_METHOD(stall_method);
    sensitive << stallable_mem_master.stall << other_mem_stall;

    SC_METHOD(wr_req_rdy);
    sensitive << stall;

    SC_METHOD(rd_req_rdy);
    sensitive << stall;

    SC_METHOD(mem_read_en);
    sensitive << rd_req_chan.vld << rd_req_chan.rdy;

    SC_METHOD(mem_write_en);
    sensitive << wr_req_delayed << wr_req_chan.vld << rd_req_chan.vld 
              << wr_req_chan.rdy << rd_req_chan.rdy << stall;

    SC_METHOD(mem_address);
    sensitive << rd_req_chan.vld << wr_req_chan.dat << rd_req_chan.dat << wr_req_delayed;

    SC_METHOD(mem_wr_data);
    sensitive << wr_req_chan.dat;
  }

  void stall_method() {
    stall = stallable_mem_master.stall || other_mem_stall;
  }

  void rd_start_thread() {
    rd_start = 0;
    while (1) {
      wait();
      if (!stall)
        rd_start = rd_req_chan.vld && rd_req_chan.rdy;

      if (!stall && rsp_chan.vld) {
#ifdef HLS_ASSERTION
        HLS_ASSERTION(rsp_chan.rdy);  // Catapult 2025.3 and later
#else
        assert(rsp_chan.rdy);      // Catapult 2025.2 and earlier
#endif
      }
    }
  }

  void wr_req_delayed_thread() {
    wr_req_delayed = 0;
    while (1) {
      wait();
      if (!stall)
        wr_req_delayed = wr_req_chan.vld && rd_req_chan.vld && rd_req_chan.rdy && wr_req_chan.rdy;

      if (!stall) {
#ifdef HLS_ASSERTION
        HLS_ASSERTION(!wr_req_delayed || !rd_req_chan.vld); // Catapult 2025.3 and later
#else
        assert(!wr_req_delayed || !rd_req_chan.vld); // Catapult 2025.2 and earlier
#endif
      }
    }
  }

  void rsp_vld() {
    rsp_chan.vld = rd_start && !stall;
  }

  void rsp_dat() {
    mem_with_stall_rsp<T_data> rsp;
    rsp.read_data = stallable_mem_master.read_data;
    rsp_chan.dat = rsp;
  }

  void wr_req_rdy() {
    wr_req_chan.rdy = !stall;
  }

  void rd_req_rdy() {
    rd_req_chan.rdy = !stall;
  }

  void mem_read_en() {
    stallable_mem_master.read_en = rd_req_chan.vld && rd_req_chan.rdy;
  }

  void mem_write_en() {
    stallable_mem_master.write_en = !stall && 
      (wr_req_delayed || (!(rd_req_chan.vld && rd_req_chan.rdy)
         && (wr_req_chan.vld && wr_req_chan.rdy)));
  }

  void mem_address() {
    if (!wr_req_delayed && rd_req_chan.vld)
      stallable_mem_master.address = rd_req_chan.dat.read().address;
    else
      stallable_mem_master.address = wr_req_chan.dat.read().address;
  }

  void mem_wr_data() {
    stallable_mem_master.write_data = wr_req_chan.dat.read().write_data;
  }
};

// memory bfm
template <typename T_data, typename T_addr, int MEM_SIZE>
class mem_with_stall_memory : public sc_module {
 public:
  sc_in<bool> SC_NAMED(clk);
  sc_in<bool> SC_NAMED(rst_n);

  bool debug{false};
  bool do_stall{false};

  mem_with_stall_in<T_data, T_addr> SC_NAMED(stallable_mem_slave);

  SC_CTOR(mem_with_stall_memory) {
    SC_CTHREAD(ModelThread, clk.pos());
    async_reset_signal_is(rst_n, false);
  }

  T_data mem[MEM_SIZE];

  void ModelThread() {
    T_addr read_addr;
    stallable_mem_slave.read_data = 0;
    stallable_mem_slave.stall = false;
    wait();

    stable_random gen;

    while (1) {
      wait();

      if (!stallable_mem_slave.stall.read()) {
        read_addr = stallable_mem_slave.address.read();

        if (stallable_mem_slave.write_en.read()) {
          assert(read_addr < MEM_SIZE);
          mem[read_addr] = stallable_mem_slave.write_data.read();
          if (debug) {
            CCS_LOG(
                "mem: write addr data " << (int)read_addr << " " << (int)mem[read_addr]
            );
          }
        }
        if (stallable_mem_slave.read_en.read()) {
          assert(read_addr < MEM_SIZE);
          stallable_mem_slave.read_data = mem[read_addr];
          if (debug) {
            CCS_LOG("mem: read addr data " << (int)read_addr << " " << (int)mem[read_addr]);
          }
        }
      }

      stallable_mem_slave.stall = false;

      if (do_stall) {
        if ((gen.get() & 3) == 3) {
          stallable_mem_slave.stall = true;
        }
      }
    }
  }
};

template <typename T_data, typename T_addr>
class mem_with_stall_in_xact {
 public:
  std::string prefix;
  mem_with_stall_in_xactor<T_data, T_addr> slave_xactor;
  Connections::Combinational<mem_with_stall_rsp<T_data>> SC_NAMED(rsp_chan);
  Connections::Combinational<mem_with_stall_req<T_data, T_addr>> SC_NAMED(req_chan);

  mem_with_stall_in_xact(const char* s)
      : prefix(s)
      , slave_xactor(SYNTH_NAME(prefix, "_xactor1"))
      , rsp_chan(SYNTH_NAME(prefix, "rsq_chan"))
      , req_chan(SYNTH_NAME(prefix, "req_chan")) {}

  template <class C, class R, class P>
  void bind(C& clk, R& rst_bar, P& in1) {
    slave_xactor.clk(clk);
    slave_xactor.rst_bar(rst_bar);
    slave_xactor.req_chan(req_chan);
    slave_xactor.rsp_chan(rsp_chan);
    slave_xactor.stallable_mem_slave(in1);
  }
};

template <typename T_data, typename T_addr>
class mem_with_stall_out_xact {
 public:
  std::string prefix;
  mem_with_stall_out_xactor<T_data, T_addr> master_xactor;
  Connections::Combinational<mem_with_stall_rsp<T_data>> rsp_chan;
  Connections::Combinational<mem_with_stall_req<T_data, T_addr>> rd_req_chan;
  Connections::Combinational<mem_with_stall_req<T_data, T_addr>> wr_req_chan;

  using T_DATA = T_data;
  using T_ADDR = T_addr;

  mem_with_stall_out_xact(const char* s)
      : prefix(s)
      , master_xactor(SYNTH_NAME(prefix, "_xactor"))
      , rsp_chan(SYNTH_NAME(prefix, "rsp_chan"))
      , rd_req_chan(SYNTH_NAME(prefix, "rd_req_chan"))
      , wr_req_chan(SYNTH_NAME(prefix, "wr_req_chan")) {}

  template <typename T_element_data, typename T_element_addr>
  class elem_proxy {
   public:
    using T_master_xact = mem_with_stall_out_xact<T_element_data, T_element_addr>;
    T_master_xact& xact;
    T_element_data idx;

    elem_proxy(T_master_xact& _xact, T_element_addr _idx) : xact(_xact), idx(_idx) {}

    operator T_element_data() {
      mem_with_stall_req<T_element_data, T_element_addr> req;
      req.write_en = false;
      req.read_en = true;
      req.address = idx;
      xact.rd_req_chan.Push(req);
      auto rsp = xact.rsp_chan.Pop();
      return rsp.read_data;
    }

    void operator=(const T_element_data& val) {
      mem_with_stall_req<T_element_data, T_element_addr> req;
      req.write_en = true;
      req.read_en = false;
      req.address = idx;
      req.write_data = val;
      xact.wr_req_chan.Push(req);
    }
  };

  void Reset() {
    rsp_chan.ResetRead();
    rd_req_chan.ResetWrite();
    wr_req_chan.ResetWrite();
  }

  template <class C, class R, class P, class B>
  void bind(C& clk, R& rst_n, P& master, B& stall) {
    master_xactor.clk(clk);
    master_xactor.rst_n(rst_n);
    master_xactor.stallable_mem_master(master);
    master_xactor.rd_req_chan(rd_req_chan);
    master_xactor.wr_req_chan(wr_req_chan);
    master_xactor.rsp_chan(rsp_chan);
    master_xactor.other_mem_stall(stall);
  }

  elem_proxy<T_data, T_addr> operator[](T_addr idx) {
    return elem_proxy<T_data, T_addr>(*this, idx);
  }

  const elem_proxy<T_data, T_addr> operator[](T_addr idx) const {
    return elem_proxy<T_data, T_addr>(*this, idx);
  }
};
