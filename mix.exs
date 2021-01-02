defmodule Mix.Tasks.Compile.Extractor do
  def run(_) do
    if match? {:win32, _}, :os.type do
      {result, _error_code} = System.cmd("nmake", ["/F", "Makefile.win", "priv\\extractor.dll"], stderr_to_stdout: true)
      IO.binwrite result
    else
      {result, _error_code} = System.cmd("make", ["priv/extractor.so"], stderr_to_stdout: true)
      IO.binwrite result
    end
    :ok
  end
end

defmodule Extractor.Mixfile do
  use Mix.Project

  @version "0.1.0"

  def project do
    [app: :extractor,
     version: @version,
     elixir: ">= 0.14.3 and < 2.0.0",
     compilers: [:extractor, :elixir, :app],
     deps: deps()]
  end

  def application do
    []
  end

  defp deps do
    [
      {:ex_doc, "~> 0.21", only: :dev, runtime: false},
    ]
  end
end
