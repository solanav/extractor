#include "extractor.h"

int main()
{
  struct EXTRACTOR_PluginList *plugins = EXTRACTOR_plugin_add_defaults (EXTRACTOR_OPTION_DEFAULT_POLICY);
  /* 
  EXTRACTOR_extract (plugins, argv[1], NULL, 0, &EXTRACTOR_meta_data_print, stdout);
  */
  EXTRACTOR_plugin_remove_all (plugins);
  
  return 0;
}