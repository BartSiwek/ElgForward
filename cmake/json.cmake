if(NOT DEFINED JSON_CONFIGURED)
	# Enable ExternalProject CMake module
	include(ExternalProject)

	# Download and install json
	ExternalProject_Add(
	  json
	  GIT_REPOSITORY https://github.com/nlohmann/json.git
	  GIT_TAG master
	  PREFIX ${CMAKE_CURRENT_BINARY_DIR}/json
	  INSTALL_COMMAND ""
	  CMAKE_ARGS -DJSON_BuildTests=OFF
	)

	# Get the binary and source dirs
	ExternalProject_Get_Property(json source_dir binary_dir)

	# Create a libjson target to be used as a dependency by test programs
	add_library(libjson IMPORTED INTERFACE GLOBAL)
	add_dependencies(libjson json)

	# Set libjson properties
	include_directories("${source_dir}/include")

	set(JSON_CONFIGURED TRUE)
endif()
