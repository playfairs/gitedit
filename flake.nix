{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = { self, ... }@inputs:
  let
    systems = [
      "x86_64-linux"
      "aarch64-linux"
      "x86_64-darwin"
      "aarch64-darwin"
    ];

    forAllSystems = f:
      builtins.listToAttrs (map
        (system: {
          name = system;
          value = f system;
        })
        systems);
  in { 
    devShells = forAllSystems (system:
      let
        pkgs = import inputs.nixpkgs { inherit system; };
      in {
        default = pkgs.mkShell {
          nativeBuildInputs = with pkgs; [
            miniz
            ncurses
            pkg-config
          ];
          packages = with pkgs; [
            nasm
            gcc
            binutils
            gdb
            meson
            ninja
            cmake
            clang
            lld
          ];
        };
      });
      packages = forAllSystems (system:
        let
          pkgs = import inputs.nixpkgs { inherit system; };
        in
        {
          default = pkgs.stdenv.mkDerivation {
            pname = "dev";
            version = "0.1.0";
            src = self;

            enableParallelBuilding = true;
            nativeBuildInputs = with pkgs; [ 
              meson 
              ninja 
              miniz
              ncurses
              pkg-config
            ];
          };
        }
      );
  };
}