include ../../config.mk

SOURCES := $(filter-out %/generate.sh %.o, $(shell $(FIND) -name "*.*"))

.PHONY:
generate:
	./generate.sh

.PHONY:
install: $(SOURCES)
	@for source in $(SOURCES); do 																\
		$(INSTALL) -v -D -m 644 $$source $(patsubst %, $(CSPC_DEFAULT_STD_PATH)/%, $$source);	\
	done

.PHONY:
uninstall:
	if [ ! -z $(CSPC_DEFAULT_STD_PATH) ]; then	\
		$(RM) -rf $(CSPC_DEFAULT_STD_PATH); 	\
	fi

