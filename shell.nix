{
  pkgs ? import <nixpkgs> { },
}:

let
  ompHeader = pkgs.runCommand "omp-header" { } ''
    mkdir -p $out/include
    cp ${pkgs.stdenv.cc.cc}/lib/gcc/*/*/include/omp.h $out/include/
  '';
in
pkgs.mkShell {
  nativeBuildInputs = with pkgs; [
    bc
    cmake
    ninja
    pkg-config
    gcc
    clang-tools
  ];

  buildInputs = with pkgs; [
    shader-slang
    yaml-cpp
    vulkan-headers
    vulkan-loader
    vulkan-validation-layers
    glfw3
    glm
    (python3.withPackages (
      ps: with ps; [
        matplotlib
      ]
    ))
  ];

  shellHook = ''
    export NIX_ENFORCE_NO_NATIVE=0
    export CPLUS_INCLUDE_PATH="${ompHeader}/include''${CPLUS_INCLUDE_PATH:+:$CPLUS_INCLUDE_PATH}"
    export LD_LIBRARY_PATH="${pkgs.lib.makeLibraryPath [ pkgs.vulkan-loader pkgs.wayland pkgs.libxkbcommon ]}:$LD_LIBRARY_PATH"
    export VK_LAYER_PATH="${pkgs.vulkan-validation-layers}/share/vulkan/explicit_layer.d"
  '';
}
