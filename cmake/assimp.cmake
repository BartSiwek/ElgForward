if(NOT DEFINED ASSIMP_CONFIGURED)
	# Enable ExternalProject CMake module
	include(ExternalProject)

	# We need git
	find_package(Git REQUIRED)

	# Download and install assimp
	ExternalProject_Add(
	  assimp
	  GIT_REPOSITORY https://github.com/assimp/assimp.git
	  GIT_TAG master
	  CMAKE_ARGS -DASSIMP_BUILD_ASSIMP_TOOLS=OFF -DASSIMP_BUILD_SAMPLES=OFF -DASSIMP_BUILD_TESTS=OFF -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
	  PATCH_COMMAND "${GIT_EXECUTABLE}" checkout . COMMAND "${GIT_EXECUTABLE}" apply "${CMAKE_CURRENT_LIST_DIR}/assimp.patch"
	  PREFIX ${CMAKE_CURRENT_BINARY_DIR}/assimp
	  INSTALL_COMMAND ""
	)

	# Get the binary and source dirs
	ExternalProject_Get_Property(assimp source_dir binary_dir)

	# Create a libassimp target to be used as a dependency by test programs
	add_library(libassimp IMPORTED SHARED GLOBAL)
	add_dependencies(libassimp assimp)

	# Figure out the lib name (from assimp's CMakeLists.txt)
	if( MSVC )
		if( MSVC70 OR MSVC71 )
			set(MSVC_PREFIX "vc70")
		elseif( MSVC80 )
			set(MSVC_PREFIX "vc80")
		elseif( MSVC90 )
			set(MSVC_PREFIX "vc90")
		elseif( MSVC10 )
			set(MSVC_PREFIX "vc100")
		elseif( MSVC11 )
			set(MSVC_PREFIX "vc110")
		elseif( MSVC12 )
			set(MSVC_PREFIX "vc120")
		elseif( MSVC14 )
			set(MSVC_PREFIX "vc140")
		else()
			set(MSVC_PREFIX "vc150")
		endif()
		set(LIBRARY_SUFFIX "${ASSIMP_LIBRARY_SUFFIX}-${MSVC_PREFIX}-mt" CACHE STRING "the suffix for the assimp windows library")
	endif()

	# Set libassimp properties
	set_target_properties(libassimp PROPERTIES
	  "IMPORTED_IMPLIB" "${binary_dir}/code/${CMAKE_CFG_INTDIR}/assimp${LIBRARY_SUFFIX}.lib"
	  "IMPORTED_LOCATION" "${binary_dir}/code/${CMAKE_CFG_INTDIR}/assimp${LIBRARY_SUFFIX}.dll"
	)
	include_directories("${source_dir}/include")

	set(ASSIMP_CONFIGURED TRUE)
endif()
