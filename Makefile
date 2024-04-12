exe:
	@mkdir -p target
	@g++ --std=c++11 $(CPP_FILE) -o ./target/a.out
	@./target/a.out

clean:
	@rm -rf target