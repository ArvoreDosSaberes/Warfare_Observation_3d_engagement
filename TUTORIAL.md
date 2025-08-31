# Tutorial para Iniciantes

Bem-vindo! Este tutorial explica, passo a passo, como instalar, compilar e usar o aplicativo "Warfare Observation 3D Engagement".

Se você nunca usou CMake ou Raylib, siga as instruções com calma. Em poucos minutos você estará movendo a aeronave e o alvo em 3D e entendendo os ângulos mostrados no HUD.

---

## 1) O que este app faz?

Ele mostra uma cena 3D com:
- Uma aeronave (com orientação yaw, pitch, roll).
- Um alvo (uma esfera) que você pode mover.
- Um HUD (display 2D na tela) que indica onde o alvo aparece para o piloto.

Ele calcula e exibe:
- Azimute (Az) e Elevação (El) do alvo em relação à aeronave.
- Azimute/Elevação do eixo de rolagem (a direção do nariz da aeronave).
- Ângulos esféricos j, J, E, F, G usados para posicionar o símbolo do alvo no HUD.

Em resumo: você verá na prática como as posições 3D viram ângulos que o piloto enxerga no HUD.

---

## 2) Requisitos do sistema (Linux)

Instale os pacotes básicos e bibliotecas gráficas:

```bash
sudo apt update
sudo apt install -y build-essential cmake git \
  libx11-dev libxrandr-dev libxi-dev libxinerama-dev libxcursor-dev \
  mesa-common-dev libgl1-mesa-dev
```

- Se você já tem esses pacotes, pode pular este passo.
- O projeto baixa e compila a Raylib automaticamente (via CMake/FetchContent), então não precisa instalar Raylib separadamente.

---

## 3) Como compilar

No diretório do projeto, rode:

```bash
cmake -S . -B build
cmake --build build -j
```

Isso vai:
- Gerar os arquivos de build em `build/`.
- Baixar e compilar a Raylib (apenas no primeiro build).
- Construir o executável `build/woe3d`.

---

## 4) Como executar

```bash
./build/woe3d
```

Se a janela abrir, está tudo certo! Você deve ver a cena 3D, a aeronave, o alvo e um HUD ao centro.

---

## 5) Controles (teclado e mouse)

- Aeronave (mover no espaço):
  - I / K: mover no eixo Y (+Y / -Y)
  - J / L: mover no eixo X (-X / +X)
  - U / O: mover no eixo Z (+Z / -Z)

- Alvo (mover no espaço):
  - W / S: mover no eixo Y (+Y / -Y)
  - A / D: mover no eixo X (-X / +X)
  - Q / E: mover no eixo Z (+Z / -Z)

- Orientação da aeronave:
  - Setas Esquerda/Direita: Yaw (girar no plano horizontal)
  - Setas Cima/Baixo: Pitch (nariz para cima/baixo)
  - Z / X: Roll (rolagem para esquerda/direita)

- Câmera:
  - Botão direito do mouse + arrastar: orbita a câmera ao redor da aeronave

Dica: se “perder” a cena, orbite um pouco com o mouse e recoloque aeronave/alvo com as teclas.

---

## 6) O que significam os números na tela?

No canto superior esquerdo aparecem ângulos em graus:

- Linha 1: `AzT` (azimute do alvo), `ElT` (elevação do alvo), `AzR` e `ElR` (do eixo de rolagem).
- Linha 2: `j`, `J`, `E`, `F`, `G` (ângulos de triângulos esféricos usados para o HUD).

No HUD (o desenho 2D no meio da tela):
- A distância do ponto ao centro aumenta quando `j` aumenta.
- A direção onde o ponto aparece é dada por `G + Roll` (ou seja, a orientação do símbolo leva em conta a rolagem da aeronave).

Tradução simples:
- `j` pequeno → alvo perto do centro (alinhado ao nariz).
- `j` grande → alvo mais afastado do centro.
- Girar a aeronave em roll (Z/X) faz o símbolo girar em torno do centro, mantendo o raio.

---

## 7) Experimentos rápidos

- Alinhar alvo ao nariz (j ≈ 0):
  - Mova o alvo com WASD/QE até o ponto do HUD ir ao centro.

- Observar efeito do roll:
  - Pressione Z/X e note o símbolo girar em torno do centro.

- Testar azimute e elevação do alvo:
  - Use A/D e W/S para mudar AzT.
  - Use Q/E para mudar ElT (alvo mais alto/baixo).

---

## 8) Onde alterar configurações

Abra `src/main.c`:
- Velocidades de movimento/rotação: constantes `moveSpeed` e `rotSpeed`.
- Escala do HUD: constante `kpix` (pixels por radiano) determina o quão “espalhado” fica o HUD.
- Aparência: funções `DrawAircraft()` e os `DrawSphere()`/`DrawCylinderEx()`.

---

## 9) Problemas comuns

- Erro ao compilar pedindo X11/OpenGL:
  - Confirme os pacotes do passo 2 (instalação).

- Primeira compilação muito lenta:
  - Normal: a Raylib será baixada/compilada apenas uma vez.

- Janela preta ou sem renderização:
  - Atualize/instale drivers de vídeo (NVIDIA, Mesa/OpenGL) e tente novamente.

---

## 10) Próximos passos (opcional)

- Publicar no GitHub
  - `git init && git add . && git commit -m "Initial"`
  - `git remote add origin <URL do repositório>`
  - `git push -u origin main`

- Ideias de extensão
  - Múltiplos alvos, trilhas, gravação de telemetria, ajuste de FOV, modo de câmera “do cockpit”.

---

Pronto! Agora você já sabe instalar, compilar e usar o aplicativo. Divirta-se explorando os conceitos e, se quiser, contribua com melhorias!
