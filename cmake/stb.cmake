if(NOT DEFINED STB_CONFIGURED)
	# Enable ExternalProject CMake module
	include(ExternalProject)

	# Download and install stb
	ExternalProject_Add(
	  stb
	  GIT_REPOSITORY https://github.com/nothings/stb.git
	  GIT_TAG master
	  PREFIX ${CMAKE_CURRENT_BINARY_DIR}/stb
	  BUILD_COMMAND ""
	  INSTALL_COMMAND ""
	  CONFIGURE_COMMAND ""
	)

	# Get the binary and source dirs
	ExternalProject_Get_Property(stb source_dir binary_dir)

	# Create a libstb target to be used as a dependency by test programs
	add_library(libstb IMPORTED INTERFACE GLOBAL)
	add_dependencies(libstb stb)

	# Set libstb properties
	include_directories("${source_dir}")

	set(STB_CONFIGURED TRUE)
endif()
