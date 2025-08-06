////////////////////////////////////////////////////////////////////////////////
// Catapult Synthesis
// 
// Copyright (c) 2003-2011 Mentor Graphics Corp.
//       All Rights Reserved
// 
// This document contains information that is proprietary to Mentor Graphics
// Corp. The original recipient of this document may duplicate this  
// document in whole or in part for internal business purposes only, provided  
// that this entire notice appears in all copies. In duplicating any part of  
// this document, the recipient agrees to make every reasonable effort to  
// prevent the unauthorized use and distribution of the proprietary information.
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//..............................................................................
//==============================================================================
// This header file contains transactor models necessary for
// interfacing the Catapult C synthesized Cycle/RTL netlist into the original
// SystemC environment
//______________________________________________________________________________
//..............................................................................
////////////////////////////////////////////////////////////////////////////////

#ifndef MC_SCV_TRANS_H_INC
#define MC_SCV_TRANS_H_INC

#include <systemc.h>
#include <mc_typeconv.h>

#if !defined(MTI_SYSTEMC) && !defined(NCSC)
#include <sysc/kernel/sc_spawn_options.h>
#endif

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//..............................................................................
//==============================================================================
//                           'CCS_SCV'namespace
// All the Transactor Module definitions are provided inside this namespace
//______________________________________________________________________________
//..............................................................................
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
namespace CCS_SCV {

  
  //////////////////////////////////////////////////////////////////////////////
  //                          Clock Transactor Module
  //____________________________________________________________________________
  //////////////////////////////////////////////////////////////////////////////
  SC_MODULE(TXN_MOD_clock) {
    private:
      sc_clock              m_sc_clock;
      sc_signal<sc_logic>   m_sc_logic;
      sc_signal<bool>       m_bool;

    
      void changed_proc() {
        while(true) {
          wait();
          bool val = m_sc_clock.read();
          m_sc_logic.write(val ? sc_dt::Log_1 : sc_dt::Log_0);
          m_bool.write(val);
          clk.write(val);
        }
      }
    
   public:
     sc_out<bool> clk;

     SC_HAS_PROCESS(TXN_MOD_clock);

     TXN_MOD_clock( ::sc_core::sc_module_name   _name
                    , double                    period_v_
                    , sc_time_unit              period_tu_=SC_NS
                    , double                    duty_cycle_=0.5
                  ) : sc_module(_name) 
                    , m_sc_clock(basename(), period_v_, period_tu_, duty_cycle_)
                    , m_sc_logic((sc_string(basename())+":logic").c_str())
                    , m_bool((sc_string(basename())+":bool").c_str())
                    , clk((sc_string(basename())+":clk").c_str())
     { 
       SC_THREAD(changed_proc);
         sensitive << m_sc_clock.value_changed_event();
     }

     operator sc_signal<sc_logic> &() { return m_sc_logic; }
     operator sc_signal<bool> &() { return m_bool; }

     sc_clock &get_clock() { return m_sc_clock; }
  }; // clock
  //____________________________________________________________________________
  //////////////////////////////////////////////////////////////////////////////


