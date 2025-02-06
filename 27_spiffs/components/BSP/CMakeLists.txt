set(src_dirs
            LED
            MYIIC
            XL9555
            MYSPI
            SPILCD
            SPIFFS)

set(include_dirs
            LED
            MYIIC
            XL9555
            MYSPI
            SPILCD
            SPIFFS)

set(requires
            driver
            esp_lcd
            spiffs)

idf_component_register(SRC_DIRS ${src_dirs} INCLUDE_DIRS ${include_dirs} REQUIRES ${requires})

component_compile_options(-ffast-math -O3 -Wno-error=format=-Wno-format)