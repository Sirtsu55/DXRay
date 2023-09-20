
# Configure the project
project("DXRay")

# Add Dependencies
add_subdirectory("Deps/DirectX-Headers")
add_subdirectory("Deps/D3D12MemoryAllocator")

# Add the Library
file(GLOB_RECURSE DXRaySOURCES "${PROJECT_SOURCE_DIR}/Source/*.cpp")

add_library(DXRay STATIC ${DXRaySOURCES})



# Precompiled Headers
target_precompile_headers(DXRay PUBLIC "${PROJECT_SOURCE_DIR}/Include/DXRay/DXRay.h")

# Add the Include Directory
target_include_directories(DXRay PUBLIC 
	"${PROJECT_SOURCE_DIR}/Include"
	"${PROJECT_SOURCE_DIR}/Deps/DirectX-Headers/include"
	"${PROJECT_SOURCE_DIR}/Deps/D3D12MemoryAllocator/include"
)
