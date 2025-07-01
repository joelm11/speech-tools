# Speech Tools

A C++ library implementing different tools for working with speech.

## Module Architecture

Different speech processing functionality spread out into individual modules. Speech processing via the modules is implemented using a pipe/filter structure.

### API

- Each filter module implements a base filter API configured through templated parameters.
- Each filter is constructed with a threadsafe input and output pipe.
- Each filter can be run on its own thread.

**TODO**
Once the modules are completed, provide a top-level API and CLI that combines them.

## Modules

### Noise Reduction

Library implementing different methods for removing noise from speech.
