FROM alpine:3.22.4 AS build

RUN apk add --no-cache \
    build-base \
    cmake \
    ninja \
    sqlite-dev \
    asio-dev

WORKDIR /app
COPY 3rdparty ./3rdparty
COPY cmake    ./cmake
COPY src      ./src
COPY cmd      ./cmd
COPY CMakeLists.txt .

RUN cmake -B build -G Ninja \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_STANDARD=23 \
  && cmake --build build

FROM alpine:3.22.4 AS runtime

RUN apk add --no-cache sqlite-libs libstdc++ libgcc

WORKDIR /app

COPY --from=build /app/build/checkmate          /usr/local/bin/checkmate
COPY --from=build /app/build/checkmate-create_user /usr/local/bin/checkmate-create_user
COPY src/checkmate ./src/checkmate

RUN mkdir -p /app/db

ENV DATABASE_URL=sqlite:/app/db/checkmate.db

EXPOSE 5571

CMD ["/usr/local/bin/checkmate"]
