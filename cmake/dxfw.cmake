if(NOT DEFINED DXFW_CONFIGURED)
	# Enable ExternalProject CMake module
	include(ExternalProject)

	# Download and install DXFW
	ExternalProject_Add(
	  dxfw
	  GIT_REPOSITORY https://github.com/BartSiwek/DXFW.git
	  GIT_TAG master
	  CMAKE_ARGS -DBUILD_TESTS=OFF -DBUILD_SAMPLES=OFF
	  PREFIX ${CMAKE_CURRENT_BINARY_DIR}/dxfw
	  INSTALL_COMMAND ""
	)

	# Get the binary and source dirs
	ExternalProject_Get_Property(dxfw source_dir binary_dir)

	# Create a libdxfw target to be used as a dependency by test programs
	add_library(libdxfw IMPORTED SHARED GLOBAL)
	add_dependencies(libdxfw dxfw)

	# Set libdxfw properties
	set_target_properties(libdxfw PROPERTIES
	  "IMPORTED_IMPLIB" "${source_dir}/lib/${CMAKE_CFG_INTDIR}/dxfw.lib"
	)
	include_directories("${source_dir}/include")

	set(DXFW_CONFIGURED TRUE)
endif()
