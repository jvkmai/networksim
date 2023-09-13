build_lib(
    LIBNAME myudpdriver
    SOURCE_FILES
        model/rtps-udpdriver.cc
        model/app-publisher.cc
        model/app-subscriber.cc
        helper/publisher_helper.cc
        helper/subscriber_helper.cc
        helper/RTPS-UdpDriverHelper.cc
    HEADER_FILES
        model/rtps-udpdriver.h
        model/app-publisher.h
        model/app-subscriber.h
        helper/publisher_helper.h
        helper/subscriber_helper.h
        helper/RTPS-UdpDriverHelper.h
    LIBRARIES_TO_LINK 
                ${libcore}
                ${libinternet}
                ${libstats}
                lwip
                embeddedrtps
)

