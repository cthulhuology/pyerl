%% py
%%
%% MIT No Attribution  
%% Copyright 2023 David J Goehrig <dave@dloh.org>
%%
%% Permission is hereby granted, free of charge, to any person obtaining a copy 
%% of this software and associated documentation files (the "Software"), to 
%% deal in the Software without restriction, including without limitation the 
%% rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
%% sell copies of the Software, and to permit persons to whom the Software is 
%% furnished to do so.  
%%
%% THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
%% IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
%% FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
%% AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
%% LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
%% FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
%% IN THE SOFTWARE.


-module(py).
-author({ "David J Goehrig", "dave@dloh.org"}).
-copyright(<<"© 2023 David J. Goehrig"/utf8>>).
-export([ call/2, call/3, init/0, stop/0 ]).
-nifs([ call/2, call/3, init/0, stop/0 ]).
-on_load(load/0).

load() ->
	{ ok, [[Path]] } = init:get_argument(home),
	ok = erlang:load_nif(Path ++ "/.beamer/pyerl",0).

call(_,_) ->
	exit(nif_libary_not_loaded).

call(_,_,_) ->
	exit(nif_libary_not_loaded).

init() ->
	exit(nif_libary_not_loaded).

stop() ->
	exit(nif_libary_not_loaded).
