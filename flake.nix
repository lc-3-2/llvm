{
  description = "LLVM Development and Build Environment";

  inputs = {
    nixpkgs-2211.url = "github:NixOS/nixpkgs/release-22.11";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs-2211, systems, flake-utils }@inputs :
    flake-utils.lib.eachSystem ["x86_64-linux"] (system:
    let
      name = "llvm-lc-3.2";

      pkgs = nixpkgs-2211.legacyPackages.${system};
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

      devShell = pkgs.mkShell {
        inherit name buildInputs nativeBuildInputs;
        shellHook = ''
          export PS1="(${name}) [\u@\h \W]\$ "
        '';
      };
    });
}
