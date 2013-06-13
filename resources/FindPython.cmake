
FIND_PROGRAM(PYTHON_BIN python  HINT ${PYTHON_EXEC_DIR_HINT})


INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PYTHON  DEFAULT_MSG PYTHON_BIN )

IF(PYTHON_FOUND)
    SET(PYTHON_EXEC ${PYTHON_BIN})
ENDIF()

MARK_AS_ADVANCED(PYTHON_EXEC)

function(find_python_module module)
	string(TOUPPER ${module} module_upper)
	if(NOT PY_${module_upper})
		if(ARGC GREATER 1 AND ARGV1 STREQUAL "REQUIRED")
			set(${module}_FIND_REQUIRED TRUE)
		endif()
		execute_process(COMMAND "${PYTHON_EXEC}" "-c" 
			"import re, ${module}; print re.compile('/__init__.py.*').sub('',${module}.__file__)"
			RESULT_VARIABLE _${module}_status 
			OUTPUT_VARIABLE _${module}_location
			ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
		if(NOT _${module}_status)
			set(PY_${module_upper} ${_${module}_location} CACHE PATH 
				"Location of Python module ${module}")
		endif(NOT _${module}_status)
	endif(NOT PY_${module_upper})
	find_package_handle_standard_args(PY_${module} DEFAULT_MSG PY_${module_upper})
endfunction(find_python_module)