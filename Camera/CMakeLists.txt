
idf_component_register( SRCS "src/jpg2bin.cpp" "main.cpp" # "mainCamSD.cpp" "main.cpp"
                        INCLUDE_DIRS "." "include/" )

component_compile_options(-std=gnu++2a)

