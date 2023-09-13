build_lib_example(
    NAME tsn-example1
    SOURCE_FILES tsn-example1.cc
    LIBRARIES_TO_LINK 
        ${libtsn}
        ${libcore}
        ${libcsma}
        ${libpoint-to-point}
        ${libinternet}
        ${libapplications}
)

build_lib_example(
    NAME tsn-example2
    SOURCE_FILES tsn-example2.cc
    LIBRARIES_TO_LINK 
        ${libtsn}
        ${libcore}
        ${libcsma}
        ${libpoint-to-point}
        ${libinternet}
        ${libapplications}
)

build_lib_example(
    NAME tsn-example3
    SOURCE_FILES tsn-example3.cc
    LIBRARIES_TO_LINK 
        ${libtsn}
        ${libcore}
        ${libcsma}
        ${libpoint-to-point}
        ${libinternet}
        ${libapplications}
)

build_lib_example(
    NAME tsn-example4
    SOURCE_FILES tsn-example4.cc
    LIBRARIES_TO_LINK
        ${libtsn}
        ${libcore}
        ${libcsma}
        ${libinternet}
        ${libapplications}
)

build_lib_example(
    NAME tsn-example5
    SOURCE_FILES tsn-example5.cc
    LIBRARIES_TO_LINK 
        ${libtsn}
        ${libcore}
        ${libcsma}
        ${libinternet}
        ${libapplications}
)


