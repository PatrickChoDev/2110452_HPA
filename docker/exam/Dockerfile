FROM ubuntu:22.04 AS builder

ARG TARGETPLATFORM
ARG BUILDPLATFORM

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libglpk-dev \
    libomp-dev \
    libcolamd2 \
    --no-install-recommends \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build
COPY main.cpp .

RUN g++ -O2 -march=native -fopenmp -funroll-loops \
    -I/usr/include/glpk \
    main.cpp \
    -o power_grid \
    -lglpk \
    -static-libstdc++ \
    -static-libgcc

FROM gcr.io/distroless/cc-debian12 AS runtime

COPY --from=builder /build/power_grid /
COPY --from=builder /usr/lib/*/libglpk.so.40 /usr/lib/
COPY --from=builder /usr/lib/*/libomp.so.5 /usr/lib/
COPY --from=builder /usr/lib/*/libcolamd.so.2 /usr/lib/
COPY --from=builder /usr/lib/*/libamd.so.2 /usr/lib/
COPY --from=builder /usr/lib/*/libcamd.so.2 /usr/lib/
COPY --from=builder /usr/lib/*/libz.so.1 /usr/lib/
COPY --from=builder /usr/lib/*/libltdl.so.7 /usr/lib/
COPY --from=builder /usr/lib/*/libgmp.so.10 /usr/lib/
COPY --from=builder /usr/lib/*/libsuitesparseconfig.so.5 /usr/lib/

ENTRYPOINT ["/power_grid"]