  //////////////////////////////////////////////////////////////////////////////
  //                          Reset Transactor Module
  //____________________________________________________________________________
  //////////////////////////////////////////////////////////////////////////////
  SC_MODULE(TXN_MOD_reset) {
    private:
      sc_signal<sc_logic>   m_rst;
      unsigned int          m_rst_cyc_cnt;
      bool                  m_ph_neg;

      void reset_proc() {
        m_rst.write( m_ph_neg?sc_dt::Log_0:sc_dt::Log_1 );
        rst.write( m_ph_neg?sc_dt::Log_0:sc_dt::Log_1 );
        while (m_rst_cyc_cnt > 0) {
          wait();
          --m_rst_cyc_cnt;
        }
        m_rst.write( m_ph_neg?sc_dt::Log_1:sc_dt::Log_0 );
        rst.write( m_ph_neg?sc_dt::Log_1:sc_dt::Log_0 );
      }

    public:
      sc_in<bool> clk;
      sc_out<sc_logic > rst;

      SC_HAS_PROCESS(TXN_MOD_reset);

      TXN_MOD_reset( ::sc_core::sc_module_name _name
                     , unsigned int rst_cyc_cnt
                     , bool ph_neg=false
                     , bool clk_neg=false
                   ) : sc_module(_name) 
                     , m_rst((sc_string(basename())+":logic").c_str())
                     , m_rst_cyc_cnt(rst_cyc_cnt)
                     , m_ph_neg(ph_neg)
                     , clk((sc_string(basename())+":clk").c_str())
                     , rst((sc_string(basename())+":rst").c_str())
      { 
        sc_spawn_options so;
        //so.dont_initialize();
        // make it sensitive to the opposite clock edge
        so.set_sensitivity(clk_neg?&clk.pos():&clk.neg());
        sc_spawn(sc_bind(&CCS_SCV::TXN_MOD_reset::reset_proc, this), _name, &so);
      }

      void set_ecnt(unsigned int _cnt=1)            { m_rst_cyc_cnt = _cnt; }
      
      operator sc_signal<sc_logic>  &()             { return m_rst; }
      sc_signal<sc_logic>           &get()          { return m_rst; }
      sc_signal<sc_logic>           &get_reset()    { return m_rst; }

  };
  //____________________________________________________________________________
  //////////////////////////////////////////////////////////////////////////////


  //////////////////////////////////////////////////////////////////////////////
  //                        Input FIFO Transactor Module
  //____________________________________________________________________________
  //////////////////////////////////////////////////////////////////////////////
  template <class DATA_T, unsigned int DATA_W, unsigned int SIZE_W, class LVT>
  SC_MODULE(TXN_MOD_fifo_in) {
    public:
      typedef DATA_T data_T;
      enum { data_W = DATA_W 
           , size_W = SIZE_W 
           };

      sc_fifo_in<data_T>        fifo_data;  //! the input fifo
    
      sc_in<bool>               clk;
      //sc_in<sc_logic> rst;

      // 'lz' signal out of corresponding interface component in RTL model 
      // -- When high data must be valid
      sc_in<sc_logic>           valid;

      // data feeds LV to the 'z' port in RTL MODEL
      sc_out< LVT >    data;

      // 'vz' going into RTL MODEL (wait based components only)
      // -- High as long as fifo is not empty
      sc_out<sc_logic>          ready;

      // fifo_size feeds LV to the 'sizez' port in RTL MODEL (mgc_chan_in/ccs_in_wait_szchan only)
      sc_out<sc_lv<size_W> >    fifo_size;

    private:
      unsigned int              m_fifo_size;
      sc_lv<size_W>             m_fifo_size_lv;
    
    public:
      //typedef fifo_in SC_CURRENT_USER_MODULE;
      SC_HAS_PROCESS(TXN_MOD_fifo_in);

      void load_data_proc() {
        data_T d;
        LVT tval(0);
        bool output_valid = false;
        ready.write(sc_dt::Log_0);
        data.write(tval);
        while(true) {
          wait();
          if (output_valid && (valid.read() == sc_dt::Log_1))
            output_valid = false;

          // Extracting data for next read by RTL
          if (fifo_data.num_available() && !output_valid) {
            output_valid = true;
            d = fifo_data.read();
            type_to_vector(d, data_W, tval);
            data.write(tval);
          }
          ready.write(output_valid?sc_dt::Log_1:sc_dt::Log_0);
        }
      }
  
      void size_proc() {
        m_fifo_size = fifo_data.num_available();
        type_to_vector(m_fifo_size, size_W, m_fifo_size_lv);
        fifo_size.write(m_fifo_size_lv);
      }

      TXN_MOD_fifo_in( ::sc_core::sc_module_name _name
                       , bool clk_neg=false
                     ) : sc_module(_name) 
                       , fifo_data((sc_string(basename())+":fifo_data").c_str())
                       , clk((sc_string(basename())+":clk").c_str())
                       //, rst((sc_string(basename())+":rst").c_str())
                       , valid((sc_string(basename())+":valid").c_str())
                       , data((sc_string(basename())+":data").c_str())
                       , ready((sc_string(basename())+":ready").c_str())
                       , fifo_size((sc_string(basename())+":fifo_size").c_str())
                       , m_fifo_size(0)
      {
        SC_METHOD(size_proc);
          sensitive << fifo_data.data_written();
  
        sc_spawn_options so;
        so.dont_initialize();
        so.set_sensitivity(clk_neg?&clk.neg():&clk.pos());
        sc_spawn( sc_bind(&CCS_SCV::TXN_MOD_fifo_in<DATA_T, DATA_W, SIZE_W, LVT>::load_data_proc,this)
                , _name
                , &so);
      }
  };
  //____________________________________________________________________________
  //////////////////////////////////////////////////////////////////////////////


