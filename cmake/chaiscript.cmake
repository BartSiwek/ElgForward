if(NOT DEFINED CHAISCRIPT_CONFIGURED)
	# Enable ExternalProject CMake module
	include(ExternalProject)

	# Download and install chaiscript
	ExternalProject_Add(
	  chaiscript
	  GIT_REPOSITORY https://github.com/ChaiScript/ChaiScript.git
	  GIT_TAG master
	  PREFIX ${CMAKE_CURRENT_BINARY_DIR}/chaiscript
	  INSTALL_COMMAND ""
	)

	# Get the binary and source dirs
	ExternalProject_Get_Property(chaiscript source_dir binary_dir)

	# Create a libchaiscript target to be used as a dependency by test programs
	add_library(libchaiscript IMPORTED INTERFACE GLOBAL)
	add_dependencies(libchaiscript chaiscript)

	# Set libchaiscript properties
	include_directories("${source_dir}/include")

	set(CHAISCRIPT_CONFIGURED TRUE)
endif()
