defmodule Extractor do
  @moduledoc """
  File meta data extraction
  """

  @on_load { :init, 0 }

  app = Mix.Project.config[:app]

  def test do
    {:ok, file_data} = File.read("/home/solanav/krptkn/final_form.png")
    Extractor.extract(file_data)
    :msacc.start(2_000)
    :msacc.print()
    :msacc.stop
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
