#-------------------------------------------------
#
# Project created by QtCreator 2014-04-22T23:42:28
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = cxxdbggui
TEMPLATE = app


FORMS += AddWatchpointDialog.ui \
    ExpressionsListDialog.ui \
	RegularExpressionDialog.ui \
        ListOfListsDialog.ui \
        FunctionNameDialog.ui


SOURCES += main.cpp\
        AddWatchpointDialog.cpp \
        MainWindow.cpp \
    DbgTerminalWidget.cpp \
    CxxdbgApplication.cpp \
    LoadExecutableDialog.cpp \
    FileSelectWidget.cpp \
    QtApplicationAsyncEventHandler.cpp \
    AboutDialog.cpp \
    SourceCodeWidget.cpp \
    CentralWidget.cpp \
    LineNumbersWidget.cpp \
    SourceCodePlainTextEdit.cpp \
    FileSystemDockWidget.cpp \
    FunctionNameDialog.cpp \
    LoadingExecutableDialog.cpp \
    SourceTreeDockWidget.cpp \
    StoppingDialog.cpp \
    UnloadingExecutableDialog.cpp \
    ThreadsDockWidget.cpp \
    CallStackDockWidget.cpp \
    AddSourcePosBreakpointDialog.cpp \
    BreakpointsDockWidget.cpp \
    AddFunctionBreakpointDialog.cpp \
    LineNumberDialog.cpp \
    DebugSettingsWidget.cpp \
    RegularExpressionDialog.cpp \
    ExpressionsListDialog.cpp \
    ListOfListsDialog.cpp \
    SourceTreesModel.cpp \
    CxxdbgDockWidget.cpp

HEADERS  += MainWindow.h \
    AddWatchpointDialog.h \
    DbgTerminalWidget.h \
    CxxdbgApplication.h \
    LoadExecutableDialog.h \
    FileSelectWidget.h \
    QtApplicationAsyncEventHandler.h \
    AboutDialog.h \
    SourceCodeWidget.h \
    CentralWidget.h \
    LineNumbersWidget.h \
    SourceCodePlainTextEdit.h \
    FileSystemDockWidget.h \
    FunctionNameDialog.h \
    LoadingExecutableDialog.h \
    SourceTreeDockWidget.h \
    StoppingDialog.h \
    UnloadingExecutableDialog.h \
    ThreadsDockWidget.h \
    CallStackDockWidget.h \
    AddSourcePosBreakpointDialog.h \
    BreakpointsDockWidget.h \
    AddFunctionBreakpointDialog.h \
    LineNumberDialog.h \
    DebugSettingsWidget.h \
    RegularExpressionDialog.h \
    ExpressionsListDialog.h \
    ListOfListsDialog.h \
    SourceTreesModel.h \
    CxxdbgDockWidget.h

win32:CONFIG(release, debug|release) {
    LIBS += -L$$OUT_PWD/../cxxdbgcore/release/ -lcxxdbgcore
}
else:win32:CONFIG(debug, debug|release) {
    LIBS += -L$$OUT_PWD/../cxxdbgcore/debug/ -lcxxdbgcore
}
else:unix {
    LIBS += -L$$OUT_PWD/../cxxdbgcore/ -lcxxdbgcore
    PRE_TARGETDEPS += $$OUT_PWD/../cxxdbgcore/libcxxdbgcore.a
}

INCLUDEPATH += $$PWD/../cxxdbgcore
DEPENDPATH += $$PWD/../cxxdbgcore

QMAKE_CXXFLAGS += -std=c++0x

CONFIG += no_lflags_merge


