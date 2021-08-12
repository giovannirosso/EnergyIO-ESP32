-include nanopb-0.4.5-linux-x86/extra/nanopb.mk
    CFLAGS = -Wall -Werror -g -O0
    CFLAGS += "-I$(NANOPB_DIR)"

install: 
ifneq ($(wildcard nanopb-0.4.5-linux-x86),)
    echo already installed
    mkdir -p ../libraries/Proto
    make proto
else
    wget "https://jpa.kapsi.fi/nanopb/download/nanopb-0.4.5-linux-x86.tar.gz"  && \
    tar -xf nanopb-0.4.5-linux-x86.tar.gz                                         && \
    rm nanopb-0.4.5-linux-x86.tar.gz
    mkdir -p ../lib/Proto
    make proto
endif

proto: message.proto
    $(PROTOC) $(PROTOC_OPTS) --nanopb_out=../lib/Proto message.proto