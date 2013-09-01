#include <cassert>
#include <zinc_plugin>
#include <cstdlib>


using namespace std;
using namespace pnp;

namespace RussianPlugin
{
  const char* c_strings_best_strings[] = {
    "да", "нет", "ура", "победа"
  };
  void sayhello(Context* ctx, str_t remainder) {
    ctx->vtable->reply(ctx, "ПРРРРРРРРРРИВЕТ");
  }
  void randomgen(Context* ctx, str_t remainder){
    int i = rand() % 6 + 1 -2;
    i = abs(i % 4); // interesting fact; -5 % 4 = -1
    ctx->vtable->reply(ctx, c_strings_best_strings[i]);
  }
  void russianRulet(Context* ctx, str_t remainder)
  {
    int i = rand() % 2;
    if(i==0)
      ctx->vtable->reply(ctx, "unlucky"); // *((void*)0);
    else 
      ctx->vtable->reply(ctx, "lucky");
  }
  

  void install(PluginHost* host) {
    host->vtable->register_command(host, "helloworld", &sayhello);
    host->vtable->register_command(host, "randomsthinggenerater", &randomgen);
    host->vtable->register_command(host, "russianrulet", &russianRulet);
  
  }

  void die(){
  //attempt to die
    for(int i=0;i<5000;i++) {
      //host->vtable->irc_msg(host, "no.");
      //c_is_best:
      //go c_is_best;
      break;
    }
  }
  static str_t plugin_name = "RussianPlugin";
  static SemanticVersion plugin_version = {
    (unsigned short)-1, (unsigned short)-2, (unsigned short)-3
  };

  static PluginBase plugbase = {
    &install,
    &die, // uninstall
    nullptr, // transfer
    plugin_name,
    plugin_version
  };
};

PluginBase* get_plugin(std::size_t n) {
  assert(n == 0);
  return &RussianPlugin::plugbase;
}

pnp_module_t pnp_module =
{
  CUR_ZINC_VERSION,             // req_zinc_version
  nullptr,                      // init
  nullptr,                      // destroy

  1,                            // num_exported_plugins
  get_plugin                    // get_plugin
};
