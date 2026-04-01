

#pragma once

#include "new_connections.h"

#ifndef CONNECTIONS_NEG_RESET_SIGNAL_IS
#define CONNECTIONS_NEG_RESET_SIGNAL_IS(nm) async_reset_signal_is(nm, false)
#endif

#ifndef CONNECTIONS_RESET_SIGNAL_IS
#define CONNECTIONS_RESET_SIGNAL_IS(nm) CONNECTIONS_NEG_RESET_SIGNAL_IS(nm)
#endif


namespace Connections
{

  template <typename Message, connections_port_t port_marshall_type = AUTO_PORT>
  class FifoElem
  {
  public:
    // Interface
    sc_signal<Message> dat;
    Message init_val;

    FifoElem() : dat(sc_gen_unique_name("dat")) {}

    FifoElem(sc_module_name name) : dat(CONN_SYNTH_NAME(name, "dat")) {}

    void reset_state() {
      dat.write(init_val);
    }
  };

  //------------------------------------------------------------------------
  // Fifo
  //------------------------------------------------------------------------
  template <typename Message, unsigned int NumEntries, connections_port_t port_marshall_type = AUTO_PORT>
  class Fifo : public sc_module
  {
    SC_HAS_PROCESS(Fifo);

  public:
    // Interface
    sc_in_clk clk;
    sc_in<bool> rst;
    In<Message, port_marshall_type> enq;
    Out<Message, port_marshall_type> deq;

    Fifo()
      : sc_module(sc_module_name(sc_gen_unique_name("Fifo")))
      , clk("clk")
      , rst("rst")
      , enq(sc_gen_unique_name("enq"))
      , deq(sc_gen_unique_name("deq"))
    { 
      Init();
    }

    Fifo(sc_module_name name)
      : sc_module(name)
      , clk("clk")
      , rst("rst")
      , enq(CONN_SYNTH_NAME(name, "enq"))
      , deq(CONN_SYNTH_NAME(name, "deq"))
    {
      Init();
    }

  protected:
    static constexpr unsigned bits_needed(unsigned x) {
      return x == 0 ? 0 : 1 + bits_needed(x >> 1);
    }
    static const int AddrWidth = bits_needed(NumEntries);
    typedef sc_uint<AddrWidth> BuffIdx;
    typedef bool Bit;

    // Internal wires
    sc_signal<Bit> full_next;
    sc_signal<BuffIdx> head_next;
    sc_signal<BuffIdx> tail_next;

    // Internal state
    sc_signal<Bit> full;
    sc_signal<BuffIdx> head;
    sc_signal<BuffIdx> tail;
    FifoElem<Message, port_marshall_type> buffer[NumEntries];

    // Helper functions
    void Init() {
      #ifndef __SYNTHESIS__
      enq.disable_spawn();
      deq.disable_spawn();
      #endif

      SC_METHOD(EnqRdy);
      sensitive << full;

      SC_METHOD(DeqVal);
      sensitive << full << head << tail;

      SC_METHOD(DeqMsg);
      #ifndef __SYNTHESIS__
      sensitive << deq.rdy << full << head << tail;
      #else
      sensitive << tail;
      for(int i = 0; i < NumEntries; i++){
        sensitive << buffer[i].dat;
      }
      #endif

      SC_METHOD(HeadNext);
      sensitive << enq.vld << full << head;

      SC_METHOD(TailNext);
      sensitive << deq.rdy << full << head << tail;

      SC_METHOD(FullNext);
      sensitive << enq.vld << deq.rdy << full << head << tail;

      SC_THREAD(Seq);
      sensitive << clk.pos();
      CONNECTIONS_RESET_SIGNAL_IS(rst);;

      // Needed so that DeqMsg always has a good tail value
      tail.write(0);
    }

    // Combinational logic

    // Enqueue ready
    void EnqRdy() { enq.rdy.write(!full.read()); }

    // Dequeue valid
    void DeqVal() {
      bool empty = (!full.read() && (head.read() == tail.read()));
      deq.vld.write(!empty);
    }

    // Dequeue message
    void DeqMsg() {
      #ifndef __SYNTHESIS__
      bool empty = (!full.read() && (head.read() == tail.read()));
      bool do_deq = !empty;
      if (do_deq) {
      #endif
        deq.dat.write(buffer[tail.read()].dat.read());
      #ifndef __SYNTHESIS__
      } else {
        deq.dat.write(buffer[0].init_val);
      }
      #endif
    }

    // Head next calculations
    void HeadNext() {
      bool do_enq = (enq.vld.read() && !full.read());
      BuffIdx head_inc;
      if ((head.read() + 1) == NumEntries)
      { head_inc = 0; }
      else
      { head_inc = head.read() + 1; }

      if (do_enq)
      { head_next.write(head_inc); }
      else
      { head_next.write(head.read()); }
    }

