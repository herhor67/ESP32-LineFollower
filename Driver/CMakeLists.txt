
idf_component_register( SRCS "src/Buzzer.cpp" "src/Controller.cpp" "src/ESP32Encoder.cpp" "main.cpp"
                        INCLUDE_DIRS "." "include/" )

component_compile_options(-std=gnu++17)

