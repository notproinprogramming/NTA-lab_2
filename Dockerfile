FROM alpine:latest AS builder
RUN apk add --no-cache g++ make
WORKDIR /app
COPY algorithms.hpp algorithms.cpp prog.cpp ./ 
RUN g++ -std=c++17 -O3 -o prog prog.cpp algorithms.cpp 
 
FROM alpine:latest
RUN apk add --no-cache libstdc++ gcc
WORKDIR /app
COPY --from=builder /app/prog .
ENTRYPOINT ["./prog"]
 
