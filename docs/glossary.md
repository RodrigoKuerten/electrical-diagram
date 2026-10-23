# Glossary

This glossary lists Portuguese (Brazil) and English terms used in electrical control diagrams
and in FioVivo. It is a working vocabulary for the application, not a normative standard.

The Portuguese column follows common Brazilian practice in electrical controls, training and
panel documentation. Usage varies by region, manufacturer and standard; where European
Portuguese differs, the Brazilian form is listed. English equivalents are working
translations, not official IEC or NBR terms. The application uses English identifiers in code
and data; Portuguese appears in translated UI text and in the documentation only when quoted.

## Devices and control

| Portuguese (pt-BR) | English | Notes |
|---|---|---|
| contator | contactor | Electromagnetic switch for power circuits; has a coil and main/auxiliary contacts. Brazilian practice uses "contator"; European Portuguese often uses "contactor". |
| bobina | coil | Actuator of a contactor, relay or timer. In the simulator, a coil is a two-terminal load. |
| bobina de contator | contactor coil | The coil that picks up a contactor and changes its contacts. |
| contato NA (normalmente aberto) | normally open (NO) contact | Open at rest; closes when actuated. |
| contato NF (normalmente fechado) | normally closed (NC) contact | Closed at rest; opens when actuated. |
| contato auxiliar | auxiliary contact | Control contact of a contactor or relay, used in command logic. |
| contato principal | main contact | Power contact that carries the motor or load current in a real contactor. Current is not simulated. |
| contato de selo | seal-in contact | Auxiliary NO contact wired in parallel with a start button to maintain a coil after release. |
| botoeira | pushbutton | Momentary command device. Also called "botão de comando" in some contexts. |
| botão de emergência | emergency stop button | Latching NC command device used to open the control circuit. |
| chave seletora | selector switch | Common terminal switched to one of several positions; may be maintained or momentary. |
| chave fim de curso | limit switch | Switch actuated by mechanical travel. The simulator models its contact, not the mechanics. |
| relé térmico | thermal overload relay | Protection relay with 95/96 NC and 97/98 NO auxiliary contacts. In FioVivo, tripping is a user action; heating is not modelled. |
| relé auxiliar | auxiliary relay | Small relay used for command logic and contact multiplication. |
| relé de tempo / temporizador | timing relay / timer | Device that delays a contact action; FioVivo supports TON and TOF modes. |
| disjuntor | circuit breaker | Protection and switching device. In FioVivo it is a manually opened/closed pass-through; no trip curve is simulated. |
| fusível | fuse | Overcurrent protection device. In FioVivo it is intact or blown, controlled by the user; no current-based blowing. |
| chave seccionadora / seccionadora | disconnector | Device that isolates a circuit for maintenance; not a load-break device in this model. |
| chave geral | main switch | Main disconnecting switch of a panel or circuit. |
| contator de força | power contactor | Contactor intended for the main motor circuit. |
| contator auxiliar | auxiliary contactor | Contactor used mainly for command logic. |
| motor trifásico | three-phase motor | Motor with U1, V1 and W1 phase terminals. The simulator uses source groups, not real phase voltages. |
| sinaleiro | indicator lamp | Panel indicator that shows a circuit state. |
| lâmpada piloto | pilot lamp | Lamp used to indicate energized, running or fault state. |
| carga | load | Device that consumes energy, such as a coil, lamp or motor. In FioVivo a load does not conduct between its terminals. |
| fonte | source | Supply terminal or supply device that creates a source group. |
| fonte de alimentação | power supply | The supply that provides the circuit's source terminals, for example L1/L2/L3/N. |
| borne | terminal | Connection point for a wire. The application represents it as a pin. |
| borne de terra | earth terminal | Terminal connected to protective earth (PE). PE is never a return path in the simulator. |
| fio | wire | Ideal conductor between pins. FioVivo allows more than one wire per pin. |
| cabo | cable | Insulated conductor or assembly of conductors. Cable construction is not modelled. |
| condutor | conductor | Material path for current. In FioVivo, wires and closed pass-throughs are ideal conductors. |
| trilho DIN | DIN rail | Standard mounting rail inside panels. Not simulated; panel layout is out of scope. |

## Circuits, logic and starting methods

| Portuguese (pt-BR) | English | Notes |
|---|---|---|
| circuito de comando | control circuit | Circuit that operates coils, relays, timers and indicators. |
| circuito de força | power circuit | Main circuit that feeds the motor or load. FioVivo simulates only presence of energy. |
| selo | self-holding / seal-in | The practice of maintaining a coil through its own auxiliary contact after the start button is released. In FioVivo it emerges from the contact network; there is no special latch flag. |
| intertravamento | interlock | Prevention of an unsafe or conflicting state by contacts or logic. In FioVivo it is electrical only. |
| partida direta | direct-on-line (DOL) start | Starting a motor by connecting it directly to the supply through a contactor. |
| partida estrela-triângulo | star-delta starter | Starting method using star and delta contactors, a timer and interlocks. FioVivo models the contactor/timer logic, not the electrical transition. |
| partida suave | soft starter | Electronic starter that ramps voltage. Not simulated. |
| inversor de frequência | variable frequency drive (VFD) | Electronic motor drive. Not simulated. |
| parada de emergência | emergency stop | The function or action of stopping a machine in an emergency. The device is the "botão de emergência". |
| rearme | reset | Clearing a latch or returning a protection device to service. Also called "reset" in everyday Brazilian usage. |
| falta de fase | phase loss | Loss of one phase of a three-phase supply. FioVivo may show a motor stopping if the live phase count falls below `min_live_phases`; it has no dedicated phase-loss relay. |
| sobrecarga | overload | Current above the intended value. FioVivo does not calculate current or heating. |
| curto-circuito | short circuit | Low-impedance connection between points of different potential. FioVivo does not calculate short-circuit current. |
| sequência de fases | phase sequence | Order of the three phases. Not simulated; reversing phases does not reverse the motor in FioVivo. |
| comando | control | The command/logic part of an electrical system, or an actuation command. |
| manobra | switching operation | Operating a switch, contactor or disconnector. |
| retorno | return | The path from the load back to the source, commonly the neutral. A load needs a non-PE return group in FioVivo. |
| alimentação | supply | The source of electrical energy for the circuit. |