# lldb libraries
LIBS += -llldb
LIBS += -Wl,--start-group
LIBS += -llldbAPI
LIBS += -llldbBreakpoint
LIBS += -llldbCommands
LIBS += -llldbDataFormatters
LIBS += -llldbHostCommon
LIBS += -llldbCore
LIBS += -llldbExpression
LIBS += -llldbInterpreter
LIBS += -llldbSymbol
LIBS += -llldbTarget
LIBS += -llldbUtility
LIBS += -llldbUtil
LIBS += -llldbPluginDisassemblerLLVM
LIBS += -llldbPluginSymbolFileDWARF
LIBS += -llldbPluginSymbolFileSymtab
LIBS += -llldbPluginDynamicLoaderStatic
LIBS += -llldbPluginDynamicLoaderPosixDYLD
LIBS += -llldbPluginObjectFileMachO
LIBS += -llldbPluginObjectFileELF
LIBS += -llldbPluginSymbolVendorELF
LIBS += -llldbPluginObjectContainerBSDArchive
LIBS += -llldbPluginObjectContainerMachOArchive
LIBS += -llldbPluginProcessGDBRemote
LIBS += -llldbPluginProcessMachCore
LIBS += -llldbPluginProcessUtility
LIBS += -llldbPluginPlatformGDB
LIBS += -llldbPluginPlatformFreeBSD
LIBS += -llldbPluginPlatformLinux
LIBS += -llldbPluginPlatformPOSIX
LIBS += -llldbPluginPlatformWindows
LIBS += -llldbPluginPlatformMacOSX
LIBS += -llldbPluginDynamicLoaderMacOSXDYLD
LIBS += -llldbPluginUnwindAssemblyInstEmulation
LIBS += -llldbPluginUnwindAssemblyX86
LIBS += -llldbPluginAppleObjCRuntime
LIBS += -llldbPluginCXXItaniumABI
LIBS += -llldbPluginABIMacOSX_arm
LIBS += -llldbPluginABIMacOSX_i386
LIBS += -llldbPluginABISysV_x86_64
LIBS += -llldbPluginInstructionARM
LIBS += -llldbPluginObjectFilePECOFF
LIBS += -llldbPluginOSPython
LIBS += -llldbHostLinux
LIBS += -llldbPluginProcessLinux
LIBS += -llldbPluginProcessPOSIX
LIBS += -llldbPluginProcessElfCore
LIBS += -Wl,--end-group

# clang/llvm libraries
LIBS += -lclangCodeGen
LIBS += -lLLVMIRReader
LIBS += -lLLVMInstrumentation
LIBS += -lLLVMLinker
LIBS += -lclangRewriteFrontend
LIBS += -lclangFrontend
LIBS += -lclangDriver
LIBS += -lclangRewriteCore
LIBS += -lclangParse
LIBS += -lclangSerialization
LIBS += -lclangSema
LIBS += -lclangAnalysis
LIBS += -lclangEdit
LIBS += -lclangAST
LIBS += -lclangLex
LIBS += -lclangBasic
LIBS += -lLLVMX86AsmParser
LIBS += -lLLVMX86Disassembler
LIBS += -lLLVMInterpreter
LIBS += -lLLVMX86CodeGen
LIBS += -lLLVMAsmParser
LIBS += -lLLVMBitWriter
LIBS += -lLLVMipo
LIBS += -lLLVMBitReader
LIBS += -lLLVMMCJIT
LIBS += -lLLVMMCDisassembler
LIBS += -lLLVMOption
LIBS += -lLLVMAsmPrinter
LIBS += -lLLVMSelectionDAG
LIBS += -lLLVMX86Desc
LIBS += -lLLVMVectorize
LIBS += -lLLVMJIT
LIBS += -lLLVMMCParser
LIBS += -lLLVMX86AsmPrinter
LIBS += -lLLVMX86Info
LIBS += -lLLVMCodeGen
LIBS += -lLLVMExecutionEngine
LIBS += -lLLVMRuntimeDyld
LIBS += -lLLVMX86Utils
LIBS += -lLLVMObjCARCOpts
LIBS += -lLLVMScalarOpts
LIBS += -lLLVMInstCombine
LIBS += -lLLVMTransformUtils
LIBS += -lLLVMipa
LIBS += -lLLVMAnalysis
LIBS += -lLLVMTarget
LIBS += -lLLVMCore
LIBS += -lLLVMMC
LIBS += -lLLVMObject
LIBS += -lLLVMSupport

# boost libraries
LIBS += -lboost_filesystem -lboost_system -lboost_program_options

# system libraries
LIBS += -ldl -static-libgcc -static-libstdc++

