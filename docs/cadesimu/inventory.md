# CADe_SIMU component inventory

Generated from the reconstructed catalog: **215 component definitions**.

> **Current library status:** the library exposes **111 symbols** whose geometry was recorded
> from clean GDI traces of CADe_SIMU 4.0. The remaining definitions (including the 20 original
> FioVivo legacy types) remain loadable for old documents but are hidden from the library under
> the internal `legacy` category. See `report.md` for the exact-geometry method and limitations.

## Counts by category

| Category | Types | Electrical behaviour |
|---|---:|---|
| annotations | 1 | Text label |
| contactors | 11 | Coil plus group-controlled main/auxiliary contacts |
| contacts | 8 | NO/NC/CO contacts; timed and thermal variants follow their group |
| control | 19 | Momentary or maintained digital inputs; selector positions |
| grafcet | 7 | Visual GRAFCET step/transition/action elements; no token engine |
| indication | 7 | Load energized from two distinct source groups |
| logic | 13 | Boolean gate with a switched output source |
| machines | 13 | Load; three-phase machines require the configured live phases |
| measurement | 4 | Visual instrument; high impedance, no measurement value |
| plc | 10 | Visual I/O block; no user PLC program is executed |
| pneumatic | 13 | Solenoid coil loads and visual pneumatic symbols; no pressure model |
| power | 13 | Pass-through, source or visual power stage |
| protection | 30 | Pass-through poles; trip/blown opens the protected path |
| relays | 10 | Coil plus group-controlled changeover contacts |
| sensors | 21 | Digital input (NO/NC) driven by the user or a linked actuator |
| supplies | 14 | Source groups (L/N/PE, AC/DC, transformer/rectifier) |
| terminals | 13 | Connection points, terminals, cables and junctions |
| timers | 8 | TON, TOF, ON/OFF, pulse and flasher timing |

## Catalog

