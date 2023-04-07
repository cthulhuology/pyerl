// pyerl.c
//
// MIT No Attribution  
// Copyright 2023 David J Goehrig <dave@dloh.org>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy 
// of this software and associated documentation files (the "Software"), to 
// deal in the Software without restriction, including without limitation the 
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
// sell copies of the Software, and to permit persons to whom the Software is 
// furnished to do so.  
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
// IN THE SOFTWARE.
//

#include <Python.h>
#include <erl_nif.h>


PyObject* erlnif_term_to_pyobject(ErlNifEnv* env, ERL_NIF_TERM term) {
	if (enif_is_atom(env, term)) {
		unsigned atom_length;
		enif_get_atom_length(env, term, &atom_length, ERL_NIF_LATIN1);
		char* atom_str = (char*)malloc(sizeof(char) * (atom_length + 1));
		enif_get_atom(env, term, atom_str, atom_length + 1, ERL_NIF_LATIN1);

		if (strcmp(atom_str, "true") == 0) {
			//Py_INCREF(Py_True);
			free(atom_str);
			return Py_True;
		} 
		if (strcmp(atom_str, "false") == 0) {
			//Py_INCREF(Py_False);
			free(atom_str);
			return Py_False;
		} 
		PyObject* py_atom = PyUnicode_FromString(atom_str);
		free(atom_str);
		return py_atom;
	}
 	if (enif_is_number(env, term)) {
		if (enif_get_int(env, term, NULL)) {
		    int int_value;
		    enif_get_int(env, term, &int_value);
		    return PyLong_FromLong(int_value);
		} 
		if (enif_get_long(env, term, NULL)) {
		    long long_value;
		    enif_get_long(env, term, &long_value);
		    return PyLong_FromLong(long_value);
		} 
		double double_value;
		enif_get_double(env, term, &double_value);
		return PyFloat_FromDouble(double_value);
	}

    	if (enif_is_list(env, term)) {
		ERL_NIF_TERM head, tail;
		int is_proplist = 1;

		for (ERL_NIF_TERM iter = term; is_proplist && enif_get_list_cell(env, iter, &head, &tail); iter = tail) {
			if (!enif_is_tuple(env, head)) {
				is_proplist = 0;
				break;
			}

			int tuple_size;
			const ERL_NIF_TERM* tuple;
			if (! enif_get_tuple(env,head,&tuple_size,&tuple)) {
				is_proplist = 0;
				break;
			}
			if (tuple_size != 2) {
				is_proplist = 0;
				break;
			}

			if (!enif_is_binary(env, tuple[0])) {
				is_proplist = 0;
				break;
			}
		}

		if (is_proplist) {
			PyObject* py_dict = PyDict_New();
			const ERL_NIF_TERM* tuple;
			int arity;

			for (ERL_NIF_TERM iter = term; enif_get_list_cell(env, iter, &head, &tail); iter = tail) {
				
				enif_get_tuple(env, iter, &arity, &tuple);
				
				PyObject* py_key = erlnif_term_to_pyobject(env, tuple[0]);
				PyObject* py_value = erlnif_term_to_pyobject(env, tuple[1]);

				PyDict_SetItem(py_dict, py_key, py_value);
				Py_DECREF(py_key);
				Py_DECREF(py_value);
			}

			return py_dict;
		} else {
			PyObject* py_list = PyList_New(0);
			ERL_NIF_TERM iter;
			for (iter = term; enif_get_list_cell(env, iter, &head, &tail); iter = tail) {
				PyObject* py_elem = erlnif_term_to_pyobject(env, head);
				PyList_Append(py_list, py_elem);
				Py_DECREF(py_elem);
			}

			return py_list;
		}
	}
	if (enif_is_binary(env, term)) {
		ErlNifBinary binary;
		enif_inspect_binary(env, term, &binary);
		PyObject* py_string = PyUnicode_FromStringAndSize((char*)binary.data, binary.size);
		return py_string;
	} 
	 
	Py_INCREF(Py_None);
	return Py_None;
}

// converts
// none -> []
// True -> true
// False -> false
// "string" -> "string"
// 123 -> 123
// 12.3 -> 12.3
// [1,2,3] -> [1,2,3]
// { "foo": "bar" } => [ { "foo", "bar" } ]
// ( "foo",3, 12.3 ) -> { "foo", 3, 12.3 }


