#include <cassert>
#include <zinc_plugin>
#include <cstdlib>
#include <zinc_interactive>

using namespace std;
using namespace zinc;

namespace RussianPlugin
{
  const char* c_strings_best_strings[] = {
    "да", "нет", "ура", "победа"
  };
  void sayhello(OStream::Interface* ctx, str_t remainder) {
    ctx->vtable->send(ctx, "ПРРРРРРРРРРИВЕТ");
  }
  void randomgen(OStream::Interface* ctx, str_t remainder){
    int i = rand() % 6 + 1 -2;
    i = abs(i % 4); // interesting fact; -5 % 4 = -1
    ctx->vtable->send(ctx, c_strings_best_strings[i]);
  }
  void russianRulet(OStream::Interface* ctx, str_t remainder)
  {
    int i = rand() % 2;
    if(i==0)
      ctx->vtable->send(ctx, "unlucky"); // *((void*)0);
    else 
      ctx->vtable->send(ctx, "lucky");
  }
  
  struct InstallInterfaces {
    Interactive::Interface* i;
  };

  void install(Interface** argv) {
    InstallInterfaces* ii = (InstallInterfaces*)(argv);
    auto i = ii->i;
    i->vtable->register_command(i, "helloworld", &sayhello);
    i->vtable->register_command(i, "randomsthinggenerater", &randomgen);
    i->vtable->register_command(i, "russianrulet", &russianRulet);
  
  }

  static str_t plugin_name = "RussianPlugin";
  static SemanticVersion plugin_version = {
    (unsigned short)-1, (unsigned short)-1, (unsigned short)-3
  };

  InterfaceRequest requests[] = {
    Interactive::Request::required
  };

  void die(){
  //attempt to die
    for(int i=0;i<5000;i++) {
      //host->vtable->irc_msg(host, "no.");
      //c_is_best:
      //go c_is_best;
      break;
    }
  }

  static Plugin russianplugin = {
    &install,
    &die, // uninstall
    nullptr, // transfer
    plugin_name,
    plugin_version,

    1,
    requests
  };
};

Plugin* get_plugin(std::size_t n) {
  assert(n == 0);
  return &RussianPlugin::russianplugin;
}

pnp_module_t pnp_module =
{
  CUR_ZINC_VERSION,             // req_zinc_version
  1,                            // num_exported_plugins
  get_plugin                    // get_plugin
};
