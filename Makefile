.PHONY: build release test test-all

build:
	@cmake --preset=build
	@cmake --build build

release:
	@cmake --preset=release
	@cmake --build release

image:
	@bash docker/build.sh

test:
	@./build/tests/test_main --gtest_filter=$(cases)

test-all:
	@./build/tests/test_main

