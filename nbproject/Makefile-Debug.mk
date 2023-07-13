#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=arm-brcm-linux-gnueabi-gcc
CCC=arm-brcm-linux-gnueabi-g++
CXX=arm-brcm-linux-gnueabi-g++
FC=g77.exe
AS=as

# Macros
CND_PLATFORM=Gnueabi-Windows
CND_DLIB_EXT=dll
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include NbMakefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/Debug/debug.o \
	${OBJECTDIR}/Database/DatabaseFunc.o \
	${OBJECTDIR}/ECR/ECRTrans.o \
	${OBJECTDIR}/Utils/Utils.o \
	${OBJECTDIR}/ECR/MultiAptrans.o \
	${OBJECTDIR}/Main/ECRMain.o


# C Compiler Flags
CFLAGS="-I${SDKV5SINC}" -fsigned-char -Wundef -Wstrict-prototypes -Wno-trigraphs -Wimplicit -Wformat 

# CC Compiler Flags
CCFLAGS="-I${SDKV5SINC}" -fsigned-char -Wundef -Wno-trigraphs -Wimplicit -Wformat 
CXXFLAGS="-I${SDKV5SINC}" -fsigned-char -Wundef -Wno-trigraphs -Wimplicit -Wformat 

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-lcaethernet -lcafont -lcafs -lcakms -lcalcd -lcamodem -lcapmodem -lcaprt -lcartc -lcauart -lcauldpm -lcausbh -lcagsm -lcabarcode -lpthread -ldl -lcaclvw -lcatls -lctosapi -lz -lssl -lcrypto -lcurl -lfreetype -lxml2 -lcaemvl2 -lcasqlite -lcaxml -lv5smultiap -lcaclvw -lcaemvl2 -lv5smultiap -lcaclentry -lcaclmdl -lcaemvl2 -lv3_libepadso -lcaemvl2ap -lv5scfgexpress -lv5sinput -lv5smultiap

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk dist/V5S/${PRONAME}/10V5S_App/${APPNAME}.exe

dist/V5S/${PRONAME}/10V5S_App/${APPNAME}.exe: ${OBJECTFILES}
	${MKDIR} -p dist/V5S/${PRONAME}/10V5S_App
	arm-brcm-linux-gnueabi-g++ -L . "-L${SDKV5SLIB}" "-L${SDKV5SLIBN}" -o dist/V5S/${PRONAME}/10V5S_App/${APPNAME}  ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/Debug/debug.o: Debug/debug.c 
	${MKDIR} -p ${OBJECTDIR}/Debug
	$(COMPILE.c) -g -I/cygdrive/C/Program\ Files/Castles/VEGA5000S/include -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA5000S/include -o ${OBJECTDIR}/Debug/debug.o Debug/debug.c

${OBJECTDIR}/Database/DatabaseFunc.o: Database/DatabaseFunc.c 
	${MKDIR} -p ${OBJECTDIR}/Database
	$(COMPILE.c) -g -I/cygdrive/C/Program\ Files/Castles/VEGA5000S/include -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA5000S/include -o ${OBJECTDIR}/Database/DatabaseFunc.o Database/DatabaseFunc.c

${OBJECTDIR}/ECR/ECRTrans.o: ECR/ECRTrans.c 
	${MKDIR} -p ${OBJECTDIR}/ECR
	$(COMPILE.c) -g -I/cygdrive/C/Program\ Files/Castles/VEGA5000S/include -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA5000S/include -o ${OBJECTDIR}/ECR/ECRTrans.o ECR/ECRTrans.c

${OBJECTDIR}/Utils/Utils.o: Utils/Utils.c 
	${MKDIR} -p ${OBJECTDIR}/Utils
	$(COMPILE.c) -g -I/cygdrive/C/Program\ Files/Castles/VEGA5000S/include -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA5000S/include -o ${OBJECTDIR}/Utils/Utils.o Utils/Utils.c

${OBJECTDIR}/ECR/MultiAptrans.o: ECR/MultiAptrans.c 
	${MKDIR} -p ${OBJECTDIR}/ECR
	$(COMPILE.c) -g -I/cygdrive/C/Program\ Files/Castles/VEGA5000S/include -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA5000S/include -o ${OBJECTDIR}/ECR/MultiAptrans.o ECR/MultiAptrans.c

${OBJECTDIR}/Main/ECRMain.o: Main/ECRMain.c 
	${MKDIR} -p ${OBJECTDIR}/Main
	$(COMPILE.c) -g -I/cygdrive/C/Program\ Files/Castles/VEGA5000S/include -I/cygdrive/C/Program\ Files\ \(x86\)/Castles/VEGA5000S/include -o ${OBJECTDIR}/Main/ECRMain.o Main/ECRMain.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} dist/V5S/${PRONAME}/10V5S_App/${APPNAME}.exe

# Subprojects
.clean-subprojects:
