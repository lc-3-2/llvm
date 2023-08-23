{
  description = "LLVM Development and Build Environment";

  inputs = {
    nixpkgs-23-05.url = "github:NixOS/nixpkgs/release-23.05";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs-23-05, flake-utils }@inputs :
    flake-utils.lib.eachSystem ["x86_64-linux"] (system:
    let
      name = "llvm-lc-3.2-dev";
      pkgs = nixpkgs-23-05.legacyPackages.${system};
      inherit (pkgs) stdenv;

      # See: https://github.com/NixOS/nixpkgs/tree/4ecab3273592f27479a583fb6d975d4aba3486fe/pkgs/development/compilers/llvm/16
      buildInputs = [
        pkgs.libxml2
        pkgs.libffi
      ];
      nativeBuildInputs = [
        pkgs.cmake
        pkgs.ninja
        pkgs.python311
        pkgs.git
      ];
      propagatedBuildInputs = [
        pkgs.ncurses
        pkgs.zlib
      ];
    in {

      packages = rec {
        default = llvm-lc-3-2;

        llvm-lc-3-2 = stdenv.mkDerivation {
          inherit name buildInputs nativeBuildInputs propagatedBuildInputs;
          src = self;
          cmakeDir = "${self}/llvm/";
          cmakeFlags = [
            "-DLLVM_ENABLE_ASSERTIONS=ON"
            "-DLLVM_PARALLEL_LINK_JOBS=1"
            "-DLLVM_ENABLE_PROJECTS=clang;lld"
            "-DLLVM_ENABLE_RUNTIMES=compiler-rt"
            "-DLLVM_TARGETS_TO_BUILD="
            "-DLLVM_EXPERIMENTAL_TARGETS_TO_BUILD=LC32"
            "-DLLVM_DEFAULT_TARGET_TRIPLE=lc_3.2-unknown-unknown"
            "-DCOMPILER_RT_BAREMETAL_BUILD=ON"
            "-DCOMPILER_RT_EXCLUDE_PERSONALITY=ON"
            "-DCOMPILER_RT_BUILTINS_PASS_FUNCTION_SECTIONS=ON"
            "-DCOMPILER_RT_BUILTINS_PASS_DATA_SECTIONS=ON"
            "-DCOMPILER_RT_BUILTINS_PASS_G=ON"
          ];
        };
      };

      devShells = rec {
        default = llvm-lc-3-2;

        llvm-lc-3-2 = pkgs.mkShell {
          inherit name buildInputs nativeBuildInputs propagatedBuildInputs;
          shellHook = ''
            export PS1="(${name}) [\u@\h \W]\$ "
          '';
        };
      };
    });
}
