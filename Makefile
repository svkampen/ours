all:
	mkdir -p build
	cd src; ./make-proto.sh
	cd build; cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -G Ninja ..
	cd build; ninja
