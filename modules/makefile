
DIRS:=$(wildcard lab-*)

build-modules:
	make clean
	make prepare
	@for dir in $(DIRS); do \
        echo "Building module: $$dir"; \
        cd $$dir && make module.ko copy-to-fs && cd -; \
        done
	make rebuild

clean:
	@for dir in $(DIRS); do \
        echo "Building module: $$dir"; \
        cd $$dir && make clean && cd -; \
        done

prepare:
	cp ./aux/module.lds.S-$(__BUILD_ARCH) /sources/linux/scripts/module.lds

rebuild: 
	/sources/mini-bootstrap.sh


