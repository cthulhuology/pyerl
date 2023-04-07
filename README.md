pyerl - Call Python from Erlang
===============================


Getting Started
---------------

Get beamer to do the build https://github.com/cthulhuology/beamer

Then use make

	make

It will build the pyerl.so and py.beam files and install them in your ~/.beamer

Then all you need to do to use it:

	py:init(),
	UUID = py:call("uuid","uuid4").

Currently most basic data types are passed between the two languages, but for
objects that have opaque types like the UUID above the repr of the object will
be passed back as a string.

The intent behind this module is generally to do thing like run ML models,
or webhooks, or integrations that will typically return back data in JSON
or related formats, not memory regions like ndarrays and the like.


MIT License

Copyright (c) 2023 David J Goehrig

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
