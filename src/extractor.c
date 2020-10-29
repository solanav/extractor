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

typedef struct {
  raw_data **arr;
  size_t len;  
} raw_data_array;

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
  /* Get more memory to save the new struct */
  raw_data_array *rdata_array = (raw_data_array *) handle;
  rdata_array->len++;
  rdata_array->arr = realloc(rdata_array->arr, rdata_array->len * sizeof(raw_data));
  rdata_array->arr[rdata_array->len - 1] = calloc(1, sizeof(raw_data));

  /* Rename for easy access */
  raw_data *rdata = rdata_array->arr[rdata_array->len - 1];

  /* Copy all data to our struct */
  rdata->plugin_name = calloc(strlen(plugin_name) + 1, sizeof(char));
  strcpy(rdata->plugin_name, plugin_name);
  
  const char *type_str = EXTRACTOR_metatype_to_string(type);
  rdata->type = calloc(strlen(type_str) + 1, sizeof(char));
  strcpy(rdata->type, type_str);
  
  rdata->format = format;

  rdata->mime_type = calloc(strlen(data_mime_type) + 1, sizeof(char));
  strcpy(rdata->mime_type, data_mime_type);

  rdata->data = calloc(data_len, sizeof(char));
  memcpy(rdata->data, data, data_len);
  rdata->data_len = data_len;

  return 0;
}

static ERL_NIF_TERM
extract(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
  int i;
  ErlNifBinary file_bin;
  struct EXTRACTOR_PluginList *plugins;

  /* Extract the plugins */
  plugins = EXTRACTOR_plugin_add_defaults(EXTRACTOR_OPTION_DEFAULT_POLICY);

  /* Read file binary */
  if (enif_inspect_binary(env, argv[0], &file_bin) == 0) {
    return enif_make_badarg(env);
  }

  /* Extract the data */
  raw_data_array *data = calloc(1, sizeof(raw_data_array));
  data->arr = NULL;
  data->len = 0;
  EXTRACTOR_extract(plugins, NULL, file_bin.data, file_bin.size, &json_output, (void *) data);

  /* Remove the plugins */
  EXTRACTOR_plugin_remove_all(plugins);

  /* Create list of data */\
  ERL_NIF_TERM *list = calloc(data->len, sizeof(ERL_NIF_TERM));
  for (i = 0; i < data->len; i++)
  {
    list[i] = enif_make_tuple5(
      env,
      enif_make_string(env, (data->arr)[i]->plugin_name, ERL_NIF_LATIN1),
      enif_make_string(env, (data->arr)[i]->type, ERL_NIF_LATIN1),
      enif_make_int(env, (data->arr)[i]->format),
      enif_make_string(env, (data->arr)[i]->mime_type, ERL_NIF_LATIN1),
      enif_make_string(env, (data->arr)[i]->data, ERL_NIF_LATIN1)
    );
  }

  return enif_make_list_from_array(env, list, data->len);
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
