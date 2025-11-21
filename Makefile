# No files with these names in top-level directory
.PHONY: docs test debug opt profile optprof clean coverage sanitize

GITVERSION := $(shell git describe --tags --abbrev=40 --always)
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
GCC_FLAGS1 = -fPIC -Wl,-Bsymbolic-functions -c
GCC_FLAGS2 = -shared -Wl,-Bsymbolic-functions,-soname,vplanetlib.so
endif
ifeq ($(UNAME_S),Darwin)
GCC_FLAGS1 = -fPIC -c
GCC_FLAGS2 = -shared -Wl,-install_name,vplanetlib.so
endif

SRC := $(wildcard src/*.c)
LSODA_ALL := $(wildcard src/liblsoda/*.c)
# Exclude deprecated/diagnostic sources from liblsoda that don't compile in this context
LSODA_SRC := $(filter-out src/liblsoda/ewset.c src/liblsoda/printcf.c src/liblsoda/cfode_static.c,$(LSODA_ALL))
INCLUDES := -Isrc -Isrc/liblsoda

default:
	-python setup.py clean --all
	-python setup.py develop

legacy:
	-gcc -o bin/vplanet $(SRC) $(LSODA_SRC) -lm $(INCLUDES) -DGITVERSION=\"$(GITVERSION)\"
	@echo ""
	@echo "=========================================================================================================="
	@echo 'To add vplanet to your $$PATH, please run the appropriate command for your shell type:'
	@echo '( You can see your shell by typing: echo $$0 )'
	@echo 'bash:    export PATH=$$PATH:$(CURDIR)/bin'
	@echo 'tsch:    set path=($$path $(CURDIR)/bin)'
	@echo 'csh :    set path=($$path $(CURDIR)/bin)'
	@echo 'or permanently add the VPLanet directory to the $$PATH by editing the appropriate environment file. e.g.:'
	@echo 'bash:    echo '"'"'export PATH=$$PATH:$(CURDIR)/bin'"'"' >> ~/.bashrc'
	@echo "=========================================================================================================="

debug:
	-gcc -g -D DEBUG -o bin/vplanet $(SRC) $(LSODA_SRC) -lm $(INCLUDES) -DGITVERSION=\"$(GITVERSION)\"

debug_no_AE:
	-gcc -g -o bin/vplanet $(SRC) $(LSODA_SRC) -lm $(INCLUDES) -DGITVERSION=\"$(GITVERSION)\"

opt:
	-gcc -o bin/vplanet $(SRC) $(LSODA_SRC) -lm -O3 $(INCLUDES) -DGITVERSION=\"$(GITVERSION)\"
	@echo ""
	@echo "=========================================================================================================="
	@echo 'To add vplanet to your $$PATH, please run the appropriate command for your shell type:'
	@echo '( You can see your shell by typing: echo $$0 )'
	@echo 'bash:    export PATH=$$PATH:$(CURDIR)/bin'
	@echo 'tsch:    set path=($$path $(CURDIR)/bin)'
	@echo 'csh :    set path=($$path $(CURDIR)/bin)'
	@echo 'or permanently add the VPLanet directory to the $$PATH by editing the appropriate environment file. e.g.:'
	@echo 'bash:    echo '"'"'export PATH=$$PATH:$(CURDIR)/bin'"'"' >> ~/.bashrc'
	@echo "=========================================================================================================="

cpp:
	g++ -o bin/vplanet $(SRC) $(LSODA_SRC) -lm -O3 -fopenmp -fpermissive -w $(INCLUDES) -DGITVERSION=\"$(GITVERSION)\"

warnings:
		-gcc -g -D DEBUG -Wunused-but-set-variable -Wunused-variable -Wfloat-equal -o bin/vplanet $(SRC) $(LSODA_SRC) -lm $(INCLUDES) -DGITVERSION=\"$(GITVERSION)\"

parallel:
	gcc -o bin/vplanet $(SRC) $(LSODA_SRC) -lm -O3 -fopenmp $(INCLUDES) -DGITVERSION=\"$(GITVERSION)\"

profile:
	-gcc -pg -o bin/vplanet $(SRC) $(LSODA_SRC) -lm $(INCLUDES) -DGITVERSION=\"$(GITVERSION)\"

optprof:
	-gcc -pg -o bin/vplanet $(SRC) $(LSODA_SRC) -lm -O3 $(INCLUDES) -DGITVERSION=\"$(GITVERSION)\"

sanitize:
	-gcc -g -fsanitize=address -o bin/vplanet $(SRC) $(LSODA_SRC) -lm $(INCLUDES) -DGITVERSION=\"$(GITVERSION)\"

test:
	-gcc -o bin/vplanet $(SRC) $(LSODA_SRC) -lm -O3 $(INCLUDES) -DGITVERSION=\"$(GITVERSION)\"
	-pytest --tb=short

coverage:
	-rm -f ./gcov/*.gcda ./gcov/*.gcno ./.coverage
	-mkdir -p ./gcov
	-cd gcov && gcc -coverage -o ./../bin/vplanet ./../src/*.c ./../src/liblsoda/*.c -lm -I./../src -I./../src/liblsoda
	-python -m pytest --tb=short tests --junitxml=./junit/test-results.xml
	-lcov --capture --directory ./gcov --output-file ./.coverage
	-genhtml ./.coverage --output-directory ./gcov/html

docs:
	-make -C docs html && echo 'Documentation available at `docs/.build/html/index.html`.'

shared:
	-gcc ${GCC_FLAGS1} $(INCLUDES) $(SRC) $(LSODA_SRC)
	-gcc ${GCC_FLAGS2} -o bin/vplanetlib.so *.o -lc

clean:
	rm -f bin/vplanet
	rm -rf gcov
	rm -rf .pytest_cache
	rm -f src/*.o
	rm -f bin/vplanetlib.so
