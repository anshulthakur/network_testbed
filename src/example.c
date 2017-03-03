#include <Python.h>

/* Core C function whose functionality is needed in python */
int fact(int n)
{
  if(n<=1) return 1;
  else return n*fact(n-1);
}


/* Python Wrapper method for factoria*/
PyObject *wrap_fact(PyObject *self, PyObject *args)
{
  int n, result;

  if(!PyArg_ParseTuple(args, "i:fact", &n))
    return NULL;

  result = fact(n);
  return Py_BuildValue("i", result);
}

static PyMethodDef exampleMethods[] = {
		{ "fact", wrap_fact, 1 },
		{ NULL, NULL }
	};

void initexample()
{
  PyObject *m;
  m = Py_InitModule("example", exampleMethods);
}