| Type | English name | Portuguese name | Category | Pins | States | Behaviour |
|---|---|---|---|---:|---|---|
| `supply_ac1` | Supply L + N + PE | Alimentação F + N + PE | supplies | 3 | — | Source groups (L/N/PE, AC/DC, transformer/rectifier) |
| `supply_ac1_pe` | Supply L + N + PE | Alimentação F + N + PE | supplies | 3 | — | Source groups (L/N/PE, AC/DC, transformer/rectifier) |
| `supply_ac3` | Supply L1 + L2 + L3 + N + PE | Alimentação L1 + L2 + L3 + N + PE | supplies | 5 | — | Source groups (L/N/PE, AC/DC, transformer/rectifier) |
| `supply_ac3_n` | Supply L1+L2+L3+N | Alimentação L1+L2+L3+N | supplies | 4 | — | Source groups (L/N/PE, AC/DC, transformer/rectifier) |
| `supply_ac3_n_pe` | Supply L1+L2+L3+N+PE | Alimentação L1+L2+L3+N+PE | supplies | 5 | — | Source groups (L/N/PE, AC/DC, transformer/rectifier) |
| `supply_ac3_pe` | Supply L1+L2+L3+PE | Alimentação L1+L2+L3+PE | supplies | 4 | — | Source groups (L/N/PE, AC/DC, transformer/rectifier) |
| `supply_dc` | Supply + / - | Alimentação + / - | supplies | 2 | — | Source groups (L/N/PE, AC/DC, transformer/rectifier) |
| `supply_dc_pos` | Supply + | Alimentação + | supplies | 1 | — | Source groups (L/N/PE, AC/DC, transformer/rectifier) |
| `supply_neutral` | Supply N | Alimentação neutro | supplies | 1 | — | Source groups (L/N/PE, AC/DC, transformer/rectifier) |
| `supply_earth` | Supply PE | Alimentação proteção | supplies | 1 | — | Source groups (L/N/PE, AC/DC, transformer/rectifier) |
| `supply_battery` | Battery | Bateria | supplies | 2 | — | Source groups (L/N/PE, AC/DC, transformer/rectifier) |
| `supply_rectifier` | Rectifier | Retificador | supplies | 4 | — | Source groups (L/N/PE, AC/DC, transformer/rectifier) |
| `supply_transformer3` | Transformer III | Transformador III | supplies | 6 | — | Source groups (L/N/PE, AC/DC, transformer/rectifier) |
| `fuse_1p` | Fuse I | Fusível I | protection | 2 | — | Pass-through poles; trip/blown opens the protected path |
| `fuse_2p` | Fuse II | Fusível II | protection | 4 | — | Pass-through poles; trip/blown opens the protected path |
| `fuse_3p` | Fuse III | Fusível III | protection | 6 | — | Pass-through poles; trip/blown opens the protected path |
| `fuse_3pn` | Fuse IIIN | Fusível IIIN | protection | 8 | — | Pass-through poles; trip/blown opens the protected path |
| `fuse_switch_1pn` | Fuse switch IN | Fusível seccionável IN | protection | 4 | — | Pass-through poles; trip/blown opens the protected path |
| `fuse_switch_2p` | Fuse switch II | Fusível seccionável II | protection | 4 | — | Pass-through poles; trip/blown opens the protected path |
| `fuse_switch_3p` | Fuse switch III | Fusível seccionável III | protection | 6 | — | Pass-through poles; trip/blown opens the protected path |
| `breaker_1p` | Breaker I | Disjuntor I | protection | 2 | — | Pass-through poles; trip/blown opens the protected path |
| `breaker_1pn` | Breaker IN | Disjuntor IN | protection | 4 | — | Pass-through poles; trip/blown opens the protected path |
| `breaker_2p` | Breaker II | Disjuntor II | protection | 4 | — | Pass-through poles; trip/blown opens the protected path |
| `breaker_3p` | Breaker III | Disjuntor III | protection | 6 | — | Pass-through poles; trip/blown opens the protected path |
| `breaker_3pn` | Breaker IIIN | Disjuntor IIIN | protection | 8 | — | Pass-through poles; trip/blown opens the protected path |
| `rcd_2p` | Differential switch IN | Interruptor diferencial IN | protection | 4 | — | Pass-through poles; trip/blown opens the protected path |
| `rcd_4p` | Differential switch IIIN | Interruptor diferencial IIIN | protection | 8 | — | Pass-through poles; trip/blown opens the protected path |
| `motor_breaker_3p` | Motor circuit breaker III | Disjuntor-motor III | protection | 6 | — | Pass-through poles; trip/blown opens the protected path |
| `surge_1pn` | Surge arrester IN | Limitador sobretensões IN | protection | 3 | — | Pass-through poles; trip/blown opens the protected path |
| `surge_3pn` | Surge arrester IIIN | Limitador sobretensões IIIN | protection | 5 | — | Pass-through poles; trip/blown opens the protected path |
| `overvoltage_relay` | Permanent overvoltage protection | Proteção sobretensões permanentes | protection | 4 | — | Pass-through poles; trip/blown opens the protected path |
| `contactor_1p` | Contactor I | Contator I | contactors | 2 | — | Coil plus group-controlled main/auxiliary contacts |
| `contactor_2p` | Contactor II | Contator II | contactors | 4 | — | Coil plus group-controlled main/auxiliary contacts |
| `contactor_3p` | Contactor III | Contator III | contactors | 6 | — | Coil plus group-controlled main/auxiliary contacts |
| `contactor_4p` | Contactor IIII | Contator IIII | contactors | 8 | — | Coil plus group-controlled main/auxiliary contacts |
| `contactor_aux_no` | Auxiliary contact NO | Contato auxiliar NA | contactors | 2 | — | Coil plus group-controlled main/auxiliary contacts |
| `contactor_aux_nc` | Auxiliary contact NC | Contato auxiliar NF | contactors | 2 | — | Coil plus group-controlled main/auxiliary contacts |
| `contactor_aux_co` | Auxiliary changeover contact | Contato auxiliar comutado | contactors | 3 | — | Coil plus group-controlled main/auxiliary contacts |
| `contactor_coil` | Contactor coil | Bobina de contator | contactors | 2 | — | Coil plus group-controlled main/auxiliary contacts |
| `contactor_coil_bistable` | Bistable contactor coil | Bobina biestável de contator | contactors | 4 | — | Coil plus group-controlled main/auxiliary contacts |
| `relay_coil` | Relay coil | Bobina de relé | relays | 2 | — | Coil plus group-controlled changeover contacts |
| `relay_coil_bistable` | Bistable relay coil | Bobina biestável de relé | relays | 4 | — | Coil plus group-controlled changeover contacts |
| `relay_contact_no` | Relay contact NO | Contato de relé NA | relays | 2 | — | Coil plus group-controlled changeover contacts |
| `relay_contact_nc` | Relay contact NC | Contato de relé NF | relays | 2 | — | Coil plus group-controlled changeover contacts |
| `relay_contact_co` | Relay changeover contact | Contato de relé comutado | relays | 3 | — | Coil plus group-controlled changeover contacts |
| `solid_state_relay_dc` | Solid-state relay DC | Relé de estado sólido DC | relays | 4 | — | Coil plus group-controlled changeover contacts |
| `solid_state_relay_ac` | Solid-state relay AC | Relé de estado sólido AC | relays | 4 | — | Coil plus group-controlled changeover contacts |
| `solid_state_contactor` | Solid-state contactor | Contator de estado sólido | relays | 6 | — | Coil plus group-controlled changeover contacts |
| `contact_no` | Contact NO | Contato NA | contacts | 2 | — | NO/NC/CO contacts; timed and thermal variants follow their group |
| `contact_nc` | Contact NC | Contato NF | contacts | 2 | — | NO/NC/CO contacts; timed and thermal variants follow their group |
| `contact_co` | Changeover contact | Contato comutado | contacts | 3 | — | NO/NC/CO contacts; timed and thermal variants follow their group |
| `pushbutton_no` | Pushbutton NO | Botão pulsador NA | control | 2 | — | Momentary or maintained digital inputs; selector positions |
| `pushbutton_nc` | Pushbutton NC | Botão pulsador NF | control | 2 | — | Momentary or maintained digital inputs; selector positions |
| `pushbutton_co` | Pushbutton changeover | Botão pulsador comutado | control | 3 | — | Momentary or maintained digital inputs; selector positions |
| `mushroom_no` | Mushroom pushbutton NO | Pulsador seta NA | control | 2 | — | Momentary or maintained digital inputs; selector positions |
| `mushroom_nc` | Mushroom pushbutton NC | Pulsador seta NF | control | 2 | — | Momentary or maintained digital inputs; selector positions |
| `mushroom_co` | Mushroom changeover pushbutton | Pulsador seta comutado | control | 3 | — | Momentary or maintained digital inputs; selector positions |
| `emergency_stop` | Emergency stop | Botão de emergência | control | 2 | — | Momentary or maintained digital inputs; selector positions |
| `switch_no` | Switch NO | Interruptor NA | control | 2 | — | Momentary or maintained digital inputs; selector positions |
| `switch_nc` | Switch NC | Interruptor NF | control | 2 | — | Momentary or maintained digital inputs; selector positions |
| `switch_co` | Changeover switch | Interruptor comutado | control | 3 | — | Momentary or maintained digital inputs; selector positions |
| `limit_no` | Limit switch NO | Fim de curso NA | sensors | 2 | — | Digital input (NO/NC) driven by the user or a linked actuator |
| `limit_nc` | Limit switch NC | Fim de curso NF | sensors | 2 | — | Digital input (NO/NC) driven by the user or a linked actuator |
| `limit_co` | Changeover limit switch | Fim de curso comutado | sensors | 3 | — | Digital input (NO/NC) driven by the user or a linked actuator |
| `selector_2pos` | Selector I-0-II | Comutador I-0-II | control | 3 | — | Momentary or maintained digital inputs; selector positions |
| `selector_3pos` | Selector I-II-III | Comutador I-II-III | control | 4 | — | Momentary or maintained digital inputs; selector positions |
| `selector_changeover` | Changeover selector | Comutador de cruzamento | control | 4 | — | Momentary or maintained digital inputs; selector positions |
| `contact_timed_on_no` | Timed contact ON delay NO | Contato temporizado conexão NA | contacts | 2 | — | NO/NC/CO contacts; timed and thermal variants follow their group |
| `contact_timed_off_no` | Timed contact OFF delay NO | Contato temporizado desconexão NA | contacts | 2 | — | NO/NC/CO contacts; timed and thermal variants follow their group |
| `contact_timed_on_nc` | Timed contact ON delay NC | Contato temporizado conexão NF | contacts | 2 | — | NO/NC/CO contacts; timed and thermal variants follow their group |
| `contact_timed_off_nc` | Timed contact OFF delay NC | Contato temporizado desconexão NF | contacts | 2 | — | NO/NC/CO contacts; timed and thermal variants follow their group |
| `contact_timed_onoff_no` | Timed contact ON/OFF NO | Contato temporizado conexão/desconexão NA | contacts | 2 | — | NO/NC/CO contacts; timed and thermal variants follow their group |
| `contact_thermal_no` | Thermal relay contact NO | Contato auxiliar térmico NA | protection | 2 | — | Pass-through poles; trip/blown opens the protected path |
| `contact_thermal_nc` | Thermal relay contact NC | Contato auxiliar térmico NF | protection | 2 | — | Pass-through poles; trip/blown opens the protected path |
| `contact_thermal_co` | Thermal relay changeover contact | Contato auxiliar térmico comutado | protection | 3 | — | Pass-through poles; trip/blown opens the protected path |
| `sensor_inductive_no` | Inductive sensor NA | Sensor indutivo NA | sensors | 3 | — | Digital input (NO/NC) driven by the user or a linked actuator |
| `sensor_inductive_nc` | Inductive sensor NF | Sensor indutivo NF | sensors | 3 | — | Digital input (NO/NC) driven by the user or a linked actuator |
| `sensor_capacitive_no` | Capacitive sensor NA | Sensor capacitivo NA | sensors | 3 | — | Digital input (NO/NC) driven by the user or a linked actuator |
| `sensor_capacitive_nc` | Capacitive sensor NF | Sensor capacitivo NF | sensors | 3 | — | Digital input (NO/NC) driven by the user or a linked actuator |
| `sensor_magnetic_no` | Magnetic sensor NA | Sensor magnético NA | sensors | 3 | — | Digital input (NO/NC) driven by the user or a linked actuator |
| `sensor_magnetic_nc` | Magnetic sensor NF | Sensor magnético NF | sensors | 3 | — | Digital input (NO/NC) driven by the user or a linked actuator |
| `sensor_photoelectric_no` | Photoelectric sensor NA | Sensor fotoelétrico NA | sensors | 3 | — | Digital input (NO/NC) driven by the user or a linked actuator |
| `sensor_photoelectric_nc` | Photoelectric sensor NF | Sensor fotoelétrico NF | sensors | 3 | — | Digital input (NO/NC) driven by the user or a linked actuator |
| `pressure_switch_no` | Pressure switch NO | Presostato NA | sensors | 2 | — | Digital input (NO/NC) driven by the user or a linked actuator |
| `vacuum_switch_no` | Vacuum switch NO | Vacuostato NA | sensors | 2 | — | Digital input (NO/NC) driven by the user or a linked actuator |
| `pressure_switch_nc` | Pressure switch NC | Presostato NF | sensors | 2 | — | Digital input (NO/NC) driven by the user or a linked actuator |
| `vacuum_switch_nc` | Vacuum switch NC | Vacuostato NF | sensors | 2 | — | Digital input (NO/NC) driven by the user or a linked actuator |
| `thermostat_no` | Thermostat NO | Termostato NA | sensors | 2 | — | Digital input (NO/NC) driven by the user or a linked actuator |
| `thermostat_nc` | Thermostat NC | Termostato NF | sensors | 2 | — | Digital input (NO/NC) driven by the user or a linked actuator |
| `float_no` | Float switch NO | Boia NA | sensors | 2 | — | Digital input (NO/NC) driven by the user or a linked actuator |
| `float_nc` | Float switch NC | Boia NF | sensors | 2 | — | Digital input (NO/NC) driven by the user or a linked actuator |
| `reed_no` | Reed switch NO | Sensor reed NA | sensors | 2 | — | Digital input (NO/NC) driven by the user or a linked actuator |
| `reed_nc` | Reed switch NC | Sensor reed NF | sensors | 2 | — | Digital input (NO/NC) driven by the user or a linked actuator |
| `timer_on` | Timer ON delay | Temporizador na conexão | timers | 4 | — | TON, TOF, ON/OFF, pulse and flasher timing |
| `timer_off` | Timer OFF delay | Temporizador na desconexão | timers | 4 | — | TON, TOF, ON/OFF, pulse and flasher timing |
| `timer_on_off` | Timer ON/OFF | Temporizador conexão/desconexão | timers | 5 | — | TON, TOF, ON/OFF, pulse and flasher timing |
| `timer_pulse` | Pulse timer | Temporizador de pulso | timers | 4 | — | TON, TOF, ON/OFF, pulse and flasher timing |
| `flasher` | Flasher relay | Relé intermitente | timers | 4 | — | TON, TOF, ON/OFF, pulse and flasher timing |
| `time_switch` | Time switch | Interruptor horário | timers | 3 | — | TON, TOF, ON/OFF, pulse and flasher timing |
| `counter` | Counter | Contador | timers | 4 | — | TON, TOF, ON/OFF, pulse and flasher timing |
| `counter_updown` | Up/down counter | Contador ascendente-descendente | timers | 6 | — | TON, TOF, ON/OFF, pulse and flasher timing |
| `pilot_lamp` | Pilot lamp | Piloto de sinalização | indication | 2 | — | Load energized from two distinct source groups |
| `lamp_flashing` | Flashing pilot lamp | Piloto intermitente | indication | 2 | — | Load energized from two distinct source groups |
| `buzzer` | Buzzer | Zumbador | indication | 2 | — | Load energized from two distinct source groups |
| `siren` | Siren | Sirene | indication | 2 | — | Load energized from two distinct source groups |
| `bell` | Bell | Campainha | indication | 2 | — | Load energized from two distinct source groups |
| `led` | LED | Diodo LED | indication | 2 | — | Load energized from two distinct source groups |
| `motor_3ph` | Three-phase motor | Motor trifásico | machines | 3 | — | Load; three-phase machines require the configured live phases |
| `motor_3ph_6` | Three-phase motor (6 terminals) | Motor trifásico (6 terminais) | machines | 7 | — | Load; three-phase machines require the configured live phases |
| `motor_3ph_yd` | Three-phase motor Y/D | Motor trifásico Y/D | machines | 7 | — | Load; three-phase machines require the configured live phases |
| `motor_1ph` | Single-phase motor | Motor monofásico | machines | 6 | — | Load; three-phase machines require the configured live phases |
| `motor_2speed` | Two-speed motor | Motor de duas velocidades | machines | 7 | — | Load; three-phase machines require the configured live phases |
| `motor_dahlander` | Dahlander motor | Motor Dahlander | machines | 7 | — | Load; three-phase machines require the configured live phases |
| `motor_wound_rotor` | Wound-rotor motor | Motor de rotor bobinado | machines | 10 | — | Load; three-phase machines require the configured live phases |
| `motor_dc_pm` | DC motor permanent magnet | Motor CC ímã permanente | machines | 2 | — | Load; three-phase machines require the configured live phases |
| `motor_dc_indep` | DC motor independent excitation | Motor CC excitação independente | machines | 6 | — | Load; three-phase machines require the configured live phases |
| `stepper_unipolar` | Unipolar stepper motor | Motor passo a passo unipolar | machines | 5 | — | Load; three-phase machines require the configured live phases |
| `stepper_bipolar` | Bipolar stepper motor | Motor passo a passo bipolar | machines | 4 | — | Load; three-phase machines require the configured live phases |
| `linear_actuator` | Electric linear actuator | Atuador linear elétrico | machines | 2 | — | Load; three-phase machines require the configured live phases |
| `resistor_1p` | Resistor I | Resistência I | power | 2 | — | Pass-through, source or visual power stage |
| `resistor_3p` | Resistor III | Resistência III | power | 6 | — | Pass-through, source or visual power stage |
| `autotransformer_3p` | Autotransformer III | Autotransformador III | power | 6 | — | Pass-through, source or visual power stage |
| `soft_starter` | Electronic soft starter | Arrancador eletrônico CA | power | 6 | — | Pass-through, source or visual power stage |
| `vfd_ac_2p` | AC variable frequency drive II | Variador de velocidade CA II | power | 5 | — | Pass-through, source or visual power stage |
| `vfd_ac_3p` | AC variable frequency drive III | Variador de velocidade CA III | power | 6 | — | Pass-through, source or visual power stage |
| `vfd_dc_2p` | DC drive II | Variador de velocidade CC II | power | 4 | — | Pass-through, source or visual power stage |
| `vfd_dc_3p` | DC drive III | Variador de velocidade CC III | power | 6 | — | Pass-through, source or visual power stage |
| `diode` | Diode | Diodo | power | 2 | — | Pass-through, source or visual power stage |
| `rectifier_bridge` | Rectifier bridge | Retificador | power | 4 | — | Pass-through, source or visual power stage |
| `transformer_3p` | Transformer III | Transformador III | power | 6 | — | Pass-through, source or visual power stage |
| `heater_1p` | Heater I | Resistência de aquecimento I | power | 2 | — | Pass-through, source or visual power stage |
| `logic_and` | Logic AND | Função lógica AND | logic | 4 | — | Boolean gate with a switched output source |
| `logic_or` | Logic OR | Função lógica OR | logic | 4 | — | Boolean gate with a switched output source |
| `logic_not` | Logic INV | Função lógica INV | logic | 3 | — | Boolean gate with a switched output source |
| `logic_xor` | Logic EXOR | Função lógica EXOR | logic | 4 | — | Boolean gate with a switched output source |
| `logic_nand` | Logic NAND | Função lógica NAND | logic | 4 | — | Boolean gate with a switched output source |
| `logic_nor` | Logic NOR | Função lógica NOR | logic | 4 | — | Boolean gate with a switched output source |
| `logic_sr` | SR latch | Biestável RS | logic | 4 | — | Boolean gate with a switched output source |
| `logic_edge_up` | Rising edge | Flanco ascendente | logic | 2 | — | Boolean gate with a switched output source |
| `logic_edge_down` | Falling edge | Flanco descendente | logic | 2 | — | Boolean gate with a switched output source |
| `logic_timer_on` | Logic timer ON | Temporizador conexão | logic | 3 | — | Boolean gate with a switched output source |
| `logic_counter` | Logic counter | Contador | logic | 3 | — | Boolean gate with a switched output source |
| `display_7seg` | 7-segment display | Display 7 segmentos | logic | 7 | — | Boolean gate with a switched output source |
| `plc_cpu` | PLC CPU | CPU PLC | plc | 5 | — | Visual I/O block; no user PLC program is executed |
| `plc_di` | Digital input PLC | Entrada digital PLC | plc | 6 | — | Visual I/O block; no user PLC program is executed |
| `plc_do` | Digital output PLC | Saída digital PLC | plc | 6 | — | Visual I/O block; no user PLC program is executed |
| `plc_power` | PLC power supply | Alimentação I/O digital PLC | plc | 4 | — | Visual I/O block; no user PLC program is executed |
| `plc_s7_1200` | PLC S7-1200 | PLC S7-1200 | plc | 12 | — | Visual I/O block; no user PLC program is executed |
| `plc_logo` | LOGO 4DI/4DO | LOGO 4DI/4DO | plc | 10 | — | Visual I/O block; no user PLC program is executed |
| `plc_et200_di` | ET-200 8DI | ET-200 8DI | plc | 10 | — | Visual I/O block; no user PLC program is executed |
| `plc_et200_do` | ET-200 8DO | ET-200 8DO | plc | 10 | — | Visual I/O block; no user PLC program is executed |
| `arduino_mega` | Arduino Mega | Arduino Mega | plc | 12 | — | Visual I/O block; no user PLC program is executed |
| `hmi_panel` | Operator panel | Botonera | plc | 3 | — | Visual I/O block; no user PLC program is executed |
| `pneumatic_cylinder_single` | Single-acting cylinder | Atuador simples efeito | pneumatic | 2 | — | Solenoid coil loads and visual pneumatic symbols; no pressure model |
| `pneumatic_cylinder_double` | Double-acting cylinder | Atuador duplo efeito | pneumatic | 4 | — | Solenoid coil loads and visual pneumatic symbols; no pressure model |
| `pneumatic_cylinder_rodless` | Rodless cylinder | Atuador sem haste | pneumatic | 2 | — | Solenoid coil loads and visual pneumatic symbols; no pressure model |
| `pneumatic_rotary` | Rotary actuator | Atuador de giro | pneumatic | 2 | — | Solenoid coil loads and visual pneumatic symbols; no pressure model |
| `pneumatic_gripper` | Double-acting gripper | Pinça duplo efeito | pneumatic | 2 | — | Solenoid coil loads and visual pneumatic symbols; no pressure model |
| `pneumatic_vacuum` | Vacuum cup | Ventosa | pneumatic | 1 | — | Solenoid coil loads and visual pneumatic symbols; no pressure model |
| `pneumatic_motor` | Pneumatic motor | Motor pneumático | pneumatic | 2 | — | Solenoid coil loads and visual pneumatic symbols; no pressure model |
| `pneumatic_motor_reversible` | Reversible pneumatic motor | Motor pneumático reversível | pneumatic | 2 | — | Solenoid coil loads and visual pneumatic symbols; no pressure model |
| `pneumatic_valve_3_2` | 3/2 valve | Válvula 3/2 | pneumatic | 4 | — | Solenoid coil loads and visual pneumatic symbols; no pressure model |
| `pneumatic_valve_5_2` | 5/2 valve | Válvula 5/2 | pneumatic | 6 | — | Solenoid coil loads and visual pneumatic symbols; no pressure model |
| `pneumatic_hose` | Hose | Manguera | pneumatic | 2 | — | Solenoid coil loads and visual pneumatic symbols; no pressure model |
| `terminal` | Terminal | Borne | terminals | 1 | — | Connection points, terminals, cables and junctions |
| `terminal_square` | Square terminal | Borne quadrado | terminals | 1 | — | Connection points, terminals, cables and junctions |
| `terminal_round` | Round terminal | Borne redondo | terminals | 1 | — | Connection points, terminals, cables and junctions |
| `terminal_2` | 2 round terminals | 2 bornes redondos | terminals | 2 | — | Connection points, terminals, cables and junctions |
| `terminal_3` | 3 round terminals | 3 bornes redondos | terminals | 3 | — | Connection points, terminals, cables and junctions |
| `junction` | Junction | Conexão | terminals | 1 | — | Connection points, terminals, cables and junctions |
| `cable_1` | Phase cable | Cabo fase | terminals | 2 | — | Connection points, terminals, cables and junctions |
| `cable_3` | Three-phase cable | Cabo trifásico | terminals | 6 | — | Connection points, terminals, cables and junctions |
| `cable_neutral` | Neutral cable | Cabo neutro | terminals | 2 | — | Connection points, terminals, cables and junctions |
| `cable_pe` | Protective cable | Cabo proteção | terminals | 2 | — | Connection points, terminals, cables and junctions |
| `meter_voltage` | Voltmeter | Voltímetro | measurement | 2 | — | Visual instrument; high impedance, no measurement value |
| `meter_current` | Ammeter | Amperímetro | measurement | 2 | — | Visual instrument; high impedance, no measurement value |
| `meter_power` | Wattmeter | Wattímetro | measurement | 3 | — | Visual instrument; high impedance, no measurement value |
| `meter_multimeter` | Multimeter | Multímetro | measurement | 3 | — | Visual instrument; high impedance, no measurement value |
| `grafcet_step_initial` | Initial step | Etapa inicial GRAFCET | grafcet | 2 | — | Visual GRAFCET step/transition/action elements; no token engine |
| `grafcet_step` | Step | Etapa GRAFCET | grafcet | 2 | — | Visual GRAFCET step/transition/action elements; no token engine |
| `grafcet_transition` | Transition | Transição GRAFCET | grafcet | 2 | — | Visual GRAFCET step/transition/action elements; no token engine |
| `grafcet_action` | Action | Ação GRAFCET | grafcet | 1 | — | Visual GRAFCET step/transition/action elements; no token engine |
| `grafcet_link` | Link line | Linha de enlace GRAFCET | grafcet | 0 | — | Visual GRAFCET step/transition/action elements; no token engine |
| `grafcet_convergence` | AND convergence | Convergência em Y | grafcet | 3 | — | Visual GRAFCET step/transition/action elements; no token engine |
| `grafcet_divergence` | OR divergence | Divergência em O | grafcet | 3 | — | Visual GRAFCET step/transition/action elements; no token engine |
| `motor_3ph_1ph` | Three-phase motor as single-phase | Motor trifásico como monofásico | machines | 9 | — | Load; three-phase machines require the configured live phases |
| `vfd_dc_control` | DC drive control | Controle variador CC | power | 3 | — | Pass-through, source or visual power stage |
| `power_supply` | Power supply | Fonte de alimentação | supplies | 4 | — | Source groups (L/N/PE, AC/DC, transformer/rectifier) |
| `hose_entry` | Hose entry | Entrada de mangueira | pneumatic | 2 | — | Solenoid coil loads and visual pneumatic symbols; no pressure model |
| `connector_in` | Input connector | Conector de entrada | terminals | 2 | — | Connection points, terminals, cables and junctions |
| `connector_out` | Output connector | Conector de saída | terminals | 2 | — | Connection points, terminals, cables and junctions |
| `terminal_4` | 4 round terminals | 4 bornes redondos | terminals | 4 | — | Connection points, terminals, cables and junctions |
| `disconnector_1p` | Disconnector I | Seccionador I | protection | 2 | — | Pass-through poles; trip/blown opens the protected path |
| `disconnector_1pn` | Disconnector IN | Seccionador IN | protection | 4 | — | Pass-through poles; trip/blown opens the protected path |
| `disconnector_2p` | Disconnector II | Seccionador II | protection | 4 | — | Pass-through poles; trip/blown opens the protected path |
| `disconnector_3p` | Disconnector III | Seccionador III | protection | 6 | — | Pass-through poles; trip/blown opens the protected path |
| `disconnector_3pn` | Disconnector IIIN | Seccionador IIIN | protection | 8 | — | Pass-through poles; trip/blown opens the protected path |
| `switch_power_1p` | Power switch I | Interruptor I | control | 2 | — | Momentary or maintained digital inputs; selector positions |
| `switch_power_2p` | Power switch II | Interruptor II | control | 4 | — | Momentary or maintained digital inputs; selector positions |
| `switch_power_3p` | Power switch III | Interruptor III | control | 6 | — | Momentary or maintained digital inputs; selector positions |
| `switch_power_4p` | Power switch IIII | Interruptor IIII | control | 8 | — | Momentary or maintained digital inputs; selector positions |
| `teleruptor` | Latching relay (teleruptor) | Telerruptor | relays | 4 | — | Coil plus group-controlled changeover contacts |
| `solenoid_valve` | Solenoid valve coil | Bobina de eletroválvula | pneumatic | 2 | — | Solenoid coil loads and visual pneumatic symbols; no pressure model |
| `horn` | Horn | Bocina | indication | 2 | — | Load energized from two distinct source groups |
| `logic_divider` | Binary divider | Divisor binário | logic | 2 | — | Boolean gate with a switched output source |
| `button_station` | Pushbutton station | Botonera | control | 6 | — | Momentary or maintained digital inputs; selector positions |
| `fuse_switch_3pn` | Fuse switch IIIN | Fusível seccionável IIIN | protection | 8 | — | Pass-through poles; trip/blown opens the protected path |
| `contactor_combo` | Contactor with coil and auxiliary contacts | Contator com bobina e contatos auxiliares | contactors | 12 | — | Coil plus group-controlled main/auxiliary contacts |
| `relay_combo` | Relay with coil and changeover contacts | Relé com bobina e contatos comutados | relays | 8 | — | Coil plus group-controlled changeover contacts |
| `fuse` | Fuse | Fusível | protection | 2 | — | Pass-through poles; trip/blown opens the protected path |
| `breaker` | Circuit breaker | Disjuntor | protection | 2 | — | Pass-through poles; trip/blown opens the protected path |
| `contactor_no` | Contactor main contact | Contato principal de contator | contactors | 2 | — | Coil plus group-controlled main/auxiliary contacts |
| `selector_switch` | Selector switch | Chave seletora | control | 4 | — | Momentary or maintained digital inputs; selector positions |
| `thermal_relay` | Thermal overload relay | Relé térmico | protection | 4 | — | Pass-through poles; trip/blown opens the protected path |
| `label_box` | Text label | Etiqueta de texto | annotations | 0 | — | Text label |