    // Tail next calculations
    void TailNext() {
      bool empty = (!full.read() && (head.read() == tail.read()));
      bool do_deq = (deq.rdy.read() && !empty);
      BuffIdx tail_inc;
      if ((tail.read() + 1) == NumEntries)
      { tail_inc = 0; }
      else
      { tail_inc = tail.read() + 1; }

      if (do_deq)
      { tail_next.write(tail_inc); }
      else
      { tail_next.write(tail.read()); }
    }

    // Full next calculations
    void FullNext() {
      bool empty = (!full.read() && (head.read() == tail.read()));
      bool do_enq = (enq.vld.read() && !full.read());
      bool do_deq = (deq.rdy.read() && !empty);

      BuffIdx head_inc;
      if ((head.read() + 1) == NumEntries)
      { head_inc = 0; }
      else
      { head_inc = head.read() + 1; }

      if (do_enq && !do_deq && (head_inc == tail.read()))
      { full_next.write(1); }
      else if (do_deq && full.read())
      { full_next.write(0); }
      else
      { full_next.write(full.read()); }
    }

    // Sequential logic
    void Seq() {
      // Reset state
      full.write(0);
      head.write(0);
      tail.write(0);
      #pragma hls_unroll yes
      for (unsigned int i = 0; i < NumEntries; ++i)
      { buffer[i].reset_state(); }

      wait();

      while (1) {
        // Head update
        head.write(head_next);

        // Tail update
        tail.write(tail_next);

        // Full update
        full.write(full_next);

        // Enqueue message
        if (enq.vld.read() && !full.read()) {
          buffer[head.read()].dat.write(enq.dat.read());
        }

        wait();
      }
    }
  };

  // Specialization for depth=1 to enable II=1 after HLS
  template <typename Message>
  class Fifo<Message, 1> : public sc_module
  {
    SC_HAS_PROCESS(Fifo);

  public:
    sc_in_clk clk;
    sc_in<bool> rst;
    In<Message> enq;
    Out<Message> deq;

    Fifo()
      : sc_module(sc_module_name(sc_gen_unique_name("Fifo")))
      , clk("clk")
      , rst("rst")
      , enq(sc_gen_unique_name("enq"))
      , deq(sc_gen_unique_name("deq"))
    {
      Init();
    }

    Fifo(sc_module_name name)
      : sc_module(name)
      , clk("clk")
      , rst("rst")
      , enq(CONN_SYNTH_NAME(name, "enq"))
      , deq(CONN_SYNTH_NAME(name, "deq"))
    {
      Init();
    }

  protected:
    void Init() {
      SC_THREAD(Seq);
      sensitive << clk.pos();
      CONNECTIONS_RESET_SIGNAL_IS(rst);
    }

    void Seq() {
      enq.Reset();
      deq.Reset();
      wait();

      #pragma hls_pipeline_init_interval 1
      #pragma pipeline_stall_mode flush
      while (1) {
        deq.Push(enq.Pop());
      }
    }
  };


#ifndef __SYNTHESIS__
  //------------------------------------------------------------------------
  // Fifo - TLM_PORT specialization uses sized tlm::tlm_fifo
  //------------------------------------------------------------------------
  template <typename Message, unsigned int NumEntries>
  class Fifo<Message, NumEntries, TLM_PORT> : public sc_module
  {
    SC_HAS_PROCESS(Fifo);

  public:
    // Interface (clk and rst not used here, but kept for consistant bindings)
    sc_in_clk clk;
    sc_in<bool> rst;
    In<Message, TLM_PORT> enq;
    Out<Message, TLM_PORT> deq;

    Fifo()
      :sc_module(sc_module_name(sc_gen_unique_name("Fifo")))
      ,clk("clk")
      ,rst("rst")
      ,enq(sc_gen_unique_name("enq"))
      ,deq(sc_gen_unique_name("deq"))
      ,fifo(sc_gen_unique_name("fifo"), NumEntries)
    {
      SC_THREAD(tput);
      SC_THREAD(tget);
    }

    Fifo(sc_module_name name)
      :sc_module(name)
      ,clk("clk") 
      ,rst("rst") 
      ,enq(CONN_SYNTH_NAME(name, "enq"))
      ,deq(CONN_SYNTH_NAME(name, "deq"))
      ,fifo(CONN_SYNTH_NAME(name, "fifo"), NumEntries)
    {
      SC_THREAD(tput);
      SC_THREAD(tget);
    }

    void tput() {
      while (1) {
        fifo.put(enq.Pop());
      }
    }

    void tget() {
      while (1) {
        deq.Push(fifo.get());
      }
    }

  protected:
    tlm::tlm_fifo<Message> fifo;

  public:
  };
#endif 

}  // namespace Connections
