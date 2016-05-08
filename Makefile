all:
	mkdir build
	cd build; cmake ..
	+$(MAKE) -sC build
