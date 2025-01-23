set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR i686)

set(DEPEND_DIR $ENV{DEPEND_DIR})

set(CTC ${DEPEND_DIR}/ctc-linux64-atom)
set(CMAKE_C_COMPILER ${CTC}/bin/i686-aldebaran-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER ${CTC}/bin/i686-aldebaran-linux-gnu-g++)

