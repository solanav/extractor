# Extractor

A simple Elixir metadata extractor for many types of files.

Implemented entirely as a [NIF][2] binding to the [libextractor][3] library.

Currently only working on linux. Should be easy to adapt to windows tho.

## Usage

```iex
iex> Extractor.extract(File.read!("a.jpg"))
{'gstreamer', 'pixel aspect ratio', 1, 'text/plain', '1/1'}
```

[1]: http://www.erlang.org/doc/tutorial/nif.html
[2]: https://www.gnu.org/software/libextractor/
