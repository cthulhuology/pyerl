
LDFLAGS=$$(python3-config --ldflags) -rdynamic -lpython3.10
INCLUDES=-fPIC -I/usr/local/lib/erlang/erts-13.2/include/ $$(python3-config --cflags)


all: pyerl.so py.beam

py.beam: src/py.erl
	beamer make

pyerl.so: pyerl.c
	gcc -fPIC -shared -o pyerl.so pyerl.c $(LDFLAGS) $(INCLUDES)
	mv pyerl.so ~/.beamer/

clean:
	rm pyerl.so