  //////////////////////////////////////////////////////////////////////////////
  //                        Output FIFO Transactor Module
  //____________________________________________________________________________
  //////////////////////////////////////////////////////////////////////////////
  template <class DATA_T, unsigned int DATA_W, class LVT>
  SC_MODULE(TXN_MOD_fifo_out) {
    public:
      typedef DATA_T data_T;
      enum { data_W = DATA_W };

      sc_fifo_out<data_T> fifo_data;
    
      sc_in<bool> clk;
      //sc_in<sc_logic> rst;
  
      // 'lz' signal out of corresponding interface component in RTL model 
      // -- When high, data is valid
      sc_in<sc_logic> valid;

      // data receives LV from the 'z' port in RTL MODEL
      sc_in< LVT > data;
  
      // 'vz' going into RTL MODEL (wait based components only)
      // -- High as long as fifo can accept data
      sc_out<sc_logic> ready;
      
      //typedef fifo_out SC_CURRENT_USER_MODULE;
      SC_HAS_PROCESS(TXN_MOD_fifo_out);
  
      // Read LV and push into fifo when both valid and ready == '1'
      void load_data_proc() {
        data_T tval;
        LVT d = data.read();
        ready.write(sc_dt::Log_1);
        while (true) {
          wait();
          const sc_dt::sc_logic vl = valid.read();
          const sc_dt::sc_logic rd = ready.read();
          if ( (rd == sc_dt::Log_1) && (vl == sc_dt::Log_1)) {
            d = data.read();
            vector_to_type(d, true, &tval);
            fifo_data.write(tval);
          }
        }
      }
  
      void ready_proc() {
        ready.write(!!fifo_data.num_free()?sc_dt::Log_1:sc_dt::Log_0);
      }

      TXN_MOD_fifo_out( ::sc_core::sc_module_name _name
                        , bool clk_neg=false
                      ) : sc_module(_name) 
                        , fifo_data((sc_string(basename())+":fifo_data").c_str())
                        , clk((sc_string(basename())+":clk").c_str())
                        //, rst((sc_string(basename())+":rst").c_str())
                        , valid((sc_string(basename())+":valid").c_str())
                        , data((sc_string(basename())+":data").c_str())
                        , ready((sc_string(basename())+":ready").c_str())
      {      
        SC_METHOD(ready_proc);
          sensitive << fifo_data.data_read();
          sensitive << (clk_neg?clk.neg():clk.pos());
  
        sc_spawn_options so;
        so.dont_initialize();
        so.set_sensitivity(clk_neg?&clk.neg():&clk.pos());
        sc_spawn( sc_bind(&CCS_SCV::TXN_MOD_fifo_out<DATA_T, DATA_W, LVT >::load_data_proc, this)
                , _name
                , &so);
      }
  };
  //____________________________________________________________________________
  //////////////////////////////////////////////////////////////////////////////

};

#endif
//______________________________________________________________________________
//..............................................................................
////////////////////////////////////////////////////////////////////////////////

