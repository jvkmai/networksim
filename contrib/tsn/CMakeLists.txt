set(source_files
    model/transmisson-gate-qdisc.cc
    model/net-device-list-config.cc
    model/TsnIpv4PacketFilter.cc
    model/TsnIpv6PacketFilter.cc
    model/tas-queue-disc.cc
    helper/tsn-helper.cc
)

set(header_files
    model/transmisson-gate-qdisc.h
    model/net-device-list-config.h
    model/TsnIpv4PacketFilter.h
    model/TsnIpv6PacketFilter.h
    model/tas-queue-disc.h
    helper/tsn-helper.h
)

build_lib(
    LIBNAME tsn
    SOURCE_FILES ${source_files}
    HEADER_FILES ${header_files}
    LIBRARIES_TO_LINK   
                ${libcore}
                ${libtraffic-control}
                ${myudpdriver}
                ${libflow-monitor} 

)

