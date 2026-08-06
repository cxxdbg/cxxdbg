# Copyright (c) 2026, Alexandr Esilevich
#
# Licensed under the Apache License, Version 2.0.
# See accompanying file LICENSE for license information.
# SPDX-License-Identifier: Apache-2.0
#

option(LLDB_ENABLE_CURSES "Enable ncurses support in LLDB" ON)
option(LLDB_ENABLE_LIBEDIT "Enable libedit support in LLDB" ON)
option(LLVM_ENABLE_TERMINFO "Enable term info support in LLDB" ON)


if("${LLDB_ENABLE_CURSES}")
    find_package(Curses REQUIRED)
    if(NOT "${PANEL_LIBRARIES}")
        find_library(PANEL_LIBRARIES NAMES panel DOC "The curses panel library" QUIET)
        if("${PANEL_LIBRARIES}" STREQUAL "")
            message(FATAL_ERROR "Can't find panel library")
        endif()
    endif()
endif()

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    set(system_lldb_libs
        lldbPluginProcessLinux
        lldbPluginProcessPOSIX
       )
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    set(system_lldb_libs
        lldbPluginSymbolVendorMacOSX
        lldbPluginProcessMacOSXKernel
        lldbHostMacOSXObjCXX
        lldbPluginPlatformMacOSXObjCXX
        lldbPluginSymbolLocatorDebugSymbols
       )
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    set(system_lldb_libs
        lldbPluginProcessWindowsCommon)
endif()

if("${CXXDBG_BUILD_LLVM}" OR "${CXXDBG_BUILD_LLDB}")
    set(llvm_lldb_lib_name "liblldb")
else()
    set(llvm_lldb_lib_name "lldb")
endif()

if("${CXXDBG_LLVM_ENABLE_SHARED}")
    # we don't need to link all lldb libraries if we use shared liblldb
    set(CXXDBG_LLDB_LIBS "${llvm_lldb_lib_name}")
