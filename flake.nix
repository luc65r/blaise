{
  description = "A very basic flake";

  inputs.flake-utils.url = "github:numtide/flake-utils";

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system: let
      pkgs = nixpkgs.legacyPackages.${system};
    in {
      devShell = pkgs.mkShell {
        nativeBuildInputs = with pkgs; [
          gdb
          meson
          ninja
          flex
          pkg-config
          bison
          libunistring
          gmp
          jansson
          valgrind
          ccls
        ];
      };
    });
}
