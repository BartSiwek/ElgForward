if(NOT DEFINED TINYOBJLOADER_CONFIGURED)
	# Enable ExternalProject CMake module
	include(ExternalProject)

	# Download and install tinyobjloader
	ExternalProject_Add(
	  tinyobjloader
	  GIT_REPOSITORY https://github.com/syoyo/tinyobjloader.git
	  GIT_TAG master
	  PREFIX ${CMAKE_CURRENT_BINARY_DIR}/tinyobjloader
	  INSTALL_COMMAND ""
	)

	# Get the binary and source dirs
	ExternalProject_Get_Property(tinyobjloader source_dir binary_dir)

	# Create a libtinyobjloader target to be used as a dependency by test programs
	add_library(libtinyobjloader IMPORTED SHARED GLOBAL)
	add_dependencies(libtinyobjloader tinyobjloader)

	# Set libtinyobjloader properties
	set_target_properties(libtinyobjloader PROPERTIES
	  "IMPORTED_IMPLIB" "${binary_dir}/${CMAKE_CFG_INTDIR}/tinyobjloader.lib"
	)
	include_directories("${source_dir}/include")

	set(TINYOBJLOADER_CONFIGURED TRUE)
endif()