else()
    set(CXXDBG_LLDB_LIBS
        "${llvm_lldb_lib_name}"
        lldbBreakpoint
        lldbCommands
        lldbDataFormatters
        lldbCore
        lldbExpression
        lldbInterpreter
        lldbInitialization
        lldbSymbol
        lldbTarget
        lldbUtility
        lldbHost
        lldbValueObject
        lldbInterpreterInterfaces
        lldbVersion
        lldbPluginClangCommon
        lldbPluginSymbolFileDWARF
        lldbPluginSymbolFileSymtab
        lldbPluginSymbolFilePDB
        lldbPluginSymbolFileNativePDB
        lldbPluginSymbolFileBreakpad
        lldbPluginSymbolFileCTF
        lldbPluginSymbolFileJSON
        lldbPluginDynamicLoaderStatic
        lldbPluginDynamicLoaderPosixDYLD
        lldbPluginDynamicLoaderWindowsDYLD
        lldbPluginDynamicLoaderDarwinKernel
        lldbPluginDynamicLoaderWasmDYLD
        lldbPluginDynamicLoaderHexagonDYLD
        lldbPluginDynamicLoaderFreeBSDKernel
        lldbPluginObjectFileMachO
        lldbPluginObjectFileELF
        lldbPluginObjectFileBreakpad
        lldbPluginObjectFilePDB
        lldbPluginObjectFileWasm
        lldbPluginObjectFileCOFF
        lldbPluginObjectFileXCOFF
        lldbPluginObjectFileJSON
        lldbPluginObjectFileMinidump
        lldbPluginSymbolVendorELF
        lldbPluginSymbolVendorWasm
        lldbPluginSymbolVendorPECOFF
        lldbPluginObjectContainerBSDArchive
        lldbPluginObjectContainerMachOArchive
        lldbPluginProcessGDBRemote
        lldbPluginProcessMachCore
        lldbPluginProcessUtility
        lldbPluginProcessMinidump
        lldbPluginScriptedProcess
        lldbPluginPlatformGDB
        lldbPluginPlatformFreeBSD
        lldbPluginPlatformNetBSD
        lldbPluginPlatformOpenBSD
        lldbPluginPlatformLinux
        lldbPluginPlatformPOSIX
        lldbPluginPlatformWindows
        lldbPluginPlatformMacOSX
        lldbPluginPlatformAndroid
        lldbPluginPlatformQemuUser
        lldbPluginSystemRuntimeMacOSX
        lldbPluginDynamicLoaderMacOSXDYLD
        lldbPluginUnwindAssemblyInstEmulation
        lldbPluginUnwindAssemblyX86
        lldbPluginAppleObjCRuntime
        lldbPluginCPPRuntime
        lldbPluginObjCRuntime
        lldbPluginGNUstepObjCRuntime
        lldbPluginCXXItaniumABI
        lldbPluginABIAArch64
        lldbPluginABIX86
        lldbPluginABIARM
        lldbPluginInstructionARM
        lldbPluginInstructionARM64
        lldbPluginInstructionMIPS
        lldbPluginInstructionMIPS64
        lldbPluginInstructionPPC64
        lldbPluginInstructionRISCV
        lldbPluginInstructionLoongArch
        lldbPluginMemoryHistoryASan
        lldbPluginJITLoaderGDB
        lldbPluginObjectFilePECOFF
        lldbPluginProcessElfCore
        lldbPluginArchitectureArm
        lldbPluginArchitectureAArch64
        lldbPluginArchitectureMips
        lldbPluginArchitecturePPC64

        lldbPluginCPlusPlusLanguage
        lldbPluginObjCLanguage
        lldbPluginObjCPlusPlusLanguage

        lldbPluginInstrumentationRuntimeASan
        lldbPluginInstrumentationRuntimeTSan
        lldbPluginInstrumentationRuntimeMainThreadChecker
        lldbPluginInstrumentationRuntimeUBSan

        lldbPluginExpressionParserClang

        lldbPluginScriptInterpreterNone
        lldbPluginStructuredDataDarwinLog

        lldbPluginTraceExporterCTF

        lldbPluginTypeSystemClang

        lldbPluginDisassemblerLLVMC

        lldbPluginObjectContainerMachOFileset
        lldbPluginObjectFilePlaceholder
        lldbPluginTraceExporterCommon
        lldbPluginInstrumentationRuntimeUtility
        lldbPluginInstrumentationRuntimeASanLibsanitizers
        lldbPluginClangREPL

        lldbPluginRegisterTypeBuilderClang
        lldbPluginSymbolLocatorDebuginfod
        lldbPluginSymbolLocatorDefault

        ${system_lldb_libs}
    )
endif()

