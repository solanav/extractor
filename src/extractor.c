#include <stdio.h>
#include <string.h>

#include "erl_nif.h"
#include "extractor.h"

typedef struct {
  char *plugin_name;
  char *type;
  int format;
  char *mime_type;

  char *data;
  size_t data_len;
} raw_data;

_EXTRACTOR_EXTERN int
json_output (
  void *handle,
  const char *plugin_name,
  enum EXTRACTOR_MetaType type,
  enum EXTRACTOR_MetaFormat format,
  const char *data_mime_type,
  const char *data,
	size_t data_len
) {
  raw_data *json_data = (raw_data *) handle;

  /* Copy all data to our struct */
  json_data->plugin_name = calloc(strlen(plugin_name) + 1, sizeof(char));
  strcpy(json_data->plugin_name, plugin_name);
  
  const char *type_str = EXTRACTOR_metatype_to_string(type);
  json_data->type = calloc(strlen(type_str) + 1, sizeof(char));
  strcpy(json_data->type, type_str);
  
  json_data->format = format;

  json_data->mime_type = calloc(strlen(data_mime_type) + 1, sizeof(char));
  strcpy(json_data->mime_type, data_mime_type);

  json_data->data = calloc(data_len, sizeof(char));
  memcpy(json_data->data, data, data_len);
  json_data->data_len = data_len;

  return 0;
}

static ERL_NIF_TERM
extract(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
  ErlNifBinary file_bin;
  struct EXTRACTOR_PluginList *plugins;

  /* Extract the plugins */
  plugins = EXTRACTOR_plugin_add_defaults(EXTRACTOR_OPTION_DEFAULT_POLICY);

  /* Read file binary */
  if (enif_inspect_binary(env, argv[0], &file_bin) == 0) {
    return enif_make_badarg(env);
  }

  /* Extract the data */
  raw_data *json_data = calloc(1, sizeof(raw_data));
  EXTRACTOR_extract(plugins, NULL, file_bin.data, file_bin.size, &json_output, (void *) json_data);

  /* Remove the plugins */
  EXTRACTOR_plugin_remove_all(plugins);

  /* Create list of data */
  ERL_NIF_TERM ret = enif_make_tuple5(
    env,
    enif_make_string(env, json_data->plugin_name, ERL_NIF_LATIN1),
    enif_make_string(env, json_data->type, ERL_NIF_LATIN1),
    enif_make_int(env, json_data->format),
    enif_make_string(env, json_data->mime_type, ERL_NIF_LATIN1),
    enif_make_string(env, json_data->data, ERL_NIF_LATIN1)
  );

  return ret;
}

static ErlNifFunc funcs[] = {
  { "extract", 2, extract }
};

static int
load(ErlNifEnv* env, void** priv, ERL_NIF_TERM info) {
  return 0;
}

static int
reload(ErlNifEnv* env, void** priv, ERL_NIF_TERM info) {
  return 0;
}

static int
upgrade(ErlNifEnv* env, void** priv, void** old_priv, ERL_NIF_TERM info) {
  return load(env, priv, info);
}

static void
unload(ErlNifEnv* env, void* priv) {
  enif_free(priv);
}

ERL_NIF_INIT(Elixir.Extractor, funcs, &load, &reload, &upgrade, &unload)
