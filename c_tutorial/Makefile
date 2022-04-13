PK = /nix/store/1yyy5ragpgw1kg4dyc4dp3sjzvwvsm8r-riscv-pk/bin/pk

tuto%: build/tuto%
	@spike $(PK) $<

playground: build/playground
	@spike $(PK) $<

build/%: %.c | build
	@riscv64-unknown-elf-gcc $^ -o $@

build:
	@mkdir -p $@

clean:
	rm -rf build