if("${CXXDBG_BUILD_LLVM}")
    if("${CXXDBG_LLVM_ENABLE_SHARED}" OR "${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
        set(libclang_name "libclang")
    else()
        set(libclang_name "libclang_static")
    endif()
else()
    set(libclang_name "clang")
endif()

set(CXXDBG_LLVM_LIBS
    # clang/llvm libraries
    clangCodeGen
    LLVMIRReader
    LLVMIRPrinter
    LLVMInstrumentation
    LLVMLinker
    ${libclang_name}
    clangRewriteFrontend
    clangRewrite
    clangIndex
    clangFormat
    clangTooling
    clangToolingCore
    clangToolingInclusions
    clangFrontend
    clangDriver
    clangParse
    clangSerialization
    clangSema
    clangAnalysis
    clangEdit
    clangAST
    clangASTMatchers
    clangLex
    clangBasic
    clangSupport
    clangAPINotes
    LLVMInterpreter
    LLVMAsmParser
    LLVMBitWriter
    LLVMipo
    LLVMMCJIT
    LLVMMCDisassembler
    LLVMOption
    LLVMAsmPrinter
    LLVMSelectionDAG
    LLVMVectorize
    LLVMProfileData
    LLVMMCParser
    LLVMCodeGen
    LLVMCodeGenTypes
    LLVMExecutionEngine
    LLVMRuntimeDyld
    LLVMObjCARCOpts
    LLVMScalarOpts
    LLVMInstCombine
    LLVMTransformUtils
    LLVMObject
    LLVMBitReader
    LLVMAnalysis
    LLVMTarget
    LLVMCore
    LLVMMC
    LLVMSupport
    LLVMDemangle
    LLVMBinaryFormat
    LLVMGlobalISel
    LLVMDebugInfoDWARF
    LLVMDebugInfoCodeView
    LLVMDebugInfoPDB
    LLVMDebugInfoMSF
    LLVMCoverage
    LLVMMIRParser
    LLVMCFGuard
    LLVMAggressiveInstCombine
    LLVMFrontendOpenMP
    LLVMFrontendAtomic
    LLVMFrontendOffloading
    LLVMBitstreamReader
    LLVMRemarks
    LLVMTextAPI
    LLVMOrcTargetProcess
    LLVMTargetParser
    LLVMCGData
    LLVMLTO
    LLVMPasses
    LLVMCoroutines
    LLVMSandboxIR
    LLVMHipStdPar
    LLVMWindowsDriver
    LLVMFrontendDriver
    LLVMFrontendHLSL
    LLVMDebuginfod
)

set(llvm_targets
    AArch64
#    AMDGPU
    ARM
#    BPF
#    Hexagon
#    Lanai
#    Mips
#    MSP430
#    NVPTX
#    PowerPC
#    RISCV
#    Sparc
#    SystemZ
#    WebAssembly
    X86
#    XCore
   )

set(llvm_targets_with_utils
    AArch64
#    AMDGPU
    ARM
#    RISCV
#   X86
   )

set(llvm_targets_with_asm_parser
    AArch64
#    AMDGPU
    ARM
#    BPF
#    Hexagon
#    Lanai
#    Mips
#    MSP430
#    PowerPC
#    RISCV
#    Sparc
#    SystemZ
#    WebAssembly
    X86
   )

set(llvm_targets_with_disassembler
    AArch64
#    AMDGPU
    ARM
#    BPF
    Hexagon
#    Lanai
#    Mips
#    MSP430
#    PowerPC
#    RISCV
#    Sparc
#    SystemZ
#    WebAssembly
    X86
#    XCore
   )

foreach(targ ${llvm_targets})
    list(APPEND CXXDBG_LLVM_LIBS
         LLVM${targ}Desc
         LLVM${targ}Info
         LLVM${targ}CodeGen)

    list(FIND llvm_targets_with_asm_parser "${targ}" res)
    if(NOT "${res}" EQUAL "-1")
        list(APPEND CXXDBG_LLVM_LIBS LLVM${targ}AsmParser)
    endif()

    list(FIND llvm_targets_with_disassembler "${targ}" res)
    if(NOT "${res}" EQUAL "-1")
        list(APPEND CXXDBG_LLVM_LIBS LLVM${targ}Disassembler)
    endif()

    list(FIND llvm_targets_with_utils "${targ}" res)
    if(NOT "${res}" EQUAL "-1")
        list(APPEND CXXDBG_LLVM_LIBS LLVM${targ}Utils)
    endif()
endforeach()


if("${CXXDBG_BUILD_LLVM}")
    add_subdirectory(build/cxxdbg-llvm cxxdbg-llvm)
    set(LLVM_ROOT "${CMAKE_CURRENT_BINARY_DIR}/cxxdbg-llvm/llvm")
    set(LLDB_ROOT "${LLVM_ROOT}")
elseif("${CXXDBG_BUILD_LLDB}")
    add_subdirectory(build/cxxdbg-lldb cxxdbg-lldb)
    set(LLDB_ROOT "${CMAKE_CURRENT_BINARY_DIR}/cxxdbg-lldb")
endif()

if(NOT "${CXXDBG_BUILD_LLVM}")
    if("${LLVM_ROOT}" STREQUAL "")
        message(FATAL_ERROR "Path to LLVM root is not set. Please set LLVM_ROOT variable")
    endif()

    if(NOT EXISTS "${LLVM_ROOT}" OR NOT IS_DIRECTORY "${LLVM_ROOT}")
        message(FATAL_ERROR "Directory '${LLVM_ROOT}' does not exist")
    endif()

    if("${LLDB_ROOT}" STREQUAL "")
        set(LLDB_ROOT "${LLVM_ROOT}")
    else()
        if(NOT EXISTS "${LLDB_ROOT}" OR NOT IS_DIRECTORY "${LLDB_ROOT}")
            message(FATAL_ERROR "Directory '${LLDB_ROOT}' does not exist")
        endif()
    endif()

    if(NOT "${CXXDBG_BUILD_LLDB}")
        foreach(lib ${CXXDBG_LLDB_LIBS})
            if("${CXXDBG_LLVM_ENABLE_SHARED}")
                add_library("${lib}" SHARED IMPORTED)
                if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
                    set_property(TARGET "${lib}" PROPERTY IMPORTED_LOCATION "${LLDB_ROOT}/bin/${lib}${CMAKE_SHARED_LIBRARY_SUFFIX}")
                    set_property(TARGET "${lib}" PROPERTY IMPORTED_IMPLIB "${LLDB_ROOT}/lib/${CMAKE_IMPORT_LIBRARY_PREFIX}${lib}${CMAKE_IMPORT_LIBRARY_SUFFIX}")
                else()
                    set_property(TARGET "${lib}" PROPERTY IMPORTED_LOCATION "${LLDB_ROOT}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}${lib}${CMAKE_SHARED_LIBRARY_SUFFIX}")
                endif()
            else()
                add_library("${lib}" STATIC IMPORTED)
                set_property(TARGET "${lib}" PROPERTY IMPORTED_LOCATION "${LLDB_ROOT}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}${lib}${CMAKE_STATIC_LIBRARY_SUFFIX}")
            endif()
        endforeach()
    endif()

    foreach(lib ${CXXDBG_LLVM_LIBS})
        if("${CXXDBG_LLVM_ENABLE_SHARED}")
            add_library("${lib}" SHARED IMPORTED)

            if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
                set_property(TARGET "${lib}" PROPERTY IMPORTED_LOCATION "${LLVM_ROOT}/bin/${lib}${CMAKE_SHARED_LIBRARY_SUFFIX}")
                set_property(TARGET "${lib}" PROPERTY IMPORTED_IMPLIB "${LLVM_ROOT}/lib/${CMAKE_IMPORT_LIBRARY_PREFIX}${lib}${CMAKE_IMPORT_LIBRARY_SUFFIX}")
            else()
                set_property(TARGET "${lib}" PROPERTY IMPORTED_LOCATION "${LLVM_ROOT}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}${lib}${CMAKE_SHARED_LIBRARY_SUFFIX}")
            endif()
        else()
            add_library("${lib}" STATIC IMPORTED)
            set_property(TARGET "${lib}" PROPERTY IMPORTED_LOCATION "${LLVM_ROOT}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}${lib}${CMAKE_STATIC_LIBRARY_SUFFIX}")
        endif()
    endforeach()
endif()


if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    set(start_group)
    set(end_group)
else()
    set(start_group "-Wl,--start-group")
    set(end_group "-Wl,--end-group")
endif()

add_library(cxxdbg-llvm-libraries INTERFACE)
target_link_libraries(cxxdbg-llvm-libraries INTERFACE
                      ${start_group}
                      ${CXXDBG_LLDB_LIBS}
                      ${CXXDBG_LLVM_LIBS}
                      ${end_group}
                     )

if (LLDB_ENABLE_CURSES)
    target_link_libraries(cxxdbg-llvm-libraries INTERFACE ${CURSES_LIBRARIES} ${PANEL_LIBRARIES})
endif()

if (LLDB_ENABLE_LIBEDIT)
    target_link_libraries(cxxdbg-llvm-libraries INTERFACE edit)
endif()

if (LLVM_ENABLE_TERMINFO)
    target_link_libraries(cxxdbg-llvm-libraries INTERFACE edit)
endif()

# Linking with external libraries manually if we use external LLVM buils
if(NOT "${CXXDBG_BUILD_LLVM}")
    # Linking with libxml2
    find_package(LibXml2)
    target_link_libraries(cxxdbg-llvm-libraries INTERFACE LibXml2::LibXml2)

    # Linking with zlib
    find_package(ZLIB)
    target_link_libraries(cxxdbg-llvm-libraries INTERFACE ZLIB::ZLIB)

    # Linking with LZMA
    find_package(LibLZMA)
    target_link_libraries(cxxdbg-llvm-libraries INTERFACE LibLZMA::LibLZMA)
endif()


if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    target_link_libraries(cxxdbg-llvm-libraries INTERFACE
                            "-framework Cocoa"
                            "-framework Security"
                            "-framework Foundation"
                            "-F /System/Library/PrivateFrameworks"
                            "-framework DebugSymbols"
                            compression)
endif()

# Linking with zlib
find_package(ZLIB)
target_link_libraries(cxxdbg-llvm-libraries INTERFACE ZLIB::ZLIB)


# linking with system libraries
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    target_link_libraries(cxxdbg-llvm-libraries INTERFACE ws2_32 psapi rpcrt4 version dbghelp)
else()
    target_link_libraries(cxxdbg-llvm-libraries INTERFACE dl -pthread)
endif()


add_library(llvm-includes INTERFACE)

# all required LLVM headers should be generated before dependent targets
if("${CXXDBG_BUILD_LLVM}")
    add_dependencies(llvm-includes clang)
endif()

# llvm/Support/DataTypes.h required __STDC_LIMIT_MACROS and __STDC_CONSTANT_MACROS
target_compile_definitions(llvm-includes INTERFACE -D__STDC_LIMIT_MACROS
                                                   -D__STDC_CONSTANT_MACROS
                                                   -D__STDC_FORMAT_MACROS)

# define IMPORT_LIBLLDB macro on Windows if we are using shared liblldb
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows" AND "${CXXDBG_LLVM_ENABLE_SHARED}")
    target_compile_definitions(llvm-includes INTERFACE "-DIMPORT_LIBLLDB=1")
endif()

# llvm include directories

target_include_directories(llvm-includes INTERFACE
                          "${LLVM_ROOT}/include"
                          "${LLVM_ROOT}/tools/clang/include"
                          "${LLVM_ROOT}/tools/lldb/include"
                          )

# CIndex requires CINDEX_NO_EXPORTS definition for static builds on Windows
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows" AND NOT "${CXXDBG_LLVM_ENABLE_SHARED}")
    target_compile_definitions(llvm-includes INTERFACE "-DCINDEX_NO_EXPORTS=1")
endif()

if("${LLDB_ROOT}" STREQUAL "${LLVM_ROOT}")
    target_include_directories(llvm-includes INTERFACE "${LLVM_ROOT}/tools/lldb/include")
else()
    target_include_directories(llvm-includes INTERFACE "${LLDB_ROOT}/lldb/include")
endif()

# if we are building LLVM from sources or LLVM_ROOT points to LLVM build root (not installed LLVM)
# then we have to add include directories from sources

if("${CXXDBG_BUILD_LLVM}" OR NOT EXISTS "${LLVM_ROOT}/include/lldb/API/SBDebugger.h")
    message(STATUS "Adding LLVM source directories into list of include dirs")
    target_include_directories(llvm-includes INTERFACE
                               "${CMAKE_CURRENT_SOURCE_DIR}/llvm-project/llvm/include"
                               "${CMAKE_CURRENT_SOURCE_DIR}/llvm-project/clang/include"
                               "${CMAKE_CURRENT_SOURCE_DIR}/llvm-project/lldb/include"
                               "${CMAKE_CURRENT_SOURCE_DIR}/llvm-project/lldb/source")
endif()

