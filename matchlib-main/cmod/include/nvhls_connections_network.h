/*
 * Copyright (c) 2016-2022, NVIDIA CORPORATION.  All rights reserved.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License")
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//========================================================================
// nvhls_connections.h
//========================================================================

#ifndef NVHLS_CONNECTIONS_NETWORK_H_
#define NVHLS_CONNECTIONS_NETWORK_H_

#include <systemc.h>
#include <nvhls_connections.h>
#include <nvhls_packet.h>

namespace Connections {

//------------------------------------------------------------------------
// InNetwork
//------------------------------------------------------------------------

template <typename Message, unsigned int DestWidthPerHop, unsigned int MaxHops,
          unsigned int PacketIdWidth>
class InNetwork : public sc_module {
  SC_HAS_PROCESS(InNetwork);

 public:
  static const int kDebugLevel = 3;
  typedef Wrapped<Message> WMessage;
  static const unsigned int width = WMessage::width;
  typedef sc_lv<WMessage::width> MsgBits;
  typedef Packet<WMessage::width, DestWidthPerHop, MaxHops, PacketIdWidth>
      Packet_t;

  // Interface
  sc_in_clk clk;
  sc_in<bool> rst;
  In<Packet_t> enq;
  Out<Message> deq;

  InNetwork()
      : sc_module(sc_module_name(sc_gen_unique_name("in_network"))),
        clk("clk"),
        rst("rst") {
    Init();
  }

  InNetwork(sc_module_name name) : sc_module(name), clk("clk"), rst("rst") {
    Init();
  }

 protected:
  void Init() {
#ifdef CONNECTIONS_SIM_ONLY
    enq.disable_spawn();
    deq.disable_spawn();
#endif
    
    SC_METHOD(AssignMsg);
    sensitive << enq._DATNAME_ << rst;

    SC_METHOD(AssignVal);
    sensitive << enq._VLDNAME_;

    SC_METHOD(AssignRdy);
    sensitive << deq._RDYNAME_;
  }

  void AssignMsg() {
    if (!rst.read()) {
      deq._DATNAME_.write(0);
    } else if (enq._VLDNAME_.read()) {
      sc_lv<Wrapped<Packet_t>::width> pbits = enq._DATNAME_.read();
      Marshaller<Wrapped<Packet_t>::width> pmarshaller(pbits);
      Packet_t packet;
      packet.Marshall(pmarshaller);
      MsgBits mbits(packet.data);
      deq._DATNAME_.write(mbits);
    }
  }

  void AssignVal() { deq._VLDNAME_.write(enq._VLDNAME_.read()); }
  void AssignRdy() { enq._RDYNAME_.write(deq._RDYNAME_.read()); }

#ifndef __SYNTHESIS__
 public:
  void line_trace() {
    if (rst.read()) {
      unsigned int pwidth = (Packet_t::width / 4);
      // Enqueue port
      if (enq._VLDNAME_.read() && enq._RDYNAME_.read()) {
        CDCOUT(std::hex << std::setw(pwidth) << enq._DATNAME_.read(), kDebugLevel);
      } else {
        CDCOUT(std::setw(pwidth + 1) << " ", kDebugLevel);
      }
      CDCOUT(" | ", kDebugLevel);

      // Dequeue port
      unsigned int mwidth = (Message().length() / 4);
      if (deq._VLDNAME_.read() && deq._RDYNAME_.read()) {
        CDCOUT(std::hex << std::setw(mwidth) << deq._DATNAME_.read(), kDebugLevel);
      } else {
        CDCOUT(std::setw(mwidth + 1) << " ", kDebugLevel);
      }
      CDCOUT(" | ", kDebugLevel);
    }
  }
#endif
};

//------------------------------------------------------------------------
// OutNetwork
//------------------------------------------------------------------------

template <typename Message, unsigned int DestWidthPerHop, unsigned int MaxHops,
          unsigned int PacketIdWidth>
class OutNetwork : public sc_module {
  SC_HAS_PROCESS(OutNetwork);

 public:
  static const int kDebugLevel = 3;
  typedef Wrapped<Message> WMessage;
  static const unsigned int width = WMessage::width;
  typedef sc_lv<WMessage::width> MsgBits;
  typedef Packet<WMessage::width, DestWidthPerHop, MaxHops, PacketIdWidth>
      Packet_t;

  // Interface
  sc_in_clk clk;
  sc_in<bool> rst;
  In<Message> enq;
  Out<Packet_t> deq;
  In<sc_lv<DestWidthPerHop * MaxHops> > route;
  In<sc_lv<PacketIdWidth> > id;

  // Internal State
  sc_signal<sc_lv<DestWidthPerHop * MaxHops> > route_state;
  sc_signal<sc_lv<PacketIdWidth> > id_state;

  OutNetwork()
      : sc_module(sc_module_name(sc_gen_unique_name("out_network"))),
        clk("clk"),
        rst("rst") {
    Init();
  }

  OutNetwork(sc_module_name name) : sc_module(name), clk("clk"), rst("rst") {
    Init();
  }

 protected:
  void Init() {
#ifdef CONNECTIONS_SIM_ONLY
    enq.disable_spawn();
    deq.disable_spawn();
    route.disable_spawn();
    id.disable_spawn();
#endif
    
    SC_METHOD(AssignMsg);
    sensitive << enq._DATNAME_ << id_state << route_state << rst;

    SC_METHOD(AssignVal);
    sensitive << enq._VLDNAME_;

    SC_METHOD(AssignRdy);
    sensitive << deq._RDYNAME_;

    SC_METHOD(TieToHigh);
    sensitive << clk.pos();

    SC_THREAD(SetState);
    sensitive << clk.pos();
    NVHLS_NEG_RESET_SIGNAL_IS(rst);
  }

  void AssignMsg() {
    if (!rst.read()) {
      deq._DATNAME_.write(0);
    } else if (enq._VLDNAME_.read()) {
      Packet_t packet;
      vector_to_type(id_state.read(), false, &packet.packet_id);
      vector_to_type(route_state.read(), false, &packet.dest);
      vector_to_type(enq._DATNAME_.read(), false, &packet.data);
      Marshaller<Wrapped<Packet_t>::width> pmarshaller;
      packet.Marshall(pmarshaller);
      deq._DATNAME_.write(pmarshaller.GetResult());
    }
  }

  void AssignVal() { deq._VLDNAME_.write(enq._VLDNAME_.read()); }
  void AssignRdy() { enq._RDYNAME_.write(deq._RDYNAME_.read()); }

  void TieToHigh() {
    route._RDYNAME_.write(1);
    id._RDYNAME_.write(1);
  }

  void SetState() {
    id_state.write(0);
    route_state.write(0);
    wait();
    while (1) {
      if (route._VLDNAME_.read()) {
        route_state.write(route._DATNAME_.read());
      }

      if (id._VLDNAME_.read()) {
        id_state.write(id._DATNAME_.read());
      }
      wait();
    }
  }

#ifndef __SYNTHESIS__
 public:
  void line_trace() {
    if (rst.read()) {
      unsigned int mwidth = (Message().length() / 4);
      // Enqueue port
      if (enq._VLDNAME_.read() && enq._RDYNAME_.read()) {
        CDCOUT(std::hex << std::setw(mwidth) << enq._DATNAME_.read(), kDebugLevel);
      } else {
        CDCOUT(std::setw(mwidth + 1) << " ", kDebugLevel);
      }
      CDCOUT(" | ", kDebugLevel);

      // Dequeue port
      unsigned int pwidth = (Packet_t::width / 4);
      if (deq._VLDNAME_.read() && deq._RDYNAME_.read()) {
        CDCOUT(std::hex << std::setw(pwidth) << deq._DATNAME_.read(), kDebugLevel);
      } else {
        CDCOUT(std::setw(pwidth + 1) << " ", kDebugLevel);
      }
      CDCOUT(" | ", kDebugLevel);
    }
  }
#endif
};

//------------------------------------------------------------------------
// InNetworkCredit
//------------------------------------------------------------------------
// NOTE: Currently, the PacketIdWidth, DestWidthPerHop, MaxHops parameters
// are assumed to be the same for a packet and a credit_packet

template <typename Message, unsigned int DestWidthPerHop, unsigned int MaxHops,
          unsigned int PacketIdWidth, unsigned int CreditWidth>
class InNetworkCredit : public sc_module {
  SC_HAS_PROCESS(InNetworkCredit);

 public:
  static const int kDebugLevel = 3;
  typedef Wrapped<Message> WMessage;
  static const unsigned int width = WMessage::width;
  typedef sc_lv<WMessage::width> MsgBits;
  typedef Packet<WMessage::width, DestWidthPerHop, MaxHops, PacketIdWidth>
      Packet_t;
  typedef Packet<CreditWidth, DestWidthPerHop, MaxHops, PacketIdWidth>
      CreditPacket_t;
  typedef sc_lv<CreditWidth> Credit_t;

  // Interface
  sc_in_clk clk;
  sc_in<bool> rst;
  In<Packet_t> enq;
  Out<Message> deq;
  Out<CreditPacket_t> credit;
  In<Credit_t> init_credits;
  In<sc_lv<DestWidthPerHop * MaxHops> > credit_route;
  In<sc_lv<PacketIdWidth> > credit_id;

  // Sub Module
  OutNetwork<Credit_t, DestWidthPerHop, MaxHops, PacketIdWidth> credit_out;

  // Wire
  Combinational<Credit_t> credit_enq;

  // Internal State
  sc_signal<Credit_t> credits;
  sc_signal<Credit_t> credits_next;

  InNetworkCredit()
      : sc_module(sc_module_name(sc_gen_unique_name("in_nw_credit"))),
        clk("clk"),
        rst("rst"),
        credit_out("credit_out") {
    Init();
  }

  InNetworkCredit(sc_module_name name)
      : sc_module(name), clk("clk"), rst("rst"), credit_out("credit_out") {
    Init();
  }

 protected:
  void Init() {
#ifdef CONNECTIONS_SIM_ONLY
    enq.disable_spawn();
    deq.disable_spawn();
    credit.disable_spawn();
    init_credits.disable_spawn();
    credit_route.disable_spawn();
    credit_id.disable_spawn();
#endif

    credit_out.clk(clk);
    credit_out.rst(rst);
    credit_out.route(credit_route);
    credit_out.id(credit_id);
    credit_out.enq(credit_enq);
    credit_out.deq(credit);

    SC_METHOD(AssignMsg);
    sensitive << enq._DATNAME_ << rst;

    SC_METHOD(AssignVal);
    sensitive << enq._VLDNAME_;

    SC_METHOD(AssignRdy);
    sensitive << deq._RDYNAME_;

    SC_METHOD(TieToHigh);
    sensitive << clk.pos();

    SC_METHOD(AssignNextCredits);
    sensitive << deq._VLDNAME_ << deq._RDYNAME_ << credit_enq._VLDNAME_ << credit_enq._RDYNAME_
              << credits;

    SC_METHOD(AssignCreditMsg);
    sensitive << deq._VLDNAME_ << deq._RDYNAME_ << credit_enq._VLDNAME_ << credit_enq._RDYNAME_
              << credits;

    SC_METHOD(AssignCreditVal);
    sensitive << deq._VLDNAME_ << deq._RDYNAME_ << credits;

    SC_THREAD(UpdateCredit);
    sensitive << clk.pos();
    NVHLS_NEG_RESET_SIGNAL_IS(rst);
  }

  void AssignMsg() {
    if (!rst.read()) {
      deq._DATNAME_.write(0);
    } else if (enq._VLDNAME_.read()) {
      sc_lv<Wrapped<Packet_t>::width> pbits = enq._DATNAME_.read();
      Marshaller<Wrapped<Packet_t>::width> pmarshaller(pbits);
      Packet_t packet;
      packet.Marshall(pmarshaller);
      MsgBits mbits(packet.data);
      deq._DATNAME_.write(mbits);
    }
  }

  void AssignVal() { deq._VLDNAME_.write(enq._VLDNAME_.read()); }
  void AssignRdy() { enq._RDYNAME_.write(deq._RDYNAME_.read()); }

  void TieToHigh() { init_credits._RDYNAME_.write(1); }

  void AssignNextCredits() {
    bool do_deq = deq._VLDNAME_.read() && deq._RDYNAME_.read();
    bool do_credit = credit_enq._VLDNAME_.read() && credit_enq._RDYNAME_.read();
    if (do_credit && !do_deq && (credits.read() != 0)) {
      credits_next.write(0);
    } else if (do_credit && do_deq) {
      credits_next.write(0);
    } else if (!do_credit && do_deq) {
      credits_next.write(credits.read().to_uint() + 1);
    } else {
      credits_next.write(credits.read());
    }
  }

  void AssignCreditMsg() {
    bool do_deq = deq._VLDNAME_.read() && deq._RDYNAME_.read();
    bool do_credit = credit_enq._VLDNAME_.read() && credit_enq._RDYNAME_.read();
    if (do_deq && do_credit && (credits.read() == 0)) {
      credit_enq._DATNAME_.write(1);
    } else if (do_deq && do_credit && (credits.read() != 0)) {
      credit_enq._DATNAME_.write(credits.read().to_uint() + 1);
    } else if (!do_deq && do_credit && (credits.read() != 0)) {
      credit_enq._DATNAME_.write(credits.read());
    } else {
      credit_enq._DATNAME_.write(0);
    }
  }

  void AssignCreditVal() {
    bool do_deq = deq._VLDNAME_.read() && deq._RDYNAME_.read();
    if (do_deq || (credits.read() != 0)) {
      credit_enq._VLDNAME_.write(1);
    } else {
      credit_enq._VLDNAME_.write(0);
    }
  }

  void UpdateCredit() {
    credits.write(0);
    wait();
    while (1) {
      if (init_credits._VLDNAME_.read()) {
        credits.write(init_credits._DATNAME_.read());
      } else {
        credits.write(credits_next.read());
      }
      wait();
    }
  }

#ifndef __SYNTHESIS__
 public:
  void line_trace() {
    if (rst.read()) {
      unsigned int pwidth = (Packet_t::width / 4);
      // Enqueue port
      if (enq._VLDNAME_.read() && enq._RDYNAME_.read()) {
        CDCOUT(std::hex << std::setw(pwidth) << enq._DATNAME_.read(), kDebugLevel);
      } else {
        CDCOUT(std::setw(pwidth + 1) << " ", kDebugLevel);
      }
      CDCOUT(" | ", kDebugLevel);

      // Dequeue port
      unsigned int mwidth = (Message().length() / 4);
      if (deq._VLDNAME_.read() && deq._RDYNAME_.read()) {
        CDCOUT(std::hex << std::setw(mwidth) << deq._DATNAME_.read(), kDebugLevel);
      } else {
        CDCOUT(std::setw(mwidth + 1) << " ", kDebugLevel);
      }
      CDCOUT(" | ", kDebugLevel);

      // Pending credits
      CDCOUT(std::hex << " ( " << credits.read() << " ) ", kDebugLevel);

      // Credit
      unsigned int cwidth = (CreditPacket_t::width / 4);
      if (credit._VLDNAME_.read() && credit._RDYNAME_.read()) {
        CDCOUT(std::hex << std::setw(cwidth) << credit._DATNAME_.read(), kDebugLevel);
      } else {
        CDCOUT(std::setw(cwidth + 1) << " ", kDebugLevel);
      }
      CDCOUT(" | ", kDebugLevel);
    }
  }
#endif
};

//------------------------------------------------------------------------
// OutNetworkCredit
//------------------------------------------------------------------------
// NOTE: Currently, the PacketIdWidth, DestWidthPerHop, MaxHops parameters
// are assumed to be the same for a packet and a credit_packet

template <typename Message, unsigned int DestWidthPerHop, unsigned int MaxHops,
          unsigned int PacketIdWidth, unsigned int CreditWidth>
class OutNetworkCredit : public sc_module {
  SC_HAS_PROCESS(OutNetworkCredit);

 public:
  static const int kDebugLevel = 3;
  typedef Wrapped<Message> WMessage;
  static const unsigned int width = WMessage::width;
  typedef sc_lv<WMessage::width> MsgBits;
  typedef Packet<WMessage::width, DestWidthPerHop, MaxHops, PacketIdWidth>
      Packet_t;
  typedef Packet<CreditWidth, DestWidthPerHop, MaxHops, PacketIdWidth>
      CreditPacket_t;
  typedef sc_lv<CreditWidth> Credit_t;

  // Interface
  sc_in_clk clk;
  sc_in<bool> rst;
  In<Message> enq;
  Out<Packet_t> deq;
  In<CreditPacket_t> credit;
  In<sc_lv<DestWidthPerHop * MaxHops> > route;
  In<sc_lv<PacketIdWidth> > id;

  // Sub Module
  InNetwork<Credit_t, DestWidthPerHop, MaxHops, PacketIdWidth> credit_in;

  // Wire
  Combinational<Credit_t> credit_deq;

  // Internal State
  sc_signal<sc_lv<DestWidthPerHop * MaxHops> > route_state;
  sc_signal<sc_lv<PacketIdWidth> > id_state;
  sc_signal<Credit_t> credits;
  sc_signal<Credit_t> credits_next;

  OutNetworkCredit()
      : sc_module(sc_module_name(sc_gen_unique_name("out_nw_credit"))),
        clk("clk"),
        rst("rst"),
        credit_in("credit_in") {
    Init();
  }

  OutNetworkCredit(sc_module_name name)
      : sc_module(name), clk("clk"), rst("rst"), credit_in("credit_in") {
    Init();
  }

 protected:
  void Init() {
#ifdef CONNECTIONS_SIM_ONLY
    enq.disable_spawn();
    deq.disable_spawn();
    credit.disable_spawn();
    route.disable_spawn();
    id.disable_spawn();
#endif
    
    credit_in.clk(clk);
    credit_in.rst(rst);
    credit_in.enq(credit);
    credit_in.deq(credit_deq);

    SC_METHOD(AssignMsg);
    sensitive << enq._DATNAME_ << id_state << route_state << rst;

    SC_METHOD(AssignVal);
    sensitive << enq._VLDNAME_ << credits << credit_deq._VLDNAME_ << credit_deq._RDYNAME_;

    SC_METHOD(AssignRdy);
    sensitive << deq._RDYNAME_;

    SC_METHOD(TieToHigh);
    sensitive << clk.pos();

    SC_METHOD(AssignNextCredits);
    sensitive << deq._VLDNAME_ << deq._RDYNAME_ << credit_deq._VLDNAME_ << credit_deq._RDYNAME_
              << credits;

    SC_METHOD(AssignCreditRdy);
    sensitive << deq._VLDNAME_ << deq._RDYNAME_ << credits;

    SC_THREAD(UpdateCredit);
    sensitive << clk.pos();
    NVHLS_NEG_RESET_SIGNAL_IS(rst);

    SC_THREAD(SetState);
    sensitive << clk.pos();
    NVHLS_NEG_RESET_SIGNAL_IS(rst);
  }

  void AssignMsg() {
    if (!rst.read()) {
      deq._DATNAME_.write(0);
    } else if (enq._VLDNAME_.read()) {
      Packet_t packet;
      vector_to_type(id_state.read(), false, &packet.packet_id);
      vector_to_type(route_state.read(), false, &packet.dest);
      vector_to_type(enq._DATNAME_.read(), false, &packet.data);
      Marshaller<Wrapped<Packet_t>::width> pmarshaller;
      packet.Marshall(pmarshaller);
      deq._DATNAME_.write(pmarshaller.GetResult());
    }
  }

  void AssignVal() {
    bool do_credit = credit_deq._VLDNAME_.read() && credit_deq._RDYNAME_.read();
    if (enq._VLDNAME_.read() && (credits.read() != 0)) {
      deq._VLDNAME_.write(1);
    } else if (enq._VLDNAME_.read() && (credits.read() == 0) && do_credit) {
      deq._VLDNAME_.write(1);
    } else {
      deq._VLDNAME_.write(0);
    }
  }

  void AssignRdy() { enq._RDYNAME_.write(deq._RDYNAME_.read()); }

  void TieToHigh() {
    route._RDYNAME_.write(1);
    id._RDYNAME_.write(1);
  }

  void AssignNextCredits() {
    bool do_deq = deq._VLDNAME_.read() && deq._RDYNAME_.read();
    bool do_credit = credit_deq._VLDNAME_.read() && credit_deq._RDYNAME_.read();
    if (!do_credit && do_deq) {
      credits_next.write(credits.read().to_uint() - 1);
    } else if (do_credit && !do_deq) {
      credits_next.write(credits.read().to_uint() +
                         credit._DATNAME_.read().to_uint());
    } else if (do_credit && do_deq) {
      credits_next.write(credits.read().to_uint() +
                         credit._DATNAME_.read().to_uint() - 1);
    } else {
      credits_next.write(credits.read());
    }
  }

  void AssignCreditRdy() { credit_deq._RDYNAME_.write(1); }

  void UpdateCredit() {
    credits.write(0);
    wait();
    while (1) {
      credits.write(credits_next.read());
      wait();
    }
  }

  void SetState() {
    id_state.write(0);
    route_state.write(0);
    wait();
    while (1) {
      if (route._VLDNAME_.read()) {
        route_state.write(route._DATNAME_.read());
      }

      if (id._VLDNAME_.read()) {
        id_state.write(id._DATNAME_.read());
      }
      wait();
    }
  }

#ifndef __SYNTHESIS__
 public:
  void line_trace() {
    if (rst.read()) {
      unsigned int mwidth = (Message().length() / 4);
      // Enqueue port
      if (enq._VLDNAME_.read() && enq._RDYNAME_.read()) {
        CDCOUT(std::hex << std::setw(mwidth) << enq._DATNAME_.read(), kDebugLevel);
      } else {
        CDCOUT(std::setw(mwidth + 1) << " ", kDebugLevel);
      }
      CDCOUT(" | ", kDebugLevel);

      // Dequeue port
      unsigned int pwidth = (Packet_t::width / 4);
      if (deq._VLDNAME_.read() && deq._RDYNAME_.read()) {
        CDCOUT(std::hex << std::setw(pwidth) << deq._DATNAME_.read(), kDebugLevel);
      } else {
        CDCOUT(std::setw(pwidth + 1) << " ", kDebugLevel);
      }
      CDCOUT(" | ", kDebugLevel);

      // Pending credits
      CDCOUT(std::hex << " ( " << credits.read() << " ) ", kDebugLevel);

      // Credit
      unsigned int cwidth = (CreditPacket_t::width / 4);
      if (credit._VLDNAME_.read() && credit._RDYNAME_.read()) {
        CDCOUT(std::hex << std::setw(cwidth) << credit._DATNAME_.read(), kDebugLevel);
      } else {
        CDCOUT(std::setw(cwidth + 1) << " ", kDebugLevel);
      }
      CDCOUT(" | ", kDebugLevel);
    }
  }
#endif
};

}  // namespace Connections

#endif  // NVHLS_CONNECTIONS_NETWORK_H_
