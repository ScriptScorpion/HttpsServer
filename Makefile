CC := g++
CCF := -std=c++17 -lssl -lcrypto
SRC := src/main.cpp src/server.cpp
OUT := server
main:
	@$(CC) $(SRC) -o $(OUT) $(CCF)
genkeys:
	chmod +x src/certificate_gen.sh
	./src/certificate_gen.sh