ERL_NIF_TERM pyobject_to_erlnif_term(ErlNifEnv* env, PyObject* obj) {
	if (Py_IsNone(obj)) {
		return enif_make_list(env,0);	
	}
	if (PyBool_Check(obj)) {
		return PyObject_IsTrue(obj) ?
			enif_make_atom(env, "true"):
			enif_make_atom(env, "false");
	}
	if (PyUnicode_Check(obj)) {
		const char* str_value = PyUnicode_AsUTF8(obj);
		return enif_make_string(env, str_value, ERL_NIF_LATIN1);
	}
	if (PyLong_Check(obj)) {
		long int_value = PyLong_AsLong(obj);
		return enif_make_int(env, int_value);
	}
	if (PyFloat_Check(obj)) {
		double double_value = PyFloat_AsDouble(obj);
		return enif_make_double(env, double_value);
	}
	if (PyList_Check(obj)) {
		Py_ssize_t list_size = PyList_Size(obj);
		ERL_NIF_TERM list_term = enif_make_list(env, 0);
		for (Py_ssize_t i = 0; i < list_size; i++) {
			PyObject* item = PyList_GetItem(obj, i);
			ERL_NIF_TERM item_term = pyobject_to_erlnif_term(env, item);
			list_term = enif_make_list_cell(env, item_term, list_term);
		}
		return list_term;
	}
	if (PyDict_Check(obj)) {
		PyObject* pKey;
		PyObject* pValue;
		Py_ssize_t pos = 0;

		ERL_NIF_TERM proplist = enif_make_list(env, 0);

		while (PyDict_Next(obj, &pos, &pKey, &pValue)) {
			PyObject* key_str = PyObject_Str(pKey);
			if (key_str == NULL ) {
			    Py_XDECREF(key_str);
			    return enif_make_badarg(env);
			}

			ERL_NIF_TERM eKey = enif_make_string(env, PyUnicode_AsUTF8(key_str), ERL_NIF_LATIN1);
			ERL_NIF_TERM eValue = pyobject_to_erlnif_term(env, pValue);

			ERL_NIF_TERM eTuple = enif_make_tuple2(env, eKey, eValue);
			proplist = enif_make_list_cell(env, eTuple, proplist);

			Py_DECREF(key_str);
		}
		return proplist;
	}
	if (PyTuple_Check(obj)) {
		Py_ssize_t tuple_size = PyTuple_Size(obj);
		ERL_NIF_TERM* terms = (ERL_NIF_TERM*)malloc(sizeof(ERL_NIF_TERM) * tuple_size);
		for (Py_ssize_t i = 0; i < tuple_size; i++) {
			PyObject* item = PyTuple_GetItem(obj, i);
			ERL_NIF_TERM item_term = pyobject_to_erlnif_term(env, item);
			terms[i] = item_term;
		}
		ERL_NIF_TERM erl_tuple = enif_make_tuple_from_array(env, terms, tuple_size);
		free(terms);
		return erl_tuple;
	}
	PyObject* rep = PyObject_Repr(obj);
	PyObject* str = PyUnicode_AsEncodedString(rep, "utf-8","~E~");
	const char* bytes = PyBytes_AS_STRING(str);
	ERL_NIF_TERM retstr = enif_make_string(env,bytes,ERL_NIF_LATIN1);
	Py_XDECREF(rep);
	Py_XDECREF(str);
	return retstr;
}

static ERL_NIF_TERM python_init_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
	Py_SetProgramName(Py_DecodeLocale("python",NULL));
	Py_Initialize();
	PySys_SetPath(Py_GetPath());
	if (! Py_IsInitialized()) return enif_make_atom(env,"error");
	return enif_make_atom(env,"ok");
}

char* pyerrstr() {
	PyObject *ptype, *pvalue, *ptraceback;
	PyObject *py_str;
	char* err_msg;

	PyErr_Fetch(&ptype, &pvalue, &ptraceback);
	PyErr_NormalizeException(&ptype, &pvalue, &ptraceback);

	py_str = PyObject_Str(pvalue);
	err_msg = PyBytes_AsString(PyUnicode_AsEncodedString(py_str, "utf-8", "Error"));

	Py_XDECREF(ptype);
	Py_XDECREF(pvalue);
	Py_XDECREF(ptraceback);
	Py_XDECREF(py_str);

	return err_msg;
}


// V = call(M,F,A)
static ERL_NIF_TERM python_call_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
	PyObject *pModule;
	PyObject *pFunction;
	PyObject *pArgs;
	PyObject *pValue;
	ERL_NIF_TERM retval;
	char ModuleName[256];
	char FunctionName[256];
	int i;

	if (! enif_get_string(env,argv[0], ModuleName, 255, ERL_NIF_LATIN1))
		return enif_make_tuple2(env, enif_make_atom(env,"error"), enif_make_string(env,pyerrstr(),ERL_NIF_LATIN1));

	if (! enif_get_string(env,argv[1], FunctionName, 255, ERL_NIF_LATIN1))
		return enif_make_tuple2(env, enif_make_atom(env,"error"), enif_make_string(env,pyerrstr(),ERL_NIF_LATIN1));
	
	pModule = PyImport_ImportModule(ModuleName);

	if (!pModule) 
		return enif_make_tuple2(env, enif_make_atom(env,"error"), enif_make_string(env,pyerrstr(), ERL_NIF_LATIN1));

	pFunction = PyObject_GetAttrString(pModule, FunctionName);

	if (! pFunction || !PyCallable_Check(pFunction))
		return enif_make_tuple2(env, enif_make_atom(env,"error"), enif_make_string(env,pyerrstr(), ERL_NIF_LATIN1));

	pArgs = PyTuple_New(argc -2);
	for (i = 2; i < argc; ++i) {
		pValue = erlnif_term_to_pyobject(env,argv[i]);
		PyTuple_SetItem(pArgs,i-2, pValue);
	}

	pValue = PyObject_CallObject(pFunction,pArgs);

	if (!pValue) {
		retval = enif_make_tuple2(env, enif_make_atom(env, "error"), enif_make_string(env,pyerrstr(), ERL_NIF_LATIN1));
		return retval;
	}
	
	retval = pyobject_to_erlnif_term(env,pValue);
	Py_DECREF(pValue);
	return retval;
}

static ERL_NIF_TERM python_stop_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
    if (Py_IsInitialized()) {
        Py_Finalize();
    }
    return enif_make_atom(env, "ok");
}

static ErlNifFunc nif_funcs[] = {
	{ "call",3,python_call_nif, ERL_NIF_DIRTY_JOB_CPU_BOUND },
	{ "call",2,python_call_nif, ERL_NIF_DIRTY_JOB_CPU_BOUND },
	{ "init",0,python_init_nif, ERL_NIF_DIRTY_JOB_CPU_BOUND },
	{ "stop",0,python_stop_nif, ERL_NIF_DIRTY_JOB_CPU_BOUND }
};

ERL_NIF_INIT(py,nif_funcs, NULL, NULL, NULL, NULL)