## Recovered CADe_SIMU type IDs

The property dialog in the shipped executable enumerates the following internal type IDs. They are retained here as the evidence trail for the catalog; FioVivo uses stable string type identifiers instead of the original numeric IDs.

| CADe_SIMU ID | Recovered name | FioVivo coverage |
|---|---|---|
| 1000 (0x3e8) | Motor trifásico | `motor_*` family |
| 1001 (0x3e9) | Motor trifásico YD | `motor_*` family |
| 1002 (0x3ea) | Motor monofásico | `motor_*` family |
| 1003 (0x3eb) | Motor Dahalander | `motor_*` family |
| 1004 (0x3ec) | Motor rotor bobinado | `motor_*` family |
| 1005 (0x3ed) | Motor dos velocidades | `motor_*` family |
| 1006 (0x3ee) | Motor cc. iman permanente | `motor_*` family |
| 1007 (0x3ef) | Motor cc. ex. indepen. | `motor_*` family |
| 1010 (0x3f2) | Motor trifásico como monofásico | `vfd_*` / power family |
| 1050 (0x41a) | Resistencia I | `vfd_*` / power family |
| 1051 (0x41b) | Resistencia III | `vfd_*` / power family |
| 1052 (0x41c) | Autotransformador III | `vfd_*` / power family |
| 1053 (0x41d) | Arrancador electrónico | `vfd_*` / power family |
| 1054 (0x41e) | Variador velocidad ca II. | `vfd_*` / power family |
| 1055 (0x41f) | Variador velocidad ca III. | `vfd_*` / power family |
| 1057 (0x421) | Variador velocidad cc. | `vfd_*` / power family |
| 1058 (0x422) | Variador velocidad cc. | `vfd_*` / power family |
| 1059 (0x423) | ControlVariador velocidad cc. | `vfd_*` / power family |
| 1076 (0x434) | Relé 1 canal | `vfd_*` / power family |
| 2000 (0x7d0) | Contactor II | `contactor_*` / `switch_power_*` |
| 2001 (0x7d1) | Contactor III | `contactor_*` / `switch_power_*` |
| 2002 (0x7d2) | Contactor IIII | `contactor_*` / `switch_power_*` |
| 2003 (0x7d3) | Interruptor II | `contactor_*` / `switch_power_*` |
| 2004 (0x7d4) | Interruptor III | `contactor_*` / `switch_power_*` |
| 2005 (0x7d5) | Interruptor IIII | `contactor_*` / `switch_power_*` |
| 2006 (0x7d6) | Contactor I | `contactor_*` / `switch_power_*` |
| 2007 (0x7d7) | Interruptor I | `contactor_*` / `switch_power_*` |
| 3000 (0xbb8) | Alimentación L | `supply_*` / `power_supply` |
| 3001 (0xbb9) | Alimentación N | `supply_*` / `power_supply` |
| 3002 (0xbba) | Alimentación PE | `supply_*` / `power_supply` |
| 3003 (0xbbb) | Alimentación L N | `supply_*` / `power_supply` |
| 3004 (0xbbc) | Alimentación L1 L2 L3 | `supply_*` / `power_supply` |
| 3005 (0xbbd) | Alimentación L1 L2 L3 PE | `supply_*` / `power_supply` |
| 3006 (0xbbe) | Alimentación L1 L2 L3 N | `supply_*` / `power_supply` |
| 3007 (0xbbf) | Alimentación L1 L2 L3 N PE | `supply_*` / `power_supply` |
| 3008 (0xbc0) | Alimentación + | `supply_*` / `power_supply` |
| 3009 (0xbc1) | Alimentación - | `supply_*` / `power_supply` |
| 3010 (0xbc2) | Alimentación + - | `supply_*` / `power_supply` |
| 3011 (0xbc3) | Alimentación L N PE | `supply_*` / `power_supply` |
| 3012 (0xbc4) | Transformador | `supply_*` / `power_supply` |
| 3013 (0xbc5) | Rectificador | `supply_*` / `power_supply` |
| 3014 (0xbc6) | Fuente de alimentacióm | `supply_*` / `power_supply` |
| 4000 (0xfa0) | Cable fase | `terminal_*` / `cable_*` / `connector_*` |
| 4001 (0xfa1) | Conexión | `terminal_*` / `cable_*` / `connector_*` |
| 4002 (0xfa2) | Manguera | `terminal_*` / `cable_*` / `connector_*` |
| 4003 (0xfa3) | Entrada manguera | `terminal_*` / `cable_*` / `connector_*` |
| 4004 (0xfa4) | Entrada manguera | `terminal_*` / `cable_*` / `connector_*` |
| 4009 (0xfa9) | Cable neutro | `terminal_*` / `cable_*` / `connector_*` |
| 4010 (0xfaa) | Cable PE | `terminal_*` / `cable_*` / `connector_*` |
| 4011 (0xfab) | Regleta cuadrada | `terminal_*` / `cable_*` / `connector_*` |
| 4012 (0xfac) | Regleta redonda | `terminal_*` / `cable_*` / `connector_*` |
| 4013 (0xfad) | Conector salida | `terminal_*` / `cable_*` / `connector_*` |
| 4014 (0xfae) | Conector entrada | `terminal_*` / `cable_*` / `connector_*` |
| 4015 (0xfaf) | Regleta redonda 2 | `terminal_*` / `cable_*` / `connector_*` |
| 4016 (0xfb0) | Regleta redonda 3 | `terminal_*` / `cable_*` / `connector_*` |
| 4017 (0xfb1) | Regleta redonda 4 | `terminal_*` / `cable_*` / `connector_*` |
| 4018 (0xfb2) | Cable + | `terminal_*` / `cable_*` / `connector_*` |
| 4019 (0xfb3) | Cable - | `terminal_*` / `cable_*` / `connector_*` |
| 5000 (0x1388) | Fusible I | `fuse_*` / `fuse_switch_*` / `disconnector_*` |
| 5001 (0x1389) | Fusible IN | `fuse_*` / `fuse_switch_*` / `disconnector_*` |
| 5002 (0x138a) | Fusible II | `fuse_*` / `fuse_switch_*` / `disconnector_*` |
| 5003 (0x138b) | Fusible III | `fuse_*` / `fuse_switch_*` / `disconnector_*` |
| 5004 (0x138c) | Fusible IIIN | `fuse_*` / `fuse_switch_*` / `disconnector_*` |
| 5005 (0x138d) | Seccionador I | `fuse_*` / `fuse_switch_*` / `disconnector_*` |
| 5006 (0x138e) | Seccionador IN | `fuse_*` / `fuse_switch_*` / `disconnector_*` |
| 5007 (0x138f) | Seccionador II | `fuse_*` / `fuse_switch_*` / `disconnector_*` |
| 5008 (0x1390) | Seccionador III | `fuse_*` / `fuse_switch_*` / `disconnector_*` |
| 5009 (0x1391) | Seccionador IIIN | `fuse_*` / `fuse_switch_*` / `disconnector_*` |
| 6000 (0x1770) | Automático I | `breaker_*` / `rcd_*` / `thermal_relay` / `motor_breaker_3p` |
| 6001 (0x1771) | Automático IN | `breaker_*` / `rcd_*` / `thermal_relay` / `motor_breaker_3p` |
| 6002 (0x1772) | Automático II | `breaker_*` / `rcd_*` / `thermal_relay` / `motor_breaker_3p` |
| 6003 (0x1773) | Automático III | `breaker_*` / `rcd_*` / `thermal_relay` / `motor_breaker_3p` |
| 6004 (0x1774) | Automático IIIN | `breaker_*` / `rcd_*` / `thermal_relay` / `motor_breaker_3p` |
| 6005 (0x1775) | Diferencial IN | `breaker_*` / `rcd_*` / `thermal_relay` / `motor_breaker_3p` |
| 6006 (0x1776) | Diferencial IIIN | `breaker_*` / `rcd_*` / `thermal_relay` / `motor_breaker_3p` |
| 6007 (0x1777) | Relé térmico | `breaker_*` / `rcd_*` / `thermal_relay` / `motor_breaker_3p` |
| 6008 (0x1778) | Disyuntor II | `breaker_*` / `rcd_*` / `thermal_relay` / `motor_breaker_3p` |
| 6009 (0x1779) | Disyuntor III | `breaker_*` / `rcd_*` / `thermal_relay` / `motor_breaker_3p` |
| 6010 (0x177a) | Disyuntor I | `breaker_*` / `rcd_*` / `thermal_relay` / `motor_breaker_3p` |
| 6015 (0x177f) | Disyuntor | `breaker_*` / `rcd_*` / `thermal_relay` / `motor_breaker_3p` |
| 6016 (0x1780) | Relé térmico | `breaker_*` / `rcd_*` / `thermal_relay` / `motor_breaker_3p` |
| 6017 (0x1781) | Interruptor automático | `breaker_*` / `rcd_*` / `thermal_relay` / `motor_breaker_3p` |
| 6018 (0x1782) | Interruptor diferencial | `breaker_*` / `rcd_*` / `thermal_relay` / `motor_breaker_3p` |
| 6019 (0x1783) | Interruptor | `breaker_*` / `rcd_*` / `thermal_relay` / `motor_breaker_3p` |
| 6020 (0x1784) | Seccionador | `breaker_*` / `rcd_*` / `thermal_relay` / `motor_breaker_3p` |
| 7002 (0x1b5a) | Contacto auxiliar NA-NC | `contactor_*` / `relay_*` / `contact_timed_*` / `contactor_combo` / `relay_combo` |
| 7003 (0x1b5b) | Contacto auxiliar conmutado | `contactor_*` / `relay_*` / `contact_timed_*` / `contactor_combo` / `relay_combo` |
| 7004 (0x1b5c) | Contacto temporizador conexión NA | `contactor_*` / `relay_*` / `contact_timed_*` / `contactor_combo` / `relay_combo` |
| 7005 (0x1b5d) | Contacto temporizador conexión NC | `contactor_*` / `relay_*` / `contact_timed_*` / `contactor_combo` / `relay_combo` |
| 7006 (0x1b5e) | Contacto temporizador conexión NA-NC | `contactor_*` / `relay_*` / `contact_timed_*` / `contactor_combo` / `relay_combo` |
| 7007 (0x1b5f) | Contacto temporizador conexión conmutado | `contactor_*` / `relay_*` / `contact_timed_*` / `contactor_combo` / `relay_combo` |
| 7008 (0x1b60) | Contacto temporizador desconexión NA | `contactor_*` / `relay_*` / `contact_timed_*` / `contactor_combo` / `relay_combo` |
| 7009 (0x1b61) | Contacto temporizador desconexión NC | `contactor_*` / `relay_*` / `contact_timed_*` / `contactor_combo` / `relay_combo` |
| 7010 (0x1b62) | Contacto temporizador desconexión NA-NC | `contactor_*` / `relay_*` / `contact_timed_*` / `contactor_combo` / `relay_combo` |
| 7011 (0x1b63) | Contacto temporizador desconexión conmutado | `contactor_*` / `relay_*` / `contact_timed_*` / `contactor_combo` / `relay_combo` |
| 7012 (0x1b64) | Contacto temporizador conexión desconexión NA | `contactor_*` / `relay_*` / `contact_timed_*` / `contactor_combo` / `relay_combo` |
| 7013 (0x1b65) | Contacto temporizador conexión desconexión NC | `contactor_*` / `relay_*` / `contact_timed_*` / `contactor_combo` / `relay_combo` |
| 7014 (0x1b66) | Contacto temporizador conexión desconexión NA-NC | `contactor_*` / `relay_*` / `contact_timed_*` / `contactor_combo` / `relay_combo` |
| 7015 (0x1b67) | Contacto temporizador conexión desconexión conmutado | `contactor_*` / `relay_*` / `contact_timed_*` / `contactor_combo` / `relay_combo` |
| 7016 (0x1b68) | Rele | `contactor_*` / `relay_*` / `contact_timed_*` / `contactor_combo` / `relay_combo` |
| 7018 (0x1b6a) | Contactor | `contactor_*` / `relay_*` / `contact_timed_*` / `contactor_combo` / `relay_combo` |
| 7019 (0x1b6b) | Contactos auxiliares | `contactor_*` / `relay_*` / `contact_timed_*` / `contactor_combo` / `relay_combo` |
| 7020 (0x1b6c) | Contactos ON Delay | `contactor_*` / `relay_*` / `contact_timed_*` / `contactor_combo` / `relay_combo` |
| 7021 (0x1b6d) | Contactos OFF Delay | `contactor_*` / `relay_*` / `contact_timed_*` / `contactor_combo` / `relay_combo` |
| 7550 (0x1d7e) | Barrera fotoeléctrica reflex | `contactor_*` / `relay_*` / `contact_timed_*` / `contactor_combo` / `relay_combo` |
| 8000 (0x1f40) | Contacto pulsador NA | `pushbutton_*` / `switch_*` / `limit_*` / `mushroom_*` |
| 8001 (0x1f41) | Contacto pulsador NC | `pushbutton_*` / `switch_*` / `limit_*` / `mushroom_*` |
| 8002 (0x1f42) | Contacto pulsador NA-NC | `pushbutton_*` / `switch_*` / `limit_*` / `mushroom_*` |
| 8003 (0x1f43) | Contacto pulsador conmutado | `pushbutton_*` / `switch_*` / `limit_*` / `mushroom_*` |
| 8004 (0x1f44) | Contacto pulsador seta NA | `pushbutton_*` / `switch_*` / `limit_*` / `mushroom_*` |
| 8005 (0x1f45) | Contacto pulsador seta NC | `pushbutton_*` / `switch_*` / `limit_*` / `mushroom_*` |
| 8006 (0x1f46) | Contacto pulsador seta NA-NC | `pushbutton_*` / `switch_*` / `limit_*` / `mushroom_*` |
| 8007 (0x1f47) | Contacto pulsador seta conmutado | `pushbutton_*` / `switch_*` / `limit_*` / `mushroom_*` |
| 8008 (0x1f48) | Contacto interruptor NA | `pushbutton_*` / `switch_*` / `limit_*` / `mushroom_*` |
| 8009 (0x1f49) | Contacto interruptor NC | `pushbutton_*` / `switch_*` / `limit_*` / `mushroom_*` |
| 8010 (0x1f4a) | Contacto interruptor NA-NC | `pushbutton_*` / `switch_*` / `limit_*` / `mushroom_*` |
| 8011 (0x1f4b) | Contacto interruptor conmutado | `pushbutton_*` / `switch_*` / `limit_*` / `mushroom_*` |
| 8012 (0x1f4c) | Contacto final de carrera NA | `pushbutton_*` / `switch_*` / `limit_*` / `mushroom_*` |
| 8013 (0x1f4d) | Contacto final de carrera NC | `pushbutton_*` / `switch_*` / `limit_*` / `mushroom_*` |
| 8014 (0x1f4e) | Contacto final de carrera NA-NC | `pushbutton_*` / `switch_*` / `limit_*` / `mushroom_*` |
| 8015 (0x1f4f) | Contacto final de carrera conmutado | `pushbutton_*` / `switch_*` / `limit_*` / `mushroom_*` |
| 8100 (0x1fa4) | Pulsador | `pushbutton_*` / `switch_*` / `limit_*` / `mushroom_*` |
| 8101 (0x1fa5) | Piloto | `pushbutton_*` / `switch_*` / `limit_*` / `mushroom_*` |
| 8102 (0x1fa6) | Botonera | `pushbutton_*` / `switch_*` / `limit_*` / `mushroom_*` |
| 8103 (0x1fa7) | LED | `pushbutton_*` / `switch_*` / `limit_*` / `mushroom_*` |
| 9000 (0x2328) | Bobina monoestable | `*_coil` / `timer_*` / `pilot_lamp` / `buzzer` / `siren` / `bell` / `horn` |
| 9001 (0x2329) | Bobina biestable | `*_coil` / `timer_*` / `pilot_lamp` / `buzzer` / `siren` / `bell` / `horn` |
| 9002 (0x232a) | Telerruptor | `*_coil` / `timer_*` / `pilot_lamp` / `buzzer` / `siren` / `bell` / `horn` |
| 9003 (0x232b) | Electroválvula | `*_coil` / `timer_*` / `pilot_lamp` / `buzzer` / `siren` / `bell` / `horn` |
| 9004 (0x232c) | Temporizador a la conexión | `*_coil` / `timer_*` / `pilot_lamp` / `buzzer` / `siren` / `bell` / `horn` |
| 9005 (0x232d) | Temporizador a la desconexión | `*_coil` / `timer_*` / `pilot_lamp` / `buzzer` / `siren` / `bell` / `horn` |
| 9006 (0x232e) | Temporizador a la conexión desconexión | `*_coil` / `timer_*` / `pilot_lamp` / `buzzer` / `siren` / `bell` / `horn` |
| 9007 (0x232f) | Temporizador intermitente | `*_coil` / `timer_*` / `pilot_lamp` / `buzzer` / `siren` / `bell` / `horn` |
| 9008 (0x2330) | Señalización óptica | `*_coil` / `timer_*` / `pilot_lamp` / `buzzer` / `siren` / `bell` / `horn` |
| 9009 (0x2331) | Señalización óptica intermitente | `*_coil` / `timer_*` / `pilot_lamp` / `buzzer` / `siren` / `bell` / `horn` |
| 9010 (0x2332) | Timbre | `*_coil` / `timer_*` / `pilot_lamp` / `buzzer` / `siren` / `bell` / `horn` |
| 9011 (0x2333) | Zumbador | `*_coil` / `timer_*` / `pilot_lamp` / `buzzer` / `siren` / `bell` / `horn` |
| 9012 (0x2334) | Sirena | `*_coil` / `timer_*` / `pilot_lamp` / `buzzer` / `siren` / `bell` / `horn` |
| 9013 (0x2335) | Bocina | `*_coil` / `timer_*` / `pilot_lamp` / `buzzer` / `siren` / `bell` / `horn` |
| 10040 (0x2738) | Función lógica INV | `logic_*` / `contact_*` |
| 10041 (0x2739) | Función lógica EXOR | `logic_*` / `contact_*` |
| 10050 (0x2742) | Contacto NA | `logic_*` / `contact_*` |
| 10051 (0x2743) | Contacto NC | `logic_*` / `contact_*` |
| 10052 (0x2744) | Flanco ascendente | `logic_*` / `contact_*` |
| 10053 (0x2745) | Flanco descendente | `logic_*` / `contact_*` |
| 10060 (0x274c) | Asignar | `logic_*` / `contact_*` |
| 10061 (0x274d) | Set | `logic_*` / `contact_*` |
| 10062 (0x274e) | Reset | `logic_*` / `contact_*` |
| 10064 (0x2750) | Biestable RS | `logic_*` / `contact_*` |
| 10065 (0x2751) | Bivisor Binario | `logic_*` / `contact_*` |
| 10070 (0x2756) | Prueba | `logic_*` / `contact_*` |
| 10071 (0x2757) | Visualiza | `logic_*` / `contact_*` |
| 10100 (0x2774) | Etapa 0 GRAFCET | `grafcet_*` |
| 10101 (0x2775) | Etapa GRAFCET | `grafcet_*` |
| 10106 (0x277a) | Línea enlace GRAFCET | `grafcet_*` |
| 10107 (0x277b) | Enlace de entrada GRAFCET | `grafcet_*` |
| 10108 (0x277c) | Enlace de Salida GRAFCET | `grafcet_*` |
| 10111 (0x277f) | Macro Etapa GRAFCET | `grafcet_*` |
| 10112 (0x2780) | Y GRAFCET | `grafcet_*` |

## Evidence

- `docs/cadesimu/type_names.json` — 159 type IDs and names recovered from the property-dialog dispatcher.
- `docs/cadesimu/resources/` — 715 PE resources, including 316 bitmap palette assets and 23 MFC toolbars.
- `docs/cadesimu/symbols/` — 162 insertion captures from the running CADe_SIMU 4.0 application.
- `docs/cadesimu/decompiled/` — Ghidra decompilation of the property/behaviour dispatcher and related functions.
- `docs/cadesimu/visual-comparison/` — side-by-side comparisons of representative symbols.