## Documentation and panels

| Portuguese (pt-BR) | English | Notes |
|---|---|---|
| diagrama | diagram | A drawing that represents an electrical circuit. |
| diagrama unifilar | single-line diagram | Simplified diagram using one line for a multi-conductor circuit. |
| diagrama multifilar | multi-line diagram | Diagram that shows individual conductors and connections. |
| esquema elétrico | electrical schematic | A drawing of the electrical connections and devices. |
| folha | sheet | One page of a diagram. FioVivo supports multiple sheets. |
| legenda | legend | Key that explains symbols, tags and line conventions on a drawing. |
| numeração de bornes | terminal numbering | Identification of terminals on devices and terminal blocks. |
| etiqueta | tag / label | Device identification such as KM1, Q1 or M1. |
| quadro elétrico | electrical panel | Enclosure that houses switches, contactors, relays and terminals. |
| painel de comando | control panel | Panel that contains the command devices and indicators. |
| projeto | project | The complete set of sheets, settings and data stored in a `.dcj` file. |

## Power, protection and installation

| Portuguese (pt-BR) | English | Notes |
|---|---|---|
| fase | phase | One conductor of a polyphase supply, commonly L1, L2 or L3. |
| neutro | neutral | The return conductor of a supply, commonly N. |
| terra | earth / ground | Protective earth, commonly PE. "Terra" is the common Brazilian term. |
| aterramento | earthing / grounding | The protective connection to earth. |
| corrente | current | Flow of electric charge. Not simulated. |
| tensão | voltage | Potential difference. Not simulated; source groups are identities, not voltage levels. |
| potência | power | Rate of energy transfer. Not calculated. |
| frequência | frequency | Supply frequency in hertz. Stored as metadata, not used by the boolean solver. |
| proteção | protection | Devices and practices that limit fault damage. FioVivo does not coordinate protection. |
| coordenação | coordination | Coordination between protective devices. Not simulated. |
| bitola | wire gauge / cross-section | Conductor size. Not simulated. |
| eletroduto | conduit | Conduit for conductors. Not simulated. |

## Simulation and application terms

| Portuguese (pt-BR) | English | Notes |
|---|---|---|
| grupo de etiqueta | label group | The `label_group` parameter that pairs a coil with its contacts. If absent, the component tag is used. |
| ID forte | strong ID | A type-safe identifier such as `ComponentId` or `PinId`. It is non-owning and cannot be used as an arbitrary integer. |
| grupo de fonte | source group | The simulator's identity for a supply conductor. Distinct source groups make a load energized; they are not voltage levels. |
| elemento de passagem | pass-through element | A closed contact, selector path, intact fuse or closed breaker that connects its terminals. |
| ciclo de varredura | scan cycle | One execution of the simulation loop, from input snapshot through state commit. |
| tempo simulado | simulated time | Time derived from the scan index and `cycle_ms`, not from the wall clock. |
| instantâneo | snapshot | An immutable copy of runtime or document state. Document snapshots are planned for undo; simulation snapshots are planned for debugging. |
| desfazer | undo | Restore the document to an earlier snapshot. |
| refazer | redo | Reapply a document snapshot that was undone. |
| biblioteca de símbolos | symbol library | The SVG artwork and JSON manifests that define symbols and pins. |
| painel de problemas | problems panel | UI panel that lists validation problems; selecting a problem navigates to the affected object. |
| registro de simulação | simulation log | Ordered event stream produced by the simulator and displayed by the UI. |
| energizado | energized | A load terminal or device is connected to a source group in a way that produces energization. |
| desenergizado | de-energized | Not connected to a complete source-to-return path. |
| aberto | open | A contact or switching device that does not conduct. |
| fechado | closed | A contact or switching device that conducts. |
| falha | fault | An invalid state, a non-convergent circuit or a device condition that prevents normal operation. |
| alarme | alarm | A UI indication of an abnormal condition; not a safety function. |
| partida | start | The act of starting a motor or beginning a simulation session. |
| parada | stop | The act of stopping a motor or ending a simulation session. |

## Usage notes

- "Contator" is the usual Brazilian spelling; "contactor" is common in Portugal and in
  English-influenced literature.
- "Selo" literally means seal. In command diagrams it means the self-holding path, not a
  mechanical seal.
- "Botoeira" commonly refers to the complete pushbutton command device, while "botão" refers
  to the button itself. Both appear in panel documentation.
- "Relé térmico" is often shortened to "térmico" in shop-floor usage. The full term is used in
  documentation.
- "Terra" and "aterramento" are the usual Brazilian terms; "massa" appears in some automotive
  and electronic contexts but is not used for panel earthing here.
- "Rearme" is the act of resetting a latch or protection device. The English "reset" is also
  widely used in Brazilian technical speech.
- The application's symbol conventions are inspired by common NBR 5280-style command diagrams
  and IEC 60617-style graphics. This glossary does not claim compliance with either standard.
