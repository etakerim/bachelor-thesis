set(COMPONENT_SRCS  "src/main.c" 
                    "src/window.c"
                    "src/pipeline.c"
                    "src/events.c"
                    "src/statistics.c"
                    "src/inertial_unit.c"
                    "src/peripheral.c"
                    "src/serialize.c")
set(COMPONENT_ADD_INCLUDEDIRS "include")

register_component()
