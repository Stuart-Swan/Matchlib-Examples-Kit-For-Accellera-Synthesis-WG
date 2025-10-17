

// new_connections.h back annotation file interface
// Stuart Swan
// Siemens EDA
// 17 Oct 2025

#pragma once

#include <connections/new_connections.h>

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/istreamwrapper.h>

namespace Connections {

struct annotate {

  struct chan_info {
    std::string chan_path;
    std::string src;
    std::string dst;
    int latency{0};
    int capacity{0};
  };

#ifdef CONNECTIONS_SIM_ONLY
  void output_file_helper(sc_object *obj, std::vector<chan_info>& info) {
    if ( Connections::back_annotation_if *p = dynamic_cast<Connections::back_annotation_if*>(obj) ) {
      chan_info c;
      p->get_path_names(c.chan_path, c.src, c.dst);
      p->get_latency_capacity(c.latency, c.capacity);
      info.push_back(c);
    }
    std::vector<sc_object *> children = obj->get_child_objects();
    for ( unsigned i = 0; i < children.size(); i++ ) {
      if ( children[i] ) {
        output_file_helper(children[i], info);
      }
    }
  }

  std::string out_fname;

  void output_run(sc_object* obj, std::string fname) {
    wait(50, SC_PS);  // Let start_of_sim occur so that In/Out path names get propagated
    std::vector<chan_info> info;
    output_file_helper(obj, info);

    ofstream os;
    os.open(fname);
    os << "{\n";
    os << "   \"channels\": {\n";
    for (unsigned i=0; i < info.size(); ++i) {
      os << "      \"" << info[i].chan_path << "\": {\n";
      os << "         \"latency\": " << info[i].latency << ",\n";
      os << "         \"capacity\": " << info[i].capacity << ",\n";
      os << "         \"src_name\": \"" << info[i].src << "\",\n";
      os << "         \"dest_name\": \"" << info[i].dst << "\"\n";
      if (i < info.size() - 1)
        os << "      },\n";
      else
        os << "      }\n";
    }
    os << "   }\n";
    os << "}\n";
    os.close();
    out_fname = "";
  }

  void output_file( sc_object &sc_obj, std::string fname ) {
    out_fname = fname;
    sc_spawn(sc_bind(&annotate::output_run, this, &sc_obj, fname), "annotate::output_run");
  }

  void input_file( sc_object &sc_obj, std::string fname , bool chatty=0) {
    if (sc_start_of_simulation_invoked()) {
     SC_REPORT_ERROR("CONNECTIONS-318",
       "Cannot call input_file() for back annotation after simulation is started");
     sc_stop();
     return;
    }

    if (fname == out_fname) {
     SC_REPORT_ERROR("CONNECTIONS-320",
       "Back-annotation input_file() and output_file() cannot be called with same file name ");
     return;
    }

    // Create DOM object.
    rapidjson::Document d;

    std::ifstream ifs(fname.c_str());
    if (ifs.good()) {
      rapidjson::IStreamWrapper isw(ifs);
      d.ParseStream(isw);
      if (d.HasParseError() || !d.IsObject()) {
        d.SetObject();
      }
    } else {
      SC_REPORT_ERROR("CONNECTIONS-319", (std::string("Cannot open input_file <") + fname + "> for back annotation").c_str());
      return;
    }

    // Check if array exists, if not add it.
    if (! d.HasMember("channels")) {
      rapidjson::Value v_channels;
      v_channels.SetObject();
      d.AddMember("channels", v_channels, d.GetAllocator());
    }
    rapidjson::Value &v_channels = d["channels"];

    if (!v_channels.IsObject()) {
      v_channels.SetObject();
    }

    auto get_int_field = [](const rapidjson::Value& obj, const char* key) -> int {
      if (!obj.IsObject() || !obj.HasMember(key))
        return 0;
      const rapidjson::Value& field = obj[key];
      if (field.IsInt())
        return field.GetInt();
      if (field.IsUint())
        return static_cast<int>(field.GetUint());
      if (field.IsInt64())
        return static_cast<int>(field.GetInt64());
      if (field.IsUint64())
        return static_cast<int>(field.GetUint64());
      return 0;
    };

    auto get_string_field = [](const rapidjson::Value& obj, const char* key) -> std::string {
      if (!obj.IsObject() || !obj.HasMember(key))
        return std::string();
      const rapidjson::Value& field = obj[key];
      if (field.IsString())
        return field.GetString();
      return std::string();
    };

    std::vector<chan_info> annotations;

    for (auto itr = v_channels.MemberBegin(); itr != v_channels.MemberEnd(); ++itr) {
      if (!itr->name.IsString() || !itr->value.IsObject())
        continue;

      chan_info info;
      info.chan_path = itr->name.GetString();
      const rapidjson::Value& v_channel = itr->value;

      info.latency = get_int_field(v_channel, "latency");
      info.capacity = get_int_field(v_channel, "capacity");
      info.src = get_string_field(v_channel, "src_name");
      info.dst = get_string_field(v_channel, "dest_name");

      annotations.push_back(info);
    }

    for (unsigned i=0; i < annotations.size(); ++i) {
      chan_info& ci = annotations[i];

      sc_object* obj = sc_find_object(ci.chan_path.c_str());
      if (!obj) {
        std::cout << "Object at path <" << ci.chan_path << "> not found during back-annotation.\n";
        continue;
      }
      
      back_annotation_if *p = dynamic_cast<Connections::back_annotation_if*>(obj);
      if (!p) {
        std::cout << "Object at path <" << ci.chan_path << "> cannot be back-annotated.\n";
        continue;
      }

      if (chatty) {
        std::cout << "Annotating: " << obj->name() << " L C: " 
           << ci.latency << " " << ci.capacity << "\n";
      }
      p->set_latency_capacity(ci.latency, ci.capacity);
    }
  }
#else
  void output_file( sc_object &sc_obj, std::string fname ) {}
  void input_file( sc_object &sc_obj, std::string fname, bool chatty = 0) {}
#endif
};

} // namespace Connections
