FROM gcc:14.2.0 AS builder

RUN apt-get update \
 && apt-get install -y cmake \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /tmp
RUN wget https://github.com/conan-io/conan/releases/download/2.15.1/conan-2.15.1-linux-x86_64.tgz \
 && tar -xvf conan-2.15.1-linux-x86_64.tgz \
 && mv bin/* /bin \
 && rm conan-2.15.1-linux-x86_64.tgz

RUN conan profile detect \
 && sed -i 's/compiler.cppstd=.*/compiler.cppstd=23/' ~/.conan2/profiles/default

WORKDIR /deps
RUN wget https://github.com/Makcal/TgBotStater/archive/refs/tags/v0.3.3.tar.gz \
 && tar -xvf v0.3.3.tar.gz \
 && rm v0.3.3.tar.gz \
 && cd TgBotStater-0.3.3 \
 && conan create . --build=missing

WORKDIR /app
COPY CMakeLists.txt conanfile.txt .
RUN conan install . -of build --build=missing

COPY . .
RUN cmake -B build --fresh --preset=conan-release \
 && cmake --build build


FROM gcc:14.2.0 AS runtime

WORKDIR /app
COPY --from=builder /app/build/main .

RUN useradd -m appuser \
 && chown appuser:appuser /app/main

USER appuser

CMD ["./main"]
