{
  description = "gitedit - Safe Git commit message editor";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
        
        gitedit = pkgs.stdenv.mkDerivation {
          pname = "gitedit";
          version = "1.0.0";
          
          src = ./.;
          
          nativeBuildInputs = with pkgs; [
            clang
          ];
          
          buildInputs = with pkgs; [];
          
          makeFlags = [
            "CC=clang"
            "PREFIX=$(out)"
          ];
          
          installPhase = ''
            mkdir -p $out/bin
            cp bin/gitedit $out/bin/
            chmod +x $out/bin/gitedit
          '';
          
          meta = with pkgs.lib; {
            description = "Safe Git commit message editor";
            longDescription = ''
              A robust, self-contained C program that allows safe editing of Git 
              commit messages at the raw object level. This tool provides an 
              interactive interface for modifying commit messages while maintaining 
              repository safety through automatic backups.
            '';
            homepage = "https://github.com/playfairs/gitedit";
            license = licenses.unlicense;
            platforms = platforms.linux ++ platforms.darwin;
            mainProgram = "gitedit";
          };
        };
        
        devShell = pkgs.mkShell {
          buildInputs = with pkgs; [
            clang
            gdb
            valgrind
          ];
          
          shellHook = ''
            echo "gitedit development environment"
            echo "Available commands:"
            echo "  make          - Build the project"
            echo "  make clean    - Clean build artifacts"
            echo "  make debug    - Build with debug flags"
            echo "  make install  - Install to system"
          '';
        };
        
      in {
        packages = {
          default = gitedit;
          gitedit = gitedit;
        };
        
        apps = {
          gitedit = flake-utils.lib.mkApp {
            drv = gitedit;
            program = "gitedit";
          };
          default = flake-utils.lib.mkApp {
            drv = gitedit;
            program = "gitedit";
          };
        };
        
        devShells.default = devShell;
      });
}