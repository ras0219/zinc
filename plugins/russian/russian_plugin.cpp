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
  void random(Context* ctx, str_t remainder){
    int i = rand() % 6 + 1 -2;
    ctx->vtable->reply(ctx, c_strings_best_strings[i]);
  }
  void russianRulet(void* a)
  {
    int i = rand() % 2;
    if(i==0) *((void*)0);
    else 
      ctx->vtable->reply(ctx, "lucky");
  }
  

  void install(PluginHost* host) {
    host->vtable->register_command(host, "helloworld", &sayhello);
    host->vtable->register_command(host, "random sthing generater", &random);
    host->vtable->register_command(host, "russian rulet", &russianRulet);
  
  }

  void die(PluginHost* host){
  //attempt to die
  for(int i=0;i<5000;i++)
    ctx->vtable->reply(ctx,"no.");
c_is_best:
  go c_is_best;
  }
  static str_t plugin_name = "RussianPlugin";
  static SemanticVersion plugin_version = { -1, -2, -3 };

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
  return &HelloWorldPlugin::plugbase;
}

pnp_module_t pnp_module =
{
  CUR_ZINC_VERSION,             // req_zinc_version
  nullptr,                      // init
  nullptr,                      // destroy

  1,                            // num_exported_plugins
  get_plugin                    // get_plugin
};
