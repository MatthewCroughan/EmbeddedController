{
  description = "An over-engineered Hello World in C";

  # Nixpkgs / NixOS version to use.
  inputs.nixpkgs.url = "github:nixos/nixpkgs/master";

  outputs = { self, nixpkgs }:
    let

      # to work with older version of flakes
      lastModifiedDate = self.lastModifiedDate or self.lastModified or "19700101";

      # Generate a user-friendly version number.
      version = builtins.substring 0 8 lastModifiedDate;

      # System types to support.
      supportedSystems = [ "x86_64-linux" "x86_64-darwin" "aarch64-linux" "aarch64-darwin" ];

      # Helper function to generate an attrset '{ x86_64-linux = f "x86_64-linux"; ... }'.
      forAllSystems = nixpkgs.lib.genAttrs supportedSystems;

      # Nixpkgs instantiated for supported system types.
      nixpkgsFor = forAllSystems (system: import nixpkgs { inherit system; overlays = [ self.overlay ]; });

    in

    {

      # A Nixpkgs overlay.
      overlay = final: prev: {

        img = with final; stdenv.mkDerivation rec {
          name = "img-${version}";

          src = ./.;

          nativeBuildInputs = [ gcc-arm-embedded libftdi1 ];

          buildPhase = ''
            ${gnumake}/bin/make BOARD=hx20 CROSS_COMPILE=arm-none-eabi-
          '';
        };

        ectool = with final; stdenv.mkDerivation rec {
          name = "ectool${version}";

          postPatch = ''
            patchShebangs ./util
          '';

          src = ./.;

          makeFlags = [
            "INSTALL=install"
            "PREFIX=${placeholder "out"}"
          ];

          buildPhase = ''
            make utils-host
          '';

          installPhase = "true";

          nativeBuildInputs = [ pkg-config libusb1 libftdi1 hostname ];
        };

      };

      # Provide some binary packages for selected system types.
      packages = forAllSystems (system:
        {
          inherit (nixpkgsFor.${system}) img ectool;
        });

      # The default package for 'nix build'. This makes sense if the
      # flake provides only one package or there is a clear "main"
      # package.
      defaultPackage = forAllSystems (system: self.packages.${system}.img);

    };
}
