
#include "python_bindings_common.h"

#include "export_headers.h"
#include "classad_wrapper.h"
#include "exception_utils.h"

BOOST_PYTHON_MODULE(classad)
{
	export_classad();

	PyExc_ClassAdException = CreateExceptionInModule(
		"classad.ClassAdException", "ClassAdException", PyExc_Exception );

	PyExc_ClassAdEnumError = CreateExceptionInModule(
		"classad.ClassAdEnumError", "ClassAdEnumError",
		PyExc_ClassAdException );
	PyExc_ClassAdEvaluationError = CreateExceptionInModule(
		"classad.ClassAdEvaluationError", "ClassAdEvaluationError",
		PyExc_ClassAdException );
	PyExc_ClassAdInternalError = CreateExceptionInModule(
		"classad.ClassAdInternalError", "ClassAdInternalError",
		PyExc_ClassAdException );
	PyExc_ClassAdParseError = CreateExceptionInModule(
		"classad.ClassAdParseError", "ClassAdParseError",
		PyExc_ClassAdException );
	PyExc_ClassAdValueError = CreateExceptionInModule(
		"classad.ClassAdValueError", "ClassAdValueError",
		PyExc_ClassAdException );
}
