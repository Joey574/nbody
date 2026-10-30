{
  pkgs ? import <nixpkgs> { },
}:

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
    glfw3
    glm
  ];

  shellHook = ''
    export NIX_ENFORCE_NO_NATIVE=0
    export LD_LIBRARY_PATH="${pkgs.lib.makeLibraryPath [ pkgs.vulkan-loader pkgs.wayland pkgs.libxkbcommon ]}:$LD_LIBRARY_PATH"
  '';
}
