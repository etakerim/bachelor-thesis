set(COMPONENT_SRCS  "src/main.c" 
                    "src/window.c"
                    "src/pipeline.c"
                    "src/find_peak.c"
                    "src/statistics.c"
                    "src/inertial_unit.c"
                    "src/peripheral.c")
set(COMPONENT_ADD_INCLUDEDIRS "include")

register_component()
