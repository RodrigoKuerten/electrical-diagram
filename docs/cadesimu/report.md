# Migração da biblioteca CADe_SIMU 4.0 — relatório técnico

## 1. Objetivo

Substituir a biblioteca desenhada à mão do FioVivo por símbolos com a **geometria exata** do
CADe_SIMU 4.0, preservando o editor, o simulador, os exemplos, a persistência `.dcj` e os testes
existentes. O alvo é o executável fornecido em
`C:\Users\rodri\Downloads\CADe_SIMU_4_0\CADe_SIMU_4_0\CADe_SIMU.exe`
(SHA-256 `5f7b56f8b01ea2cd472e9112c251496914a358b57e6de08d3a8111c80ba50fb9`). A senha do modo
limitado não foi contornada.

## 2. Método: captura GDI, não redesenho

A primeira tentativa reconstruiu os ícones manualmente em SVG. O resultado foi rejeitado porque
as proporções, inclinações e posições dos bornes não coincidiam com o original. A solução final
instrumenta o **GDI32 do próprio CADe_SIMU** com Frida e registra as primitivas reais de cada
inserção:

- `MoveToEx` / `LineTo` — traços, contatos, bobinas e terminais;
- `Ellipse` / `Rectangle` / `Polygon` — motores, fusíveis, caixas e símbolos pneumáticos;
- `TextOutA` — rótulos de borne e textos internos;
- cada comando é capturado com uma leitura **antes** e **depois** da inserção, para eliminar
  qualquer resíduo do símbolo anterior;
- a calibração do canvas é `672 px = 297 mm`, ou seja, `2,262626 px/mm`; os tamanhos e as
  posições dos bornes são convertidos diretamente para milímetros.

Ferramentas e evidências:

| Caminho | Conteúdo |
|---|---|
| `docs/cadesimu/gdi_clean/` | 228 traços GDI isolados (antes/depois) |
| `docs/cadesimu/gdi_screens/` | captura visual de cada inserção para conferência |
| `docs/cadesimu/gdi/` | primeira passada de traços e SVGs de apoio |
| `docs/cadesimu/type_to_cmd.json` | mapa tipo FioVivo → comando CADe |
| `docs/cadesimu/clean_contact_sheet.png` | folha de contato dos símbolos convertidos |
| `docs/cadesimu/validation_app2.png` | aplicativo real com a folha de validação |
| `docs/cadesimu/type_names.json` | IDs e nomes internos recuperados |
| `docs/cadesimu/decompiled/` | pseudocódigo Ghidra do dispatcher |

## 3. Integração

- **`tools/re_cadesimu/gdi_service.js`** — serviço Frida que registra as primitivas GDI.
- **`tools/re_cadesimu/gdi_capture_clean.py`** — orquestra limpeza, inserção e captura
  antes/depois de cada comando.
- **`tools/re_cadesimu/build_exact_assets.py`** — converte os traços em SVG, recalcula tamanho e
  bornes e atualiza os manifestos, mantendo IDs, categorias, parâmetros e estados existentes.
- **`tools/re_cadesimu/finalize_catalog.py`** — expõe somente símbolos convertidos de traços
  limpos; tipos não verificados ficam carregáveis, mas ocultos da biblioteca.
- **`assets/symbols/`** — 215 manifestos e 351 SVGs. Desses, **111 símbolos exatos** aparecem na
  biblioteca e no menu Inserir; os 20 tipos legados e os 84 não verificados ficam na categoria
  oculta `legacy`, preservando projetos e exemplos antigos.
- **`src/ui/component_item.cpp`** — o tag agora é desenhado à esquerda e centrado na vertical,
  como no CADe_SIMU, em vez de acima do símbolo.
- **`src/core/validation.cpp`** — bobinas com bornes `X1/X2` (além de `A1/A2`) são aceitas pelo
  validador por meio dos papéis `coil_input`/`coil_output`.
- **`src/sim/simulator.cpp`** — motores CC com bornes `A1/A2` ou `F1/F2` também são reconhecidos
  como cargas.

## 4. O que foi validado

- **154/154 testes CTest** passam no build Release MSYS2 UCRT64/Qt 6.
- Os 20 tipos legados continuam carregando os cinco exemplos e o golden de BOM.
- Todos os 111 símbolos visíveis têm:
  - SVG base e variantes de estado válidos;
  - bornes dentro do retângulo do componente;
  - cada borne coincidente com um vértice/terminal do desenho GDI (verificação automática,
    tolerância de 2,5 px).
- A folha de validação foi aberta no aplicativo real
  (`docs/cadesimu/validation.dcj` → `validation_app2.png`) com fusíveis, disjuntores, DR,
  contatores, contatos, bobinas, temporizador, botoeira, sensor, motor, porta lógica, bornes,
  fontes, diodo, resistor, inversor, cilindro pneumático, sinalização, display e pressostato.
- O tag à esquerda foi conferido na mesma captura.

## 5. Limitações declaradas

- **Símbolos não expostos**: GRAFCET, contadores, alguns blocos PLC/IO, display BCD e certos
  módulos eletrônicos tiveram traços incompletos ou não corresponderam ao comando esperado.
  Eles continuam carregáveis em documentos antigos, mas **não são oferecidos na biblioteca**
  para não reintroduzir desenhos aproximados.
- **Comportamento**: o modelo elétrico continua sendo presença/ausência de energia por grupos de
  fonte. Não há tensão, corrente, impedância, curto-circuito, aquecimento nem movimento
  mecânico. PLC, pneumática e GRAFCET são visuais; a bobina de válvula e os motores funcionam
  como cargas elétricas.
- **Texto**: o CADe_SIMU usa ClearType/GDI; o FioVivo usa texto vetorial Qt. A geometria dos
  traços e as posições dos bornes são as do GDI; a rasterização do texto pode variar
  subpixel.
- **Tipos legados**: mantêm os manifestos e os desenhos antigos, ocultos, para não quebrar
  exemplos/testes. A biblioteca nova não os exibe.

## 6. Conclusão

A biblioteca visível do FioVivo agora é composta por símbolos cuja geometria foi extraída dos
comandos GDI reais do CADe_SIMU 4.0, com tamanho e bornes na escala correta (`2,262626 px/mm`).
Não há mais ícones aproximados na biblioteca; os tipos não verificados foram ocultados. Os 154
testes passam e a folha de validação foi conferida no aplicativo real.
