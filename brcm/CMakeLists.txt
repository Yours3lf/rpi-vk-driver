file(GLOB brcmSrc
	"cle/*.h"
	"cle/*.c"
	"clif/*.h"
	"clif/*.c"
	"common/*.h"
	"common/*.c"
	"qpu/*.h"
	"qpu/*.c"
)

execute_process(COMMAND "python" "${CMAKE_CURRENT_SOURCE_DIR}/cle/gen_pack_header.py" "${CMAKE_CURRENT_SOURCE_DIR}/cle/v3d_packet_v21.xml" "21" OUTPUT_VARIABLE V3D_PACKET_V21_PACK ERROR_VARIABLE CMD_ERROR)
if(CMD_ERROR STREQUAL "")
	file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/cle/v3d_packet_v21_pack.h" "${V3D_PACKET_V21_PACK}" )
else(CMD_ERROR STREQUAL "")
	message(FATAL_ERROR ${CMD_ERROR})
endif(CMD_ERROR STREQUAL "")
execute_process(COMMAND "python" "${CMAKE_CURRENT_SOURCE_DIR}/cle/gen_zipped_file.py" "${CMAKE_CURRENT_SOURCE_DIR}/cle/v3d_packet_v21.xml" OUTPUT_VARIABLE V3D_XML ERROR_VARIABLE CMD_ERROR2)
if(CMD_ERROR2 STREQUAL "")
	file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/cle/v3d_xml.h" "${V3D_XML}" )
else(CMD_ERROR2 STREQUAL "")
	message(FATAL_ERROR ${CMD_ERROR2})
endif(CMD_ERROR2 STREQUAL "")

add_library(brcm OBJECT ${brcmSrc})
target_compile_options(brcm PRIVATE -Wall -std=c99
	-march=${RPI_ARCH} -fPIC
	)
#target_link_libraries(brcm expat z)
target_compile_definitions(brcm PRIVATE V3D_VERSION=21)
