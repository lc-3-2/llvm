{
  description = "LLVM Development and Build Environment";

  inputs = {
    nixpkgs-2211.url = "github:NixOS/nixpkgs/release-22.11";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs-2211, systems, flake-utils }@inputs :
    flake-utils.lib.eachSystem ["x86_64-linux"] (system:
    let
      name = "llvm-lc-3.2-dev";
      pkgs = nixpkgs-2211.legacyPackages.${system};
      inherit (pkgs) stdenv;

      buildInputs = [
        pkgs.python311
        pkgs.zlib
        pkgs.gnupg
      ];
      nativeBuildInputs = [
        pkgs.cmake
        pkgs.ninja
        pkgs.subversion
        pkgs.git
        pkgs.gdb
      ];
    in {

      packages = rec {
        default = lc-3-2;

        lc-3-2 = stdenv.mkDerivation {
          inherit name buildInputs nativeBuildInputs;

          src = self;

          configurePhase = ''
            cmake -G Ninja -B ./build/ -S ./llvm/ \
              -DCMAKE_INSTALL_PREFIX=$out \
              -DCMAKE_BUILD_TYPE=Release -DLLVM_ENABLE_ASSERTIONS=ON \
              -DLLVM_PARALLEL_LINK_JOBS=1 \
              -DLLVM_ENABLE_PROJECTS="clang;lld" -DLLVM_ENABLE_RUNTIMES="" \
              -DLLVM_TARGETS_TO_BUILD="X86;RISCV;MSP430;Lanai" \
              -DLLVM_EXPERIMENTAL_TARGETS_TO_BUILD="LC32"
          '';

          buildPhase = ''
            ninja -v -C ./build/
          '';

          installPhase = ''
            ninja -v -C ./build/ install
          '';
        };
      };

      devShells = rec {
        default = lc-3-2;

        lc-3-2 = pkgs.mkShell {
          inherit name buildInputs nativeBuildInputs;

          shellHook = ''
            export PS1="(${name}) [\u@\h \W]\$ "
          '';
        };
      };
    });
}
