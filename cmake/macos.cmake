#
# MacOS
#

# CoreFoundation
find_library(_core_lib CoreFoundation)
list(APPEND TURNER_DEP_LIBS ${_core_lib})

# Security
find_library(_security_lib Security)
list(APPEND TURNER_DEP_LIBS ${_security_lib})
