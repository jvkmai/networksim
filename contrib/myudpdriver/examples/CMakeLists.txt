build_lib_example(
    NAME example-app
    SOURCE_FILES example_app.cc
    LIBRARIES_TO_LINK 
        ${libtsn}
        ${libcore}
        ${libcsma}
        ${libinternet}
        ${libapplications}
        ${myudpdriver}
)
