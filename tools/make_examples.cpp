#include "core/model.hpp"
#include "core/types.hpp"
#include "core/validation.hpp"
#include "io/json_io.hpp"
#include "library/symbol_definition.hpp"

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#ifndef FIOVIVO_ASSET_DIR
  #define FIOVIVO_ASSET_DIR "assets"
#endif

#ifndef FIOVIVO_EXAMPLES_DIR
  #define FIOVIVO_EXAMPLES_DIR "examples"
#endif

namespace {

using namespace fiovivo;

class Example {
public:
  Example(std::string title, PaperSize paper, Orientation orientation) {
    diagram.metadata.title = std::move(title);
    diagram.metadata.author = "FioVivo examples";
    diagram.metadata.created_utc = "2026-01-01T00:00:00Z";
    diagram.metadata.modified_utc = "2026-01-01T00:00:00Z";
    diagram.metadata.revision = "A";
    Sheet sheet;
    sheet.id = diagram.ids().newSheetId();
    sheet.name = "Folha 1";
    sheet.setup.paper = paper;
    sheet.setup.orientation = orientation;
    sheet_id = diagram.addSheet(std::move(sheet)).value();
  }

  ComponentId add(std::string_view type,
                  std::string tag,
                  Vec2 position,
                  ParameterMap parameters = {},
                  std::string description = {}) {
    const SymbolDefinition* definition = library().find(type);
    if (definition == nullptr) {
      throw std::runtime_error("Unknown symbol type: " + std::string(type));
    }
    Component component = instantiateSymbol(*definition, diagram, position, std::move(tag));
    component.description = std::move(description);
    for (auto& [key, value] : parameters) {
      component.parameters[key] = std::move(value);
    }
    const auto added = diagram.addComponent(sheet_id, std::move(component));
    if (!added) {
      throw std::runtime_error("Could not add component: " + added.error().toString());
    }
    return added.value();
  }

  void setPinRequired(ComponentId component, std::string_view pin, bool required) {
    Component* item = diagram.findComponent(component);
    if (item == nullptr) {
      throw std::runtime_error("Missing component");
    }
    Pin* target = item->findPin(pin);
    if (target == nullptr) {
      throw std::runtime_error("Missing pin " + std::string(pin));
    }
    target->required = required;
  }

  void wire(ComponentId from, std::string_view from_pin, ComponentId to, std::string_view to_pin) {
    const Component* first = diagram.findComponent(from);
    const Component* second = diagram.findComponent(to);
    if (first == nullptr || second == nullptr) {
      throw std::runtime_error("Wire endpoint component missing");
    }
    const Pin* a = first->findPin(from_pin);
    const Pin* b = second->findPin(to_pin);
    if (a == nullptr || b == nullptr) {
      throw std::runtime_error("Wire endpoint pin missing: " + first->tag + ":" +
                               std::string(from_pin) + " -> " + second->tag + ":" +
                               std::string(to_pin));
    }
    const auto created = diagram.createWire(sheet_id, {from, a->id}, {to, b->id});
    if (!created) {
      throw std::runtime_error("Could not create wire: " + created.error().toString());
    }
  }

  void text(Vec2 position, std::string value, Vec2 size = {100.0, 8.0}) {
    Annotation annotation;
    annotation.id = diagram.ids().newAnnotationId();
    annotation.kind = AnnotationKind::Text;
    annotation.position = position;
    annotation.size = size;
    annotation.text = std::move(value);
    annotation.font_size = 3.0;
    const auto result = diagram.addAnnotation(sheet_id, std::move(annotation));
    if (!result) {
      throw std::runtime_error("Could not add annotation: " + result.error().toString());
    }
  }

  void save(std::string_view filename) {
    const ValidationReport report = validate(diagram);
    for (const Problem& problem : report.problems) {
      std::cout << "  " << toString(problem.severity) << " " << toString(problem.code) << ": "
                << problem.message << "\n";
    }
    if (report.hasErrors()) {
      throw std::runtime_error("Example has validation errors: " + std::string(filename));
    }
    const std::filesystem::path directory(FIOVIVO_EXAMPLES_DIR);
    std::filesystem::create_directories(directory);
    const auto saved = io::saveFile(diagram, directory / std::string(filename));
    if (!saved) {
      throw std::runtime_error("Could not save example: " + saved.error().toString());
    }
    std::cout << "Wrote " << (directory / std::string(filename)).string() << " ("
              << diagram.componentCount() << " components, " << diagram.wireCount() << " wires)\n";
  }

