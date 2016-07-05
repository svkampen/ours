all:
	mkdir -p build
	cd src; ./make-proto.sh
	cd build; cmake -G Ninja ..
	cd build; ninja
