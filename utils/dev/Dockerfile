#===- utils/dev/Dockerfile ------------------------------------------------===//
#
# Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
#===-----------------------------------------------------------------------===//
#
# Copied from the first part of the normal Dockerfile. It sets up the
# environment, then stops before building. It also installs GDB since it's
# useful to have for debugging.
#
# See: /llvm/utils/docker/debian10/Dockerfile
# See: /docker-compose.yml
#
#===-----------------------------------------------------------------------===//

# Stage 1. Check out LLVM source code and run the build.
FROM launcher.gcr.io/google/debian10:latest as builder
# Install build dependencies of llvm.
# First, Update the apt's source list and include the sources of the packages.
RUN grep deb /etc/apt/sources.list | \
    sed 's/^deb/deb-src /g' >> /etc/apt/sources.list
# Install compiler, python and subversion.
RUN apt-get update && \
    apt-get install -y --no-install-recommends ca-certificates gnupg \
           build-essential cmake make python3 zlib1g wget subversion unzip git && \
    rm -rf /var/lib/apt/lists/*
# Install a newer ninja release. It seems the older version in the debian repos
# randomly crashes when compiling llvm.
RUN wget "https://github.com/ninja-build/ninja/releases/download/v1.8.2/ninja-linux.zip" && \
    echo "d2fea9ff33b3ef353161ed906f260d565ca55b8ca0568fa07b1d2cab90a84a07 ninja-linux.zip" \
        | sha256sum -c  && \
    unzip ninja-linux.zip -d /usr/local/bin && \
    rm ninja-linux.zip

RUN apt-get update && \
    apt-get install -y gdb && \
    rm -rf /var/lib/apt/lists/*
