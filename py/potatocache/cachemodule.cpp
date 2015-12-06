
#include <Python.h>
#include "potatocache.hpp"
#include "shared.hpp"

static PyObject *
create(PyObject *self, PyObject *args)
{
    const char *name;

    if (!PyArg_ParseTuple(args, "s", &name)) {
        return NULL;
    }

    printf("%s\n", name);
    printf("%lu\n", sizeof(potatocache::block));
    printf("%lu\n", sizeof(potatocache::hash_entry));
    printf("%lu\n", sizeof(potatocache::mem_header));

    potatocache::config config;
    potatocache::api cache(name, config);
    
    Py_RETURN_NONE;
}

static PyMethodDef CacheMethods[] = {
    {"create",  create, METH_VARARGS, "Create or connect to cahe."},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef cachemodule = {
   PyModuleDef_HEAD_INIT,
   "cache",   /* name of module */
   NULL,      /* module documentation, may be NULL */
   -1,        /* size of per-interpreter state of the module, or -1 if the module keeps state in global variables. */
   CacheMethods
};

PyMODINIT_FUNC
PyInit_cache(void)
{
    return PyModule_Create(&cachemodule);
}
