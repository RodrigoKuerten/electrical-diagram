# CADe_SIMU-style component catalog

215 types generated from the reconstructed catalog.

| Type | Name | PT | Category | Pins | States |
|---|---|---|---|---|---|
| `supply_ac1` | Supply L + N + PE | Alimentação F + N + PE | supplies | 3 | - |
| `supply_ac1_pe` | Supply L + N + PE | Alimentação F + N + PE | supplies | 3 | - |
| `supply_ac3` | Supply L1 + L2 + L3 + N + PE | Alimentação L1 + L2 + L3 + N + PE | supplies | 5 | - |
| `supply_ac3_n` | Supply L1+L2+L3+N | Alimentação L1+L2+L3+N | supplies | 4 | - |
| `supply_ac3_n_pe` | Supply L1+L2+L3+N+PE | Alimentação L1+L2+L3+N+PE | supplies | 5 | - |
| `supply_ac3_pe` | Supply L1+L2+L3+PE | Alimentação L1+L2+L3+PE | supplies | 4 | - |
| `supply_dc` | Supply + / - | Alimentação + / - | supplies | 2 | - |
| `supply_dc_pos` | Supply + | Alimentação + | supplies | 1 | - |
| `supply_neutral` | Supply N | Alimentação neutro | supplies | 1 | - |
| `supply_earth` | Supply PE | Alimentação proteção | supplies | 1 | - |
| `supply_battery` | Battery | Bateria | supplies | 2 | - |
| `supply_rectifier` | Rectifier | Retificador | supplies | 4 | - |
| `supply_transformer3` | Transformer III | Transformador III | supplies | 6 | - |
| `fuse_1p` | Fuse I | Fusível I | protection | 2 | blown |
| `fuse_2p` | Fuse II | Fusível II | protection | 4 | blown |
| `fuse_3p` | Fuse III | Fusível III | protection | 6 | blown |
| `fuse_3pn` | Fuse IIIN | Fusível IIIN | protection | 8 | blown |
| `fuse_switch_1pn` | Fuse switch IN | Fusível seccionável IN | protection | 4 | blown |
| `fuse_switch_2p` | Fuse switch II | Fusível seccionável II | protection | 4 | blown |
| `fuse_switch_3p` | Fuse switch III | Fusível seccionável III | protection | 6 | blown |
| `breaker_1p` | Breaker I | Disjuntor I | protection | 2 | tripped |
| `breaker_1pn` | Breaker IN | Disjuntor IN | protection | 4 | tripped |
| `breaker_2p` | Breaker II | Disjuntor II | protection | 4 | tripped |
| `breaker_3p` | Breaker III | Disjuntor III | protection | 6 | tripped |
| `breaker_3pn` | Breaker IIIN | Disjuntor IIIN | protection | 8 | tripped |
| `rcd_2p` | Differential switch IN | Interruptor diferencial IN | protection | 4 | tripped |
| `rcd_4p` | Differential switch IIIN | Interruptor diferencial IIIN | protection | 8 | tripped |
| `motor_breaker_3p` | Motor circuit breaker III | Disjuntor-motor III | protection | 6 | tripped |
| `surge_1pn` | Surge arrester IN | Limitador sobretensões IN | protection | 3 | - |
| `surge_3pn` | Surge arrester IIIN | Limitador sobretensões IIIN | protection | 5 | - |
| `overvoltage_relay` | Permanent overvoltage protection | Proteção sobretensões permanentes | protection | 4 | active |
| `contactor_1p` | Contactor I | Contator I | contactors | 2 | active |
| `contactor_2p` | Contactor II | Contator II | contactors | 4 | active |
| `contactor_3p` | Contactor III | Contator III | contactors | 6 | active |
| `contactor_4p` | Contactor IIII | Contator IIII | contactors | 8 | active |
| `contactor_aux_no` | Auxiliary contact NO | Contato auxiliar NA | contactors | 2 | active |
| `contactor_aux_nc` | Auxiliary contact NC | Contato auxiliar NF | contactors | 2 | active |
| `contactor_aux_co` | Auxiliary changeover contact | Contato auxiliar comutado | contactors | 3 | active |
| `contactor_coil` | Contactor coil | Bobina de contator | contactors | 2 | active |
| `contactor_coil_bistable` | Bistable contactor coil | Bobina biestável de contator | contactors | 4 | active |
| `relay_coil` | Relay coil | Bobina de relé | relays | 2 | active |
| `relay_coil_bistable` | Bistable relay coil | Bobina biestável de relé | relays | 4 | active |
| `relay_contact_no` | Relay contact NO | Contato de relé NA | relays | 2 | active |
| `relay_contact_nc` | Relay contact NC | Contato de relé NF | relays | 2 | active |
| `relay_contact_co` | Relay changeover contact | Contato de relé comutado | relays | 3 | active |
| `solid_state_relay_dc` | Solid-state relay DC | Relé de estado sólido DC | relays | 4 | - |
| `solid_state_relay_ac` | Solid-state relay AC | Relé de estado sólido AC | relays | 4 | - |
| `solid_state_contactor` | Solid-state contactor | Contator de estado sólido | relays | 6 | - |
| `contact_no` | Contact NO | Contato NA | contacts | 2 | active |
| `contact_nc` | Contact NC | Contato NF | contacts | 2 | active |
| `contact_co` | Changeover contact | Contato comutado | contacts | 3 | active |
| `pushbutton_no` | Pushbutton NO | Botão pulsador NA | control | 2 | active |
| `pushbutton_nc` | Pushbutton NC | Botão pulsador NF | control | 2 | active |
| `pushbutton_co` | Pushbutton changeover | Botão pulsador comutado | control | 3 | active |
| `mushroom_no` | Mushroom pushbutton NO | Pulsador seta NA | control | 2 | active |
| `mushroom_nc` | Mushroom pushbutton NC | Pulsador seta NF | control | 2 | active |
| `mushroom_co` | Mushroom changeover pushbutton | Pulsador seta comutado | control | 3 | active |
| `emergency_stop` | Emergency stop | Botão de emergência | control | 2 | active |
| `switch_no` | Switch NO | Interruptor NA | control | 2 | active |
| `switch_nc` | Switch NC | Interruptor NF | control | 2 | active |
| `switch_co` | Changeover switch | Interruptor comutado | control | 3 | active |
| `limit_no` | Limit switch NO | Fim de curso NA | sensors | 2 | active |
| `limit_nc` | Limit switch NC | Fim de curso NF | sensors | 2 | active |
| `limit_co` | Changeover limit switch | Fim de curso comutado | sensors | 3 | active |
| `selector_2pos` | Selector I-0-II | Comutador I-0-II | control | 3 | position_1, position_2, position_3 |
| `selector_3pos` | Selector I-II-III | Comutador I-II-III | control | 4 | position_1, position_2, position_3 |
| `selector_changeover` | Changeover selector | Comutador de cruzamento | control | 4 | active |
| `contact_timed_on_no` | Timed contact ON delay NO | Contato temporizado conexão NA | contacts | 2 | active |
| `contact_timed_off_no` | Timed contact OFF delay NO | Contato temporizado desconexão NA | contacts | 2 | active |
| `contact_timed_on_nc` | Timed contact ON delay NC | Contato temporizado conexão NF | contacts | 2 | active |
| `contact_timed_off_nc` | Timed contact OFF delay NC | Contato temporizado desconexão NF | contacts | 2 | active |
| `contact_timed_onoff_no` | Timed contact ON/OFF NO | Contato temporizado conexão/desconexão NA | contacts | 2 | active |
| `contact_thermal_no` | Thermal relay contact NO | Contato auxiliar térmico NA | protection | 2 | active |
| `contact_thermal_nc` | Thermal relay contact NC | Contato auxiliar térmico NF | protection | 2 | active |
| `contact_thermal_co` | Thermal relay changeover contact | Contato auxiliar térmico comutado | protection | 3 | active |
| `sensor_inductive_no` | Inductive sensor NA | Sensor indutivo NA | sensors | 3 | active |
| `sensor_inductive_nc` | Inductive sensor NF | Sensor indutivo NF | sensors | 3 | active |
| `sensor_capacitive_no` | Capacitive sensor NA | Sensor capacitivo NA | sensors | 3 | active |
| `sensor_capacitive_nc` | Capacitive sensor NF | Sensor capacitivo NF | sensors | 3 | active |
| `sensor_magnetic_no` | Magnetic sensor NA | Sensor magnético NA | sensors | 3 | active |
| `sensor_magnetic_nc` | Magnetic sensor NF | Sensor magnético NF | sensors | 3 | active |
| `sensor_photoelectric_no` | Photoelectric sensor NA | Sensor fotoelétrico NA | sensors | 3 | active |
| `sensor_photoelectric_nc` | Photoelectric sensor NF | Sensor fotoelétrico NF | sensors | 3 | active |
| `pressure_switch_no` | Pressure switch NO | Presostato NA | sensors | 2 | active |
| `vacuum_switch_no` | Vacuum switch NO | Vacuostato NA | sensors | 2 | active |
| `pressure_switch_nc` | Pressure switch NC | Presostato NF | sensors | 2 | active |
| `vacuum_switch_nc` | Vacuum switch NC | Vacuostato NF | sensors | 2 | active |
| `thermostat_no` | Thermostat NO | Termostato NA | sensors | 2 | active |
| `thermostat_nc` | Thermostat NC | Termostato NF | sensors | 2 | active |
| `float_no` | Float switch NO | Boia NA | sensors | 2 | active |
| `float_nc` | Float switch NC | Boia NF | sensors | 2 | active |
| `reed_no` | Reed switch NO | Sensor reed NA | sensors | 2 | active |
| `reed_nc` | Reed switch NC | Sensor reed NF | sensors | 2 | active |
| `timer_on` | Timer ON delay | Temporizador na conexão | timers | 4 | active |
| `timer_off` | Timer OFF delay | Temporizador na desconexão | timers | 4 | active |
| `timer_on_off` | Timer ON/OFF | Temporizador conexão/desconexão | timers | 5 | active |
| `timer_pulse` | Pulse timer | Temporizador de pulso | timers | 4 | - |
| `flasher` | Flasher relay | Relé intermitente | timers | 4 | - |
| `time_switch` | Time switch | Interruptor horário | timers | 3 | - |
| `counter` | Counter | Contador | timers | 4 | - |
| `counter_updown` | Up/down counter | Contador ascendente-descendente | timers | 6 | - |
| `pilot_lamp` | Pilot lamp | Piloto de sinalização | indication | 2 | active |
| `lamp_flashing` | Flashing pilot lamp | Piloto intermitente | indication | 2 | active |
| `buzzer` | Buzzer | Zumbador | indication | 2 | active |
| `siren` | Siren | Sirene | indication | 2 | active |
| `bell` | Bell | Campainha | indication | 2 | active |
| `led` | LED | Diodo LED | indication | 2 | active |
| `motor_3ph` | Three-phase motor | Motor trifásico | machines | 3 | active |
| `motor_3ph_6` | Three-phase motor (6 terminals) | Motor trifásico (6 terminais) | machines | 7 | active |
| `motor_3ph_yd` | Three-phase motor Y/D | Motor trifásico Y/D | machines | 7 | active |
| `motor_1ph` | Single-phase motor | Motor monofásico | machines | 6 | active |
| `motor_2speed` | Two-speed motor | Motor de duas velocidades | machines | 7 | active |
| `motor_dahlander` | Dahlander motor | Motor Dahlander | machines | 7 | active |
| `motor_wound_rotor` | Wound-rotor motor | Motor de rotor bobinado | machines | 10 | active |
| `motor_dc_pm` | DC motor permanent magnet | Motor CC ímã permanente | machines | 2 | active |
| `motor_dc_indep` | DC motor independent excitation | Motor CC excitação independente | machines | 6 | active |
| `stepper_unipolar` | Unipolar stepper motor | Motor passo a passo unipolar | machines | 5 | - |
| `stepper_bipolar` | Bipolar stepper motor | Motor passo a passo bipolar | machines | 4 | - |
| `linear_actuator` | Electric linear actuator | Atuador linear elétrico | machines | 2 | - |
| `resistor_1p` | Resistor I | Resistência I | power | 2 | active |
| `resistor_3p` | Resistor III | Resistência III | power | 6 | active |
| `autotransformer_3p` | Autotransformer III | Autotransformador III | power | 6 | - |
| `soft_starter` | Electronic soft starter | Arrancador eletrônico CA | power | 6 | - |
| `vfd_ac_2p` | AC variable frequency drive II | Variador de velocidade CA II | power | 5 | - |
| `vfd_ac_3p` | AC variable frequency drive III | Variador de velocidade CA III | power | 6 | - |
| `vfd_dc_2p` | DC drive II | Variador de velocidade CC II | power | 4 | - |
| `vfd_dc_3p` | DC drive III | Variador de velocidade CC III | power | 6 | - |
| `diode` | Diode | Diodo | power | 2 | - |
| `rectifier_bridge` | Rectifier bridge | Retificador | power | 4 | - |
| `transformer_3p` | Transformer III | Transformador III | power | 6 | - |
| `heater_1p` | Heater I | Resistência de aquecimento I | power | 2 | active |
| `logic_and` | Logic AND | Função lógica AND | logic | 4 | active |
| `logic_or` | Logic OR | Função lógica OR | logic | 4 | active |
| `logic_not` | Logic INV | Função lógica INV | logic | 3 | active |
| `logic_xor` | Logic EXOR | Função lógica EXOR | logic | 4 | active |
| `logic_nand` | Logic NAND | Função lógica NAND | logic | 4 | active |
| `logic_nor` | Logic NOR | Função lógica NOR | logic | 4 | active |
| `logic_sr` | SR latch | Biestável RS | logic | 4 | active |
| `logic_edge_up` | Rising edge | Flanco ascendente | logic | 2 | active |
| `logic_edge_down` | Falling edge | Flanco descendente | logic | 2 | active |
| `logic_timer_on` | Logic timer ON | Temporizador conexão | logic | 3 | active |
| `logic_counter` | Logic counter | Contador | logic | 3 | - |
| `display_7seg` | 7-segment display | Display 7 segmentos | logic | 7 | active |
| `plc_cpu` | PLC CPU | CPU PLC | plc | 5 | - |
| `plc_di` | Digital input PLC | Entrada digital PLC | plc | 6 | - |
| `plc_do` | Digital output PLC | Saída digital PLC | plc | 6 | - |
| `plc_power` | PLC power supply | Alimentação I/O digital PLC | plc | 4 | - |
| `plc_s7_1200` | PLC S7-1200 | PLC S7-1200 | plc | 12 | - |
| `plc_logo` | LOGO 4DI/4DO | LOGO 4DI/4DO | plc | 10 | - |
| `plc_et200_di` | ET-200 8DI | ET-200 8DI | plc | 10 | - |
| `plc_et200_do` | ET-200 8DO | ET-200 8DO | plc | 10 | - |
| `arduino_mega` | Arduino Mega | Arduino Mega | plc | 12 | - |
| `hmi_panel` | Operator panel | Botonera | plc | 3 | - |
| `pneumatic_cylinder_single` | Single-acting cylinder | Atuador simples efeito | pneumatic | 2 | - |
| `pneumatic_cylinder_double` | Double-acting cylinder | Atuador duplo efeito | pneumatic | 4 | - |
| `pneumatic_cylinder_rodless` | Rodless cylinder | Atuador sem haste | pneumatic | 2 | - |
| `pneumatic_rotary` | Rotary actuator | Atuador de giro | pneumatic | 2 | - |
| `pneumatic_gripper` | Double-acting gripper | Pinça duplo efeito | pneumatic | 2 | - |
| `pneumatic_vacuum` | Vacuum cup | Ventosa | pneumatic | 1 | - |
| `pneumatic_motor` | Pneumatic motor | Motor pneumático | pneumatic | 2 | - |
| `pneumatic_motor_reversible` | Reversible pneumatic motor | Motor pneumático reversível | pneumatic | 2 | - |
| `pneumatic_valve_3_2` | 3/2 valve | Válvula 3/2 | pneumatic | 4 | - |
| `pneumatic_valve_5_2` | 5/2 valve | Válvula 5/2 | pneumatic | 6 | - |
| `pneumatic_hose` | Hose | Manguera | pneumatic | 2 | - |
| `terminal` | Terminal | Borne | terminals | 1 | - |
| `terminal_square` | Square terminal | Borne quadrado | terminals | 1 | - |
| `terminal_round` | Round terminal | Borne redondo | terminals | 1 | - |
| `terminal_2` | 2 round terminals | 2 bornes redondos | terminals | 2 | - |
| `terminal_3` | 3 round terminals | 3 bornes redondos | terminals | 3 | - |
| `junction` | Junction | Conexão | terminals | 1 | - |
| `cable_1` | Phase cable | Cabo fase | terminals | 2 | - |
| `cable_3` | Three-phase cable | Cabo trifásico | terminals | 6 | - |
| `cable_neutral` | Neutral cable | Cabo neutro | terminals | 2 | - |
| `cable_pe` | Protective cable | Cabo proteção | terminals | 2 | - |
| `meter_voltage` | Voltmeter | Voltímetro | measurement | 2 | - |
| `meter_current` | Ammeter | Amperímetro | measurement | 2 | - |
| `meter_power` | Wattmeter | Wattímetro | measurement | 3 | - |
| `meter_multimeter` | Multimeter | Multímetro | measurement | 3 | - |
| `grafcet_step_initial` | Initial step | Etapa inicial GRAFCET | grafcet | 2 | - |
| `grafcet_step` | Step | Etapa GRAFCET | grafcet | 2 | - |
| `grafcet_transition` | Transition | Transição GRAFCET | grafcet | 2 | - |
| `grafcet_action` | Action | Ação GRAFCET | grafcet | 1 | - |
| `grafcet_link` | Link line | Linha de enlace GRAFCET | grafcet | 0 | - |
| `grafcet_convergence` | AND convergence | Convergência em Y | grafcet | 3 | - |
| `grafcet_divergence` | OR divergence | Divergência em O | grafcet | 3 | - |
| `motor_3ph_1ph` | Three-phase motor as single-phase | Motor trifásico como monofásico | machines | 9 | active |
| `vfd_dc_control` | DC drive control | Controle variador CC | power | 3 | - |
| `power_supply` | Power supply | Fonte de alimentação | supplies | 4 | - |
| `hose_entry` | Hose entry | Entrada de mangueira | pneumatic | 2 | - |
| `connector_in` | Input connector | Conector de entrada | terminals | 2 | - |
| `connector_out` | Output connector | Conector de saída | terminals | 2 | - |
| `terminal_4` | 4 round terminals | 4 bornes redondos | terminals | 4 | - |
| `disconnector_1p` | Disconnector I | Seccionador I | protection | 2 | active |
| `disconnector_1pn` | Disconnector IN | Seccionador IN | protection | 4 | active |
| `disconnector_2p` | Disconnector II | Seccionador II | protection | 4 | active |
| `disconnector_3p` | Disconnector III | Seccionador III | protection | 6 | active |
| `disconnector_3pn` | Disconnector IIIN | Seccionador IIIN | protection | 8 | active |
| `switch_power_1p` | Power switch I | Interruptor I | control | 2 | active |
| `switch_power_2p` | Power switch II | Interruptor II | control | 4 | active |
| `switch_power_3p` | Power switch III | Interruptor III | control | 6 | active |
| `switch_power_4p` | Power switch IIII | Interruptor IIII | control | 8 | active |
| `teleruptor` | Latching relay (teleruptor) | Telerruptor | relays | 4 | active |
| `solenoid_valve` | Solenoid valve coil | Bobina de eletroválvula | pneumatic | 2 | active |
| `horn` | Horn | Bocina | indication | 2 | active |
| `logic_divider` | Binary divider | Divisor binário | logic | 2 | active |
| `button_station` | Pushbutton station | Botonera | control | 6 | active |
| `fuse_switch_3pn` | Fuse switch IIIN | Fusível seccionável IIIN | protection | 8 | blown |
| `contactor_combo` | Contactor with coil and auxiliary contacts | Contator com bobina e contatos auxiliares | contactors | 12 | active |
| `relay_combo` | Relay with coil and changeover contacts | Relé com bobina e contatos comutados | relays | 8 | active |
| `fuse` | Fuse | Fusível | protection | 2 | blown |
| `breaker` | Circuit breaker | Disjuntor | protection | 2 | tripped |
| `contactor_no` | Contactor main contact | Contato principal de contator | contactors | 2 | active |
| `selector_switch` | Selector switch | Chave seletora | control | 4 | position_1, position_2, position_3 |
| `thermal_relay` | Thermal overload relay | Relé térmico | protection | 4 | tripped |
| `label_box` | Text label | Etiqueta de texto | annotations | 0 | - |
