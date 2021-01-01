defmodule Extractor do
  @moduledoc """
  File meta data extraction
  """

  @on_load { :init, 0 }
  @zip_magic <<0x50, 0x4b, 0x03, 0x04>>
  @gz_magic <<0x1f, 0x8b>>
  @tar_magic <<0x75, 0x73, 0x74, 0x61, 0x72>>

  app = Mix.Project.config[:app]

  def decompress(_file_name, @zip_magic  <> rest) do
    # Decompress ZIP file
    :zip.unzip(@zip_magic  <> rest, [:memory])
  end

  def decompress(_file_name, <<header::2056>> <> @tar_magic <> rest) do
    # Decompress tar and tar.gz files
    :erl_tar.extract({:binary, <<header::2056>> <> @tar_magic <> rest}, [:compressed, :memory])
  end

  def decompress(file_name, @gz_magic <> rest) do
    # Try tar.gz and then gz if it fails
    case :erl_tar.extract({:binary, @gz_magic <> rest}, [:compressed, :memory]) do
      {:ok, file_list} -> {:ok, file_list}
      {:error, _} -> {:ok, [{file_name, :zlib.gunzip(@gz_magic <> rest)}]}
    end
  end

  def decompress(_file_name, _buffer) do
    {:error, []}
  end

  def flat_extract_aux(file_name, buffer) do
    case decompress(file_name, buffer) do
      {:ok, file_list} ->
        [{file_name, buffer} | Enum.flat_map(file_list, fn {name, buff} -> flat_extract_aux(name, buff) end)]
      {:error, file_list} ->
        [{file_name, buffer} | file_list]
    end
  end

  def flat_extract(buffer, _options \\ []) do
    flat_extract_aux("unknown", buffer)
    |> Enum.map(fn {_file_name, file} -> extract(file, []) end)
  end

  def init do
    path = :filename.join(:code.priv_dir(unquote(app)), 'extractor')
    :ok = :erlang.load_nif(path, 0)
  end

  @spec extract(doc :: binary()) :: tuple()
  @spec extract(doc :: binary(), options :: Keyword.t) :: tuple()
  def extract(doc, options \\ [])

  def extract(_, _) do
    exit(:nif_library_not_loaded)
  end
end
