#include <stdio.h>
#include <string.h>
#include <time.h>

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
text_output (
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
  raw_data *rdata;
  const char *type_str;

  rdata_array->len++;
  rdata_array->arr = realloc(rdata_array->arr, rdata_array->len * sizeof(raw_data));
  rdata_array->arr[rdata_array->len - 1] = calloc(1, sizeof(raw_data));

  /* Rename for easy access */
  rdata = rdata_array->arr[rdata_array->len - 1];

  /* Copy all data to our struct */
  rdata->plugin_name = calloc(strlen(plugin_name) + 1, sizeof(char));
  strcpy(rdata->plugin_name, plugin_name);
  
  type_str = EXTRACTOR_metatype_to_string(type);
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
  int len;
  size_t i;
  ErlNifBinary file_bin;
  raw_data_array *data;
  ERL_NIF_TERM *list;
  struct EXTRACTOR_PluginList *plugins;
  
  clock_t t;
  double time_taken;

  /* Extract the plugins */
  printf("Extracting plugins...");
  t = clock();

  plugins = (struct EXTRACTOR_PluginList *) enif_priv_data(env);

  t = clock() - t;
  time_taken = ((double) t) / CLOCKS_PER_SEC;
  printf("[OK] [%f]\n\r", time_taken);

  /* Read file binary */
  printf("Reading file binary...");
  t = clock();

  if (enif_inspect_binary(env, argv[0], &file_bin) == 0) {
    return enif_make_badarg(env);
  }

  t = clock() - t;
  time_taken = ((double) t) / CLOCKS_PER_SEC;
  printf("[OK] [%f]\n\r", time_taken);

  /* Extract the data */
  printf("Extracting the data...");
  t = clock();

  data = calloc(1, sizeof(raw_data_array));
  data->arr = NULL;
  data->len = 0;
  EXTRACTOR_extract(plugins, NULL, file_bin.data, file_bin.size, &text_output, (void *) data);

  t = clock() - t;
  time_taken = ((double) t) / CLOCKS_PER_SEC;
  printf("[OK] [%f]\n\r", time_taken);

  /* Create list of data */
  printf("Creating list of data...");
  t = clock();

  list = calloc(data->len, sizeof(ERL_NIF_TERM));
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

  t = clock() - t;
  time_taken = ((double) t) / CLOCKS_PER_SEC;
  printf("[OK] [%f]\n\r", time_taken);

  printf("Destroying the data array...");
  t = clock();

  len = data->len;

  /* Destroy the data array 
  for (i = 0; i < data->len; i++)
  {
    free((data->arr)[i]->plugin_name);
    free((data->arr)[i]->type);
    free((data->arr)[i]->mime_type);
    free((data->arr)[i]->data);

    free((data->arr)[i]);
  }
  free(data->arr);
  free(data);*/

  t = clock() - t;
  time_taken = ((double) t) / CLOCKS_PER_SEC;
  printf("[OK] [%f]\n\r\n\r", time_taken);

  return enif_make_list_from_array(env, list, len);
}

static ErlNifFunc funcs[] = {
  { "extract", 2, extract, ERL_NIF_DIRTY_JOB_CPU_BOUND }
};

static int
load(ErlNifEnv* env, void** priv, ERL_NIF_TERM info) {
  *priv = (void *) EXTRACTOR_plugin_add_defaults(EXTRACTOR_OPTION_DEFAULT_POLICY);
  return 0;
}

static int
reload(ErlNifEnv* env, void** priv, ERL_NIF_TERM info) {
  *priv = (void *) EXTRACTOR_plugin_add_defaults(EXTRACTOR_OPTION_DEFAULT_POLICY);
  return 0;
}

static int
upgrade(ErlNifEnv* env, void** priv, void** old_priv, ERL_NIF_TERM info) {
  *priv = (void *) EXTRACTOR_plugin_add_defaults(EXTRACTOR_OPTION_DEFAULT_POLICY);
  return load(env, priv, info);
}

static void
unload(ErlNifEnv* env, void* priv) {
  EXTRACTOR_plugin_remove_all((struct EXTRACTOR_PluginList *) priv);
}

ERL_NIF_INIT(Elixir.Extractor, funcs, &load, &reload, &upgrade, &unload)