  static const SymbolLibrary& library() {
    static SymbolLibrary instance = [] {
      SymbolLibrary symbols;
      const std::filesystem::path directory = std::filesystem::path(FIOVIVO_ASSET_DIR) / "symbols";
      const auto loaded = symbols.loadFromDirectory(directory);
      if (!loaded) {
        throw std::runtime_error("Could not load symbols: " + loaded.error().toString());
      }
      return symbols;
    }();
    return instance;
  }

  Diagram diagram;
  SheetId sheet_id;
};

void makeDirectOn() {
  Example example("Partida direta", PaperSize::A3, Orientation::Landscape);
  const ComponentId supply = example.add(component_type::kSupplyAc3, "X1", {10.0, 20.0});
  const ComponentId breaker = example.add(component_type::kBreaker, "Q1", {80.0, 20.0});
  const ComponentId main1 = example.add(component_type::kContactorNo, "KM1.1", {130.0, 20.0});
  const ComponentId main2 = example.add(component_type::kContactorNo, "KM1.2", {130.0, 80.0});
  const ComponentId main3 = example.add(component_type::kContactorNo, "KM1.3", {130.0, 140.0});
  const ComponentId motor = example.add(component_type::kMotor3ph, "M1", {320.0, 60.0});
  const ComponentId fuse = example.add(component_type::kFuse, "F2", {40.0, 200.0});
  const ComponentId stop = example.add(component_type::kPushbuttonNc, "S2", {90.0, 200.0});
  const ComponentId start = example.add(component_type::kPushbuttonNo, "S1", {140.0, 200.0});
  const ComponentId seal = example.add(component_type::kContactorAuxNo, "KM1.4", {140.0, 260.0});
  const ComponentId nc_aux = example.add(component_type::kContactorAuxNc, "KM1.5", {300.0, 160.0});
  const ComponentId thermal = example.add(component_type::kThermalRelay, "F1", {220.0, 200.0});
  const ComponentId coil = example.add(component_type::kContactorCoil, "KM1", {360.0, 200.0});
  const ComponentId lamp_on = example.add(component_type::kPilotLamp, "H1", {300.0, 250.0});
  const ComponentId lamp_off = example.add(component_type::kPilotLamp, "H2", {340.0, 250.0});
  const ComponentId lamp_trip = example.add(component_type::kPilotLamp, "H3", {380.0, 250.0});

  example.wire(supply, "L1", breaker, "1");
  example.wire(breaker, "2", main1, "1");
  example.wire(main1, "2", motor, "U1");
  example.wire(supply, "L2", main2, "1");
  example.wire(main2, "2", motor, "V1");
  example.wire(supply, "L3", main3, "1");
  example.wire(main3, "2", motor, "W1");

  example.wire(supply, "L1", fuse, "1");
  example.wire(fuse, "2", stop, "21");
  example.wire(stop, "22", start, "13");
  example.wire(stop, "22", seal, "13");
  example.wire(start, "14", thermal, "95");
  example.wire(seal, "14", thermal, "95");
  example.wire(thermal, "96", coil, "A1");
  example.wire(coil, "A2", supply, "N");
  example.wire(coil, "A1", lamp_on, "A1");
  example.wire(lamp_on, "A2", supply, "N");
  example.wire(fuse, "2", nc_aux, "21");
  example.wire(nc_aux, "22", lamp_off, "A1");
  example.wire(lamp_off, "A2", supply, "N");
  example.wire(fuse, "2", thermal, "97");
  example.wire(thermal, "98", lamp_trip, "A1");
  example.wire(lamp_trip, "A2", supply, "N");

  example.text({10.0, 270.0},
               "Controle em L1 + N. Os contatos 95/96 e 97/98 representam o relé térmico; "
               "o aquecimento não é simulado.",
               {260.0, 10.0});
  example.save("partida-direta.dcj");
}

void makeStarDelta() {
  Example example("Partida estrela-triângulo", PaperSize::A2, Orientation::Landscape);
  const ComponentId supply = example.add(component_type::kSupplyAc3, "X1", {10.0, 20.0});
  const ComponentId breaker = example.add(component_type::kBreaker, "Q1", {80.0, 20.0});
  const ComponentId main1 = example.add(component_type::kContactorNo, "KM1.1", {130.0, 20.0});
  const ComponentId main2 = example.add(component_type::kContactorNo, "KM1.2", {130.0, 80.0});
  const ComponentId main3 = example.add(component_type::kContactorNo, "KM1.3", {130.0, 140.0});
  const ComponentId motor = example.add(component_type::kMotor3ph, "M1", {300.0, 40.0});
  const ComponentId star_main = example.add(component_type::kContactorNo, "KM2.2", {430.0, 20.0});
  const ComponentId delta_main = example.add(component_type::kContactorNo, "KM3.2", {430.0, 80.0});
  const ComponentId star_lamp = example.add(component_type::kPilotLamp, "H2", {500.0, 20.0});
  const ComponentId delta_lamp = example.add(component_type::kPilotLamp, "H3", {500.0, 80.0});

  const ComponentId fuse = example.add(component_type::kFuse, "F2", {30.0, 250.0});
  const ComponentId stop = example.add(component_type::kPushbuttonNc, "S0", {70.0, 250.0});
  const ComponentId start = example.add(component_type::kPushbuttonNo, "S1", {110.0, 250.0});
  const ComponentId seal = example.add(component_type::kContactorAuxNo, "KM1.4", {110.0, 310.0});
  const ComponentId timer = example.add(
      component_type::kTimerOn, "TM1", {180.0, 250.0}, {{"delay_ms", std::int64_t{3000}}});
  const ComponentId transition = example.add(component_type::kContactorCoil, "K4", {280.0, 250.0});
  const ComponentId star_coil = example.add(component_type::kContactorCoil, "KM2", {370.0, 250.0});
  const ComponentId delta_coil = example.add(component_type::kContactorCoil, "KM3", {460.0, 250.0});
  const ComponentId main_coil = example.add(component_type::kContactorCoil, "KM1", {370.0, 180.0});
  const ComponentId transition_nc =
      example.add(component_type::kContactorAuxNc, "K4.1", {180.0, 330.0});
  const ComponentId delta_interlock =
      example.add(component_type::kContactorAuxNc, "KM3.1", {250.0, 330.0});
  const ComponentId transition_no =
      example.add(component_type::kContactorAuxNo, "K4.2", {280.0, 330.0});
  const ComponentId star_interlock =
      example.add(component_type::kContactorAuxNc, "KM2.1", {370.0, 330.0});

  example.wire(supply, "L1", breaker, "1");
  example.wire(breaker, "2", main1, "1");
  example.wire(main1, "2", motor, "U1");
  example.wire(supply, "L2", main2, "1");
  example.wire(main2, "2", motor, "V1");
  example.wire(supply, "L3", main3, "1");
  example.wire(main3, "2", motor, "W1");

  example.wire(supply, "L1", star_main, "1");
  example.wire(star_main, "2", star_lamp, "A1");
  example.wire(star_lamp, "A2", supply, "N");
  example.wire(supply, "L1", delta_main, "1");
  example.wire(delta_main, "2", delta_lamp, "A1");
  example.wire(delta_lamp, "A2", supply, "N");

  example.wire(supply, "L1", fuse, "1");
  example.wire(fuse, "2", stop, "21");
  example.wire(stop, "22", start, "13");
  example.wire(stop, "22", seal, "13");
  example.wire(start, "14", seal, "14");
  example.wire(start, "14", main_coil, "A1");
  example.wire(main_coil, "A2", supply, "N");
  example.wire(start, "14", timer, "A1");
  example.wire(timer, "A2", supply, "N");
  example.wire(start, "14", timer, "15");
  example.wire(timer, "18", transition, "A1");
  example.wire(transition, "A2", supply, "N");

  example.wire(start, "14", transition_nc, "21");
  example.wire(transition_nc, "22", delta_interlock, "21");
  example.wire(delta_interlock, "22", star_coil, "A1");
  example.wire(star_coil, "A2", supply, "N");

  example.wire(start, "14", transition_no, "13");
  example.wire(transition_no, "14", star_interlock, "21");
  example.wire(star_interlock, "22", delta_coil, "A1");
  example.wire(delta_coil, "A2", supply, "N");

  example.text(
      {30.0, 390.0},
      "Estrela e triângulo são intertravados por contatos NC cruzados. K4 é o relé de "
      "transição acionado pelo temporizador; o motor é alimentado pelo contator principal.",
      {420.0, 10.0});
  example.save("partida-estrela-triangulo.dcj");
}

void makeTimerSequence() {
  Example example("Sequência temporizada", PaperSize::A4, Orientation::Landscape);
  const ComponentId supply = example.add(component_type::kSupplyAc1, "X1", {10.0, 20.0});
  const ComponentId start = example.add(
      component_type::kPushbuttonNo, "S1", {70.0, 20.0}, {{"mode", std::string("maintained")}});
  const ComponentId on_timer = example.add(
      component_type::kTimerOn, "TM1", {130.0, 20.0}, {{"delay_ms", std::int64_t{2000}}});
  const ComponentId off_timer = example.add(
      component_type::kTimerOff, "TM2", {130.0, 100.0}, {{"delay_ms", std::int64_t{3000}}});
  const ComponentId lamp_on = example.add(component_type::kPilotLamp, "H1", {250.0, 20.0});
  const ComponentId lamp_off = example.add(component_type::kPilotLamp, "H2", {250.0, 100.0});

  example.wire(supply, "L", start, "13");
  example.wire(start, "14", on_timer, "A1");
  example.wire(start, "14", off_timer, "A1");
  example.wire(on_timer, "A2", supply, "N");
  example.wire(off_timer, "A2", supply, "N");
  example.wire(supply, "L", on_timer, "15");
  example.wire(on_timer, "18", lamp_on, "A1");
  example.wire(lamp_on, "A2", supply, "N");
  example.wire(supply, "L", off_timer, "15");
  example.wire(off_timer, "18", lamp_off, "A1");
  example.wire(lamp_off, "A2", supply, "N");

  example.text({20.0, 170.0},
               "S1 é mantida. H1 liga após 2 s (TON) e H2 desliga 3 s após a abertura (TOF).",
               {230.0, 10.0});
  example.save("sequencia-temporizada.dcj");
}

void makeTwoMotorInterlock() {
  Example example("Intertravamento de dois motores", PaperSize::A2, Orientation::Landscape);
  const ComponentId supply = example.add(component_type::kSupplyAc3, "X1", {10.0, 20.0});
  const ComponentId main1a = example.add(component_type::kContactorNo, "K1.3", {120.0, 20.0});
  const ComponentId main1b = example.add(component_type::kContactorNo, "K1.4", {120.0, 80.0});
  const ComponentId motor1 = example.add(component_type::kMotor3ph, "M1", {300.0, 40.0});
  const ComponentId main2a = example.add(component_type::kContactorNo, "K2.3", {120.0, 160.0});
  const ComponentId main2b = example.add(component_type::kContactorNo, "K2.4", {120.0, 220.0});
  const ComponentId motor2 = example.add(component_type::kMotor3ph, "M2", {300.0, 180.0});

  const ComponentId fuse = example.add(component_type::kFuse, "F2", {20.0, 300.0});
  const ComponentId stop1 = example.add(component_type::kPushbuttonNc, "S3", {60.0, 300.0});
  const ComponentId start1 = example.add(component_type::kPushbuttonNo, "S1", {110.0, 300.0});
  const ComponentId interlock2 =
      example.add(component_type::kContactorAuxNc, "K2.2", {160.0, 300.0});
  const ComponentId coil1 = example.add(component_type::kContactorCoil, "K1", {210.0, 300.0});
  const ComponentId seal1 = example.add(component_type::kContactorAuxNo, "K1.1", {110.0, 360.0});

  const ComponentId stop2 = example.add(component_type::kPushbuttonNc, "S4", {300.0, 300.0});
  const ComponentId start2 = example.add(component_type::kPushbuttonNo, "S2", {350.0, 300.0});
  const ComponentId interlock1 =
      example.add(component_type::kContactorAuxNc, "K1.2", {400.0, 300.0});
  const ComponentId coil2 = example.add(component_type::kContactorCoil, "K2", {450.0, 300.0});
  const ComponentId seal2 = example.add(component_type::kContactorAuxNo, "K2.1", {350.0, 360.0});

  example.wire(supply, "L1", main1a, "1");
  example.wire(main1a, "2", motor1, "U1");
  example.wire(supply, "L2", main1b, "1");
  example.wire(main1b, "2", motor1, "V1");
  example.wire(supply, "L3", motor1, "W1");
  example.wire(supply, "L1", main2a, "1");
  example.wire(main2a, "2", motor2, "U1");
  example.wire(supply, "L2", main2b, "1");
  example.wire(main2b, "2", motor2, "V1");
  example.wire(supply, "L3", motor2, "W1");

  example.wire(supply, "L1", fuse, "1");
  example.wire(fuse, "2", stop1, "21");
  example.wire(fuse, "2", stop2, "21");
  example.wire(stop1, "22", start1, "13");
  example.wire(stop1, "22", seal1, "13");
  example.wire(start1, "14", interlock2, "21");
  example.wire(seal1, "14", interlock2, "21");
  example.wire(interlock2, "22", coil1, "A1");
  example.wire(coil1, "A2", supply, "N");

  example.wire(stop2, "22", start2, "13");
  example.wire(stop2, "22", seal2, "13");
  example.wire(start2, "14", interlock1, "21");
  example.wire(seal2, "14", interlock1, "21");
  example.wire(interlock1, "22", coil2, "A1");
  example.wire(coil2, "A2", supply, "N");

  example.text({20.0, 410.0},
               "Cada contator possui um contato NC no circuito do outro. O primeiro contator "
               "avaliado vence quando ambas as partidas são pressionadas.",
               {430.0, 10.0});
  example.save("intertravamento-dois-motores.dcj");
}

void makeEmergencyStop() {
  Example example("Parada de emergência", PaperSize::A4, Orientation::Landscape);
  const ComponentId supply = example.add(component_type::kSupplyAc1, "X1", {10.0, 20.0});
  const ComponentId fuse = example.add(component_type::kFuse, "F1", {50.0, 20.0});
  const ComponentId stop = example.add(component_type::kEmergencyStop, "S0", {80.0, 20.0});
  const ComponentId start = example.add(component_type::kPushbuttonNo, "S1", {140.0, 20.0});
  const ComponentId seal = example.add(component_type::kContactorAuxNo, "KM1.1", {140.0, 80.0});
  const ComponentId coil = example.add(component_type::kContactorCoil, "KM1", {230.0, 20.0});
  const ComponentId lamp = example.add(component_type::kPilotLamp, "H1", {230.0, 80.0});
  const ComponentId reset =
      example.add(component_type::kPushbuttonNo,
                  "SR",
                  {20.0, 120.0},
                  {{"action", std::string("reset_emergency")}, {"color", std::string("black")}});
  example.setPinRequired(reset, "13", false);
  example.setPinRequired(reset, "14", false);

  example.wire(supply, "L", fuse, "1");
  example.wire(fuse, "2", stop, "1");
  example.wire(stop, "2", start, "13");
  example.wire(stop, "2", seal, "13");
  example.wire(start, "14", coil, "A1");
  example.wire(seal, "14", coil, "A1");
  example.wire(coil, "A2", supply, "N");
  example.wire(coil, "A1", lamp, "A1");
  example.wire(lamp, "A2", supply, "N");

  example.text({20.0, 170.0},
               "A parada de emergência é retentiva. Pressione SR no simulador ou use "
               "Simular > Reset Emergency Stop(s) para liberar.",
               {250.0, 12.0});
  example.save("parada-emergencia.dcj");
}

} // namespace

int main() {
  try {
    makeDirectOn();
    makeStarDelta();
    makeTimerSequence();
    makeTwoMotorInterlock();
    makeEmergencyStop();
  } catch (const std::exception& error) {
    std::cerr << "Error: " << error.what() << "\n";
    return 1;
  }
  return 0;
}
