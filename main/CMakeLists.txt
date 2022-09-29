# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES )

set(COMPONENT_SRCS "main.c utils.c control/data_logger.c connectivity/spi.c peripherals/sd_card.c peripherals/rc.c")
set(COMPONENT_ADD_INCLUDEDIRS "")

register_component()
