T?=amd64
E?=minimal

build-container:
	docker build --platform $(T) . -t lkp-$(T) --build-arg __ARCH=$(T) --build-arg __ENV=$(E)

build-sys:
	docker run -ti -v "`pwd`:/repo" --rm lkp-$(T):latest /bin/bash -c "cd /repo/modules && make build-modules"

rebuild:
	make build-container 
	make build-sys

enter-container:
	docker run -ti -p 5900:5900 -p 8080:6000 -v "`pwd`:/repo" --privileged --rm lkp-$(T):latest /bin/bash 

attach-container:
	docker exec -it $$(docker ps --filter ancestor=lkp-$(T) --format '{{.ID}}') /bin/bash

stop-container:
	ps aux | grep -E 'bzImage' | grep -v grep | awk '{print $$2}' | xargs kill -9

# The following work if you have installed qemu locally on your machine.

build-sys-nrun:
	make build-sys
	make run

run:
	./stage/start-qemu.sh --arch $T

dbg:
	./stage/start-qemu.sh --arch $T --dbg
