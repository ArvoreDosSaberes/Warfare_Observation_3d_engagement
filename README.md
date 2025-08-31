[![CI](https://github.com/ArvoreDosSaberes/Warfare_Observation_3d_Engagement/actions/workflows/ci.yml/badge.svg)](https://github.com/ArvoreDosSaberes/Warfare_Observation_3d_Engagement/actions/workflows/ci.yml)
![visitors](https://visitor-badge.laobi.icu/badge?page_id=ArvoreDosSaberes.Warfare_Observation_3d_Engagement)
[![License: CC BY-SA 4.0](https://img.shields.io/badge/License-CC_BY--SA_4.0-blue.svg)](https://creativecommons.org/licenses/by-sa/4.0/)
![Language: Portuguese](https://img.shields.io/badge/Language-Portuguese-brightgreen.svg)
[![Language-C](https://img.shields.io/badge/language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![CMake](https://img.shields.io/badge/build-CMake-informational.svg)](https://cmake.org/)
[![Raylib](https://img.shields.io/badge/graphics-raylib-2ea44f.svg)](https://www.raylib.com/)
[![Issues](https://img.shields.io/github/issues/ArvoreDosSaberes/Warfare_Observation_3d_Engagement.svg)](https://github.com/ArvoreDosSaberes/Warfare_Observation_3d_Engagement/issues)
[![Stars](https://img.shields.io/github/stars/ArvoreDosSaberes/Warfare_Observation_3d_Engagement.svg)](https://github.com/ArvoreDosSaberes/Warfare_Observation_3d_Engagement/stargazers)
[![Forks](https://img.shields.io/github/forks/ArvoreDosSaberes/Warfare_Observation_3d_Engagement.svg)](https://github.com/ArvoreDosSaberes/Warfare_Observation_3d_Engagement/network/members)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](https://makeapullrequest.com)
[![Watchers](https://img.shields.io/github/watchers/ArvoreDosSaberes/Warfare_Observation_3d_Engagement)](https://github.com/ArvoreDosSaberes/Warfare_Observation_3d_Engagement/watchers)
[![Last Commit](https://img.shields.io/github/last-commit/ArvoreDosSaberes/Warfare_Observation_3d_Engagement)](https://github.com/ArvoreDosSaberes/Warfare_Observation_3d_Engagement/commits)
[![Contributors](https://img.shields.io/github/contributors/ArvoreDosSaberes/Warfare_Observation_3d_Engagement)](https://github.com/ArvoreDosSaberes/Warfare_Observation_3d_Engagement/graphs/contributors)

# Warfare Observation 3D Engagement

![Demonstração do aplicativo](img/screenshot000.png)

Um demonstrador interativo em C (Raylib) para visualização 3D e HUD dos conceitos de Azimute/Elevação, triângulos esféricos e posicionamento do alvo relativo ao eixo de rolagem da aeronave.

 - Renderização 3D com Raylib
 - Cálculo de AzT, ElT, AzR, ElR e ângulos esféricos j, J, E, F, G
 - HUD 2D com posição radial por j e orientação por G + Roll
 - Controles de câmera e interação para mover aeronave e alvo

## Tutorial para iniciantes

Se você está começando agora, siga o passo a passo em: [TUTORIAL.md](TUTORIAL.md)

# Warfare Observation 3D Engagement

Um demonstrador interativo em C (Raylib) para visualização 3D e HUD dos conceitos de Azimute/Elevação, triângulos esféricos e posicionamento do alvo relativo ao eixo de rolagem da aeronave.

 - Renderização 3D com Raylib
 - Cálculo de AzT, ElT, AzR, ElR e ângulos esféricos j, J, E, F, G
 - HUD 2D com posição radial por j e orientação por G + Roll
 - Controles de câmera e interação para mover aeronave e alvo
- Cálculo de AzT, ElT, AzR, ElR e ângulos esféricos j, J, E, F, G
- HUD 2D com posição radial por j e orientação por G + Roll
- Controles de câmera e interação para mover aeronave e alvo

## Conceitos Implementados

- Azimute do alvo: `AzT = atan2(X_T - X_A, Y_T - Y_A)`
- Elevação do alvo: `ElT = atan2(Z_T - Z_A, sqrt((X_T - X_A)^2 + (Y_T - Y_A)^2))`
- Azimute/Elevação do eixo de rolagem (a partir do vetor frente da aeronave)
- Triângulos esféricos: cálculo de `f, h, C, D, J` e ângulo relativo `j`
- Ângulos auxiliares `E, F, G` e projeção no HUD: `HUD = (j, G + Roll)`

## Controles

- Aeronave (mover): I/K (±Y), J/L (±X), U/O (±Z)
- Alvo (mover): W/S (±Y), A/D (±X), Q/E (±Z)
- Orientação aeronave: Setas (Yaw/Pitch), Z/X (Roll)
- Câmera: Botão direito do mouse e arraste para orbitar

## Build

O projeto usa CMake e busca a dependência Raylib via FetchContent (clona do GitHub se não houver Raylib instalado no sistema).

Pré-requisitos no Linux (dependências do Raylib):
- `build-essential` `cmake` `git`
- Bibliotecas X11 e afins: `libx11-dev libxrandr-dev libxi-dev libxinerama-dev libxcursor-dev`
- `mesa-common-dev libgl1-mesa-dev` (OpenGL)

Passos:

```bash
cmake -S . -B build
cmake --build build -j
```

Executar:

```bash
./build/woe3d
```

## Estrutura

- `CMakeLists.txt`: configuração de build e Raylib
- `src/main.c`: renderização 3D, HUD e matemática esférica

## Licença

Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International:x. Veja o arquivo LICENSE (a ser adicionado no repositório destino).
